#include "refreshtestwindow.h"

#include "logoloader.h"

#include <QApplication>
#include <QGuiApplication>
#include <QHideEvent>
#include <QKeyEvent>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScreen>
#include <QShowEvent>
#include <QtMath>

#include <cmath>

namespace {

QString localized(UiLanguage language, const QString &chinese, const QString &english,
                  const QString &japanese)
{
    switch (language) {
    case UiLanguage::English:
        return english;
    case UiLanguage::Japanese:
        return japanese;
    case UiLanguage::Chinese:
    default:
        return chinese;
    }
}

QFont uiFont(int size, bool bold, UiLanguage language)
{
    QFont font;
#if defined(Q_OS_MACOS)
    font.setFamily(language == UiLanguage::Japanese
        ? QStringLiteral("Hiragino Sans") : QStringLiteral("PingFang SC"));
#elif defined(Q_OS_WIN)
    font.setFamily(language == UiLanguage::Japanese
        ? QStringLiteral("Yu Gothic UI") : QStringLiteral("Microsoft YaHei UI"));
#else
    font.setFamily(language == UiLanguage::Japanese
        ? QStringLiteral("Noto Sans CJK JP") : QStringLiteral("Noto Sans CJK SC"));
#endif
    font.setPixelSize(size);
    font.setBold(bold);
    return font;
}

void drawRoundedPanel(QPainter &p, const QRectF &rect, const QColor &fill,
                      const QColor &border)
{
    p.setPen(QPen(border, 1.0));
    p.setBrush(fill);
    p.drawRoundedRect(rect, 13, 13);
}

}

RefreshTestWindow::RefreshTestWindow(QScreen *targetScreen, UiLanguage language, QWidget *parent)
    : QWidget(parent)
    , m_targetScreen(targetScreen)
    , m_language(language)
{
    setWindowTitle(localized(language,
        QStringLiteral("DVD 屏幕刷新率测试"), QStringLiteral("DVD Refresh Rate Test"),
        QStringLiteral("DVD リフレッシュレートテスト")));
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);

    if (!m_targetScreen) {
        m_targetScreen = QGuiApplication::primaryScreen();
    }
    if (m_targetScreen && m_targetScreen->refreshRate() >= 24.0) {
        m_targetHz = m_targetScreen->refreshRate();
    }
    m_frameIntervalNs = qMax<qint64>(1000000,
        qRound64(1000000000.0 / m_targetHz));

    const QVector<QColor> colors = {
        QColor("#78d6ff"), QColor("#ff78c6"), QColor("#b89cff")
    };
    for (const QColor &color : colors) {
        QImage logo = LogoLoader::loadLogo();
        LogoLoader::recolorLogo(logo, color);
        m_logos.push_back(logo);
    }

    m_timer.setTimerType(Qt::PreciseTimer);
    m_timer.setInterval(1);
    connect(&m_timer, &QTimer::timeout, this, &RefreshTestWindow::scheduleFrame);
}

void RefreshTestWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    m_clock.restart();
    m_nextFrameNs = 0;
    m_lastPaintNs = 0;
    m_fpsWindowNs = 0;
    m_framesInWindow = 0;
    m_frameTimes.clear();
    m_timer.start();
}

void RefreshTestWindow::hideEvent(QHideEvent *event)
{
    m_timer.stop();
    QWidget::hideEvent(event);
}

void RefreshTestWindow::scheduleFrame()
{
    if (!isVisible()) {
        return;
    }
    const qint64 now = m_clock.nsecsElapsed();
    if (m_nextFrameNs == 0) {
        m_nextFrameNs = now;
    }
    if (now < m_nextFrameNs) {
        return;
    }
    update();
    m_nextFrameNs += m_frameIntervalNs;
    if (now - m_nextFrameNs > m_frameIntervalNs * 4) {
        m_nextFrameNs = now + m_frameIntervalNs;
    }
}

