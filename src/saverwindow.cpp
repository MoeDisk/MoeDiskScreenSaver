#include "saverwindow.h"

#include "logoloader.h"
#include "saverdebug.h"

#include <QApplication>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QScreen>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {
QVector<QColor> defaultColors()
{
    return {
        QColor(190, 0, 255),
        QColor(255, 0, 139),
        QColor(255, 131, 0),
        QColor(0, 38, 255),
        QColor(255, 250, 0),
    };
}
}

SaverWindow::SaverWindow(RunMode mode, QWidget *parent)
    : QWidget(parent)
    , m_mode(mode)
{
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAutoFillBackground(false);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);

    if (m_mode == RunMode::ScreenSaver || m_mode == RunMode::Standalone) {
        setCursor(Qt::BlankCursor);
    }

    setupLogo();

    m_timer.setInterval(8);
    connect(&m_timer, &QTimer::timeout, this, [this]() {
        if (m_previewHwnd) {
#ifdef Q_OS_WIN
            if (!IsWindow(reinterpret_cast<HWND>(m_previewHwnd))) {
                qApp->quit();
                return;
            }
#endif
        }

        if (m_logo) {
            m_logo->animate();
            update();
        }
    });
}

SaverWindow::~SaverWindow()
{
    delete m_logo;
}

void SaverWindow::attachPreviewWindow(WId previewHwnd)
{
    m_previewHwnd = previewHwnd;
#ifdef Q_OS_WIN
    HWND hwnd = reinterpret_cast<HWND>(winId());
    HWND parent = reinterpret_cast<HWND>(previewHwnd);
    SetWindowLongPtr(hwnd, GWL_STYLE, WS_CHILD | WS_VISIBLE);
    SetParent(hwnd, parent);
    RECT rc;
    GetClientRect(parent, &rc);
    SetWindowPos(hwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW);
#endif
}

void SaverWindow::setupLogo()
{
    QImage base = LogoLoader::loadLogo();
    if (base.isNull()) {
        saverDebugLog(QStringLiteral("loadLogo failed"));
        return;
    }
    saverDebugLog(QStringLiteral("loadLogo ok ") + QString::number(base.width()) + QLatin1Char('x')
                  + QString::number(base.height()));
    m_logo = new MovingLogo(base, defaultColors());
    m_logo->nextColor();
}

void SaverWindow::updateBounds()
{
    if (!m_logo) {
        return;
    }
    RectDbl bounds;
    bounds.x = 0;
    bounds.y = 0;
    bounds.width = width();
    bounds.height = height();
    m_logo->rescale(bounds, kLogoScale);
    m_logo->placeInRandomSpot();
}

void SaverWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    if (!m_logo || m_logo->image.isNull()) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter,
                         tr("DVD logo failed to load.\nRebuild the app or run macdeployqt on the .app bundle."));
        return;
    }
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    const QRectF target(m_logo->rect.x, m_logo->rect.y, m_logo->rect.width, m_logo->rect.height);
    painter.drawImage(target, m_logo->image);
}

void SaverWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!m_logo) {
        return;
    }
    RectDbl bounds;
    bounds.x = 0;
    bounds.y = 0;
    bounds.width = width();
    bounds.height = height();
    m_logo->rescale(bounds, kLogoScale);
}

void SaverWindow::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (width() < 32 || height() < 32) {
        if (QScreen *s = screen()) {
            setGeometry(s->geometry());
        } else if (QScreen *primary = QGuiApplication::primaryScreen()) {
            setGeometry(primary->geometry());
        }
    }
    updateBounds();
    m_timer.start();
    QTimer::singleShot(0, this, [this]() { update(); });
}

void SaverWindow::handleInteractiveKeys(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        if (isFullScreen() || (windowFlags() & Qt::FramelessWindowHint)) {
            setWindowFlags(Qt::Window);
            showNormal();
            if (m_logo) {
                m_logo->speed = 1.0;
            }
        }
        return;
    }

    if (!m_logo) {
        return;
    }

    switch (event->key()) {
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_F11:
        if (windowFlags() & Qt::FramelessWindowHint) {
            setWindowFlags(Qt::Window);
            showNormal();
            m_logo->speed = 1.0;
        } else {
            setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
            showFullScreen();
            m_logo->speed = 1.33;
        }
        break;
    case Qt::Key_Space:
        m_logo->nextMode();
        break;
    case Qt::Key_Left:
        m_logo->moveRight = false;
        break;
    case Qt::Key_Right:
        m_logo->moveRight = true;
        break;
    case Qt::Key_Up:
        m_logo->moveDown = false;
        break;
    case Qt::Key_Down:
        m_logo->moveDown = true;
        break;
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        m_logo->speed = (event->key() - Qt::Key_0) * 0.33;
        break;
    default:
        break;
    }
}

void SaverWindow::exitIfScreenSaver()
{
    if (m_mode == RunMode::ScreenSaver) {
        qApp->quit();
    }
}

void SaverWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_mode == RunMode::Preview) {
        QWidget::keyPressEvent(event);
        return;
    }

    if (m_mode == RunMode::ScreenSaver) {
        exitIfScreenSaver();
        return;
    }

    handleInteractiveKeys(event);
    update();
    QWidget::keyPressEvent(event);
}

void SaverWindow::mouseMoveEvent(QMouseEvent *event)
{
    exitIfScreenSaver();
    QWidget::mouseMoveEvent(event);
}

void SaverWindow::mousePressEvent(QMouseEvent *event)
{
    exitIfScreenSaver();
    QWidget::mousePressEvent(event);
}