void RefreshTestWindow::paintEvent(QPaintEvent *)
{
    const qint64 now = m_clock.nsecsElapsed();
    if (m_lastPaintNs > 0) {
        m_lastFrameMs = (now - m_lastPaintNs) / 1000000.0;
        if (m_lastFrameMs < 250.0) {
            m_frameTimes.push_back(m_lastFrameMs);
            if (m_frameTimes.size() > 180) {
                m_frameTimes.remove(0, m_frameTimes.size() - 180);
            }
        }
    }
    m_lastPaintNs = now;
    if (m_fpsWindowNs == 0) {
        m_fpsWindowNs = now;
    }
    ++m_framesInWindow;
    const qint64 fpsSpan = now - m_fpsWindowNs;
    if (fpsSpan >= 1000000000LL) {
        m_measuredFps = m_framesInWindow * 1000000000.0 / fpsSpan;
        m_framesInWindow = 0;
        m_fpsWindowNs = now;
    }

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.fillRect(rect(), QColor("#07101f"));

    QLinearGradient topGradient(0, 0, width(), 102);
    topGradient.setColorAt(0, QColor("#10213d"));
    topGradient.setColorAt(0.5, QColor("#15284b"));
    topGradient.setColorAt(1, QColor("#251d49"));
    p.fillRect(QRectF(0, 0, width(), 104), topGradient);
    p.setPen(QColor("#eef7ff"));
    p.setFont(uiFont(24, true, m_language));
    p.drawText(QRectF(30, 15, width() * .45, 35), Qt::AlignLeft | Qt::AlignVCenter,
               m_mode == 0
                   ? localized(m_language, QStringLiteral("DVD 运动清晰度测试"),
                         QStringLiteral("DVD Motion Clarity Test"),
                         QStringLiteral("DVD 動きの鮮明度テスト"))
                   : localized(m_language, QStringLiteral("帧节奏与扫描测试"),
                         QStringLiteral("Frame Pacing & Scan Test"),
                         QStringLiteral("フレームペーシング・走査テスト")));
    p.setPen(QColor("#91add4"));
    p.setFont(uiFont(12, false, m_language));
    const QString screenName = m_targetScreen ? m_targetScreen->name()
        : localized(m_language, QStringLiteral("当前屏幕"), QStringLiteral("Current display"),
                    QStringLiteral("現在の画面"));
    const QString instruction = localized(m_language,
        QStringLiteral("%1  ·  左键/空格切换画面  ·  右键/Esc退出"),
        QStringLiteral("%1  ·  Left click/Space: change view  ·  Right click/Esc: exit"),
        QStringLiteral("%1  ·  左クリック/Space：画面切替  ·  右クリック/Esc：終了"));
    p.drawText(QRectF(31, 54, width() * .5, 25), Qt::AlignLeft | Qt::AlignVCenter,
               instruction.arg(screenName));

    const qreal cardWidth = qMin<qreal>(168, width() * .14);
    const qreal cardGap = 11;
    const qreal cardsLeft = width() - 30 - cardWidth * 3 - cardGap * 2;
    const QStringList captions = {
        localized(m_language, QStringLiteral("系统报告"), QStringLiteral("System report"),
                  QStringLiteral("システム報告")),
        localized(m_language, QStringLiteral("绘制采样"), QStringLiteral("Render sample"),
                  QStringLiteral("描画サンプル")),
        localized(m_language, QStringLiteral("最近帧时间"), QStringLiteral("Latest frame time"),
                  QStringLiteral("直近フレーム時間"))
    };
    const QString sampling = localized(m_language, QStringLiteral("采样中…"),
        QStringLiteral("Sampling…"), QStringLiteral("計測中…"));
    const QStringList values = {
        QStringLiteral("%1 Hz").arg(m_targetHz, 0, 'f', 2),
        m_measuredFps > 0.0 ? QStringLiteral("%1 FPS").arg(m_measuredFps, 0, 'f', 1)
                            : sampling,
        m_lastFrameMs > 0.0 ? QStringLiteral("%1 ms").arg(m_lastFrameMs, 0, 'f', 2)
                            : sampling
    };
    for (int i = 0; i < 3; ++i) {
        const QRectF card(cardsLeft + i * (cardWidth + cardGap), 14, cardWidth, 73);
        drawRoundedPanel(p, card, QColor(255, 255, 255, 16), QColor(126, 168, 222, 75));
        p.setPen(QColor("#88a6cf"));
        p.setFont(uiFont(11, false, m_language));
        p.drawText(card.adjusted(10, 7, -10, -38), Qt::AlignCenter, captions.at(i));
        p.setPen(i == 0 ? QColor("#75dcff") : QColor("#f5f8ff"));
        p.setFont(uiFont(17, true, m_language));
        p.drawText(card.adjusted(8, 29, -8, -8), Qt::AlignCenter, values.at(i));
    }

    const QRectF testArea(24, 124, width() - 48, height() - 247);
    if (m_mode == 0) {
        drawMotionTest(p, testArea, now / 1000000000.0);
    } else {
        drawPacingTest(p, testArea, now / 1000000000.0);
    }
    drawFrameGraph(p, QRectF(24, height() - 105, width() - 48, 78));
}

void RefreshTestWindow::drawMotionTest(QPainter &p, const QRectF &area, double seconds)
{
    drawRoundedPanel(p, area, QColor("#0b172a"), QColor(76, 112, 166, 100));
    const int lanes = 3;
    const qreal laneHeight = area.height() / lanes;
    const QVector<double> speeds = {260.0, 520.0, 900.0};
    const QVector<QColor> laneColors = {
        QColor("#63caee"), QColor("#ef74bd"), QColor("#a98cf2")
    };
    for (int lane = 0; lane < lanes; ++lane) {
        const QRectF laneRect(area.left() + 14, area.top() + lane * laneHeight + 8,
                              area.width() - 28, laneHeight - 16);
        if (lane > 0) {
            p.setPen(QPen(QColor(92, 124, 170, 55), 1));
            p.drawLine(QPointF(area.left() + 16, laneRect.top() - 8),
                       QPointF(area.right() - 16, laneRect.top() - 8));
        }
        p.setPen(QColor(134, 165, 207));
        p.setFont(uiFont(12, true, m_language));
        p.drawText(QRectF(laneRect.left() + 8, laneRect.top() + 5, 150, 24),
                   Qt::AlignLeft | Qt::AlignVCenter,
                   QStringLiteral("%1 px/s").arg(speeds.at(lane), 0, 'f', 0));

        const qreal logoWidth = qBound<qreal>(112, laneRect.height() * 1.65, 220);
        const qreal logoHeight = logoWidth * 360.0 / 822.0;
        const qreal travel = laneRect.width() + logoWidth;
        const qreal x = laneRect.left() - logoWidth
            + std::fmod(seconds * speeds.at(lane) + lane * travel * .31, travel);
        const qreal y = laneRect.center().y() - logoHeight / 2 + 7;
        p.drawImage(QRectF(x, y, logoWidth, logoHeight), m_logos.at(lane));

        p.setPen(QPen(laneColors.at(lane), 1));
        const qreal step = qMax<qreal>(10, speeds.at(lane) / m_targetHz);
        for (qreal marker = laneRect.left() + 5; marker < laneRect.right(); marker += step) {
            p.drawLine(QPointF(marker, laneRect.bottom() - 9),
                       QPointF(marker, laneRect.bottom() - 4));
        }
    }
}

void RefreshTestWindow::drawPacingTest(QPainter &p, const QRectF &area, double seconds)
{
    drawRoundedPanel(p, area, QColor("#091322"), QColor(76, 112, 166, 100));
    const QRectF inner = area.adjusted(16, 16, -16, -16);
    const int cell = qMax(14, int(inner.height() / 11));
    for (int y = 0; y < int(inner.height()); y += cell) {
        for (int x = 0; x < int(inner.width()); x += cell) {
            const bool alternate = ((x / cell) + (y / cell)) % 2;
            p.fillRect(QRectF(inner.left() + x, inner.top() + y,
                              qMin(cell, int(inner.width()) - x),
                              qMin(cell, int(inner.height()) - y)),
                       alternate ? QColor(18, 35, 60) : QColor(10, 23, 42));
        }
    }

    const double speed = qMax(520.0, inner.width() * .55);
    const qreal x = inner.left() + std::fmod(seconds * speed, inner.width());
    QLinearGradient beam(x - 46, 0, x + 18, 0);
    beam.setColorAt(0, QColor(78, 209, 255, 0));
    beam.setColorAt(.72, QColor(78, 209, 255, 85));
    beam.setColorAt(1, QColor(255, 255, 255, 235));
    p.fillRect(QRectF(x - 46, inner.top(), 64, inner.height()), beam);
    p.fillRect(QRectF(x, inner.top(), 2, inner.height()), QColor("#ffffff"));

    const qreal orbit = std::fmod(seconds * 320.0, inner.width() + 180) - 90;
    const QRectF puck(inner.left() + orbit - 54, inner.center().y() - 30, 108, 60);
    p.setPen(QPen(QColor("#dff8ff"), 2));
    p.setBrush(QColor("#4fbce8"));
    p.drawRoundedRect(puck, 18, 18);
    p.setPen(Qt::white);
    p.setFont(uiFont(14, true, m_language));
    p.drawText(puck, Qt::AlignCenter,
               localized(m_language, QStringLiteral("追踪我"), QStringLiteral("Follow me"),
                         QStringLiteral("追いかけて")));

    p.setPen(QColor(215, 230, 250, 205));
    p.setFont(uiFont(13, false, m_language));
    p.drawText(inner.adjusted(16, 14, -16, -14), Qt::AlignLeft | Qt::AlignTop,
               localized(m_language,
                   QStringLiteral("用眼睛跟随蓝色块：跳跃感代表帧节奏不均，长尾或重影主要反映画面响应。"),
                   QStringLiteral("Follow the blue block with your eyes: jumps indicate uneven pacing; trails and ghosting mostly reflect panel response."),
                   QStringLiteral("青いブロックを目で追ってください。動きの跳びはフレーム間隔の乱れ、尾や残像は主に画面応答を示します。")));
}

void RefreshTestWindow::drawFrameGraph(QPainter &p, const QRectF &area)
{
    drawRoundedPanel(p, area, QColor(255, 255, 255, 10), QColor(94, 132, 184, 70));
    const double expectedMs = 1000.0 / m_targetHz;
    p.setPen(QColor("#90a9cb"));
    p.setFont(uiFont(11, false, m_language));
    const QString graphText = localized(m_language,
        QStringLiteral("应用绘制帧时间（参考线 %1 ms；它是程序采样，不是硬件测量）"),
        QStringLiteral("App render frame time (reference %1 ms; software sample, not a hardware measurement)"),
        QStringLiteral("アプリ描画フレーム時間（基準 %1 ms；ソフトウェア計測で、ハードウェア測定ではありません）"));
    p.drawText(area.adjusted(13, 5, -13, -48), Qt::AlignLeft | Qt::AlignVCenter,
               graphText.arg(expectedMs, 0, 'f', 2));
    const QRectF graph = area.adjusted(13, 27, -13, -9);
    const double maxMs = qMax(33.0, expectedMs * 2.4);
    const qreal expectedY = graph.bottom() - qMin(1.0, expectedMs / maxMs) * graph.height();
    p.setPen(QPen(QColor(112, 215, 247, 100), 1, Qt::DashLine));
    p.drawLine(QPointF(graph.left(), expectedY), QPointF(graph.right(), expectedY));
    if (m_frameTimes.size() < 2) {
        return;
    }
    QPainterPath path;
    const int count = m_frameTimes.size();
    for (int i = 0; i < count; ++i) {
        const qreal x = graph.left() + graph.width() * i / qMax(1, count - 1);
        const qreal normalized = qMin(1.0, m_frameTimes.at(i) / maxMs);
        const qreal y = graph.bottom() - normalized * graph.height();
        if (i == 0) path.moveTo(x, y); else path.lineTo(x, y);
    }
    p.setPen(QPen(QColor("#f08bc8"), 1.7));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
}

void RefreshTestWindow::nextMode()
{
    m_mode = (m_mode + 1) % 2;
    update();
}

void RefreshTestWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
        return;
    }
    if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return
        || event->key() == Qt::Key_Enter) {
        nextMode();
        return;
    }
    QWidget::keyPressEvent(event);
}

void RefreshTestWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        close();
        return;
    }
    if (event->button() == Qt::LeftButton) {
        nextMode();
        return;
    }
    QWidget::mousePressEvent(event);
}
