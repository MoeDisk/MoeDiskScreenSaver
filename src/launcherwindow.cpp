#include "launcherwindow.h"

#include "refreshtestwindow.h"
#include "saverwindow.h"

#include <QApplication>
#include <QDialog>
#include <QFrame>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QLocale>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QStyle>
#include <QVBoxLayout>
#include <QWindow>

namespace {

QString launcherStyle()
{
    return QStringLiteral(
        "QLabel#brand{color:#285fb7;font-size:16px;font-weight:800;}"
        "QLabel#edition{background:#dcecff;border:1px solid #a9caf1;border-radius:10px;"
        "color:#4c77b9;font-size:11px;font-weight:700;padding:4px 10px;}"
        "QLabel#eyebrow{color:#7290bd;font-size:11px;font-weight:700;}"
        "QLabel#title{color:#174ba8;font-size:28px;font-weight:800;}"
        "QLabel#intro{color:#5d7fac;font-size:13px;}"
        "QLabel#speech{background:rgba(255,255,255,220);border:1px solid #b8d5f7;"
        "border-radius:16px;color:#486fae;font-size:13px;padding:11px 14px;}"
        "QLabel#screenInfo{background:#eef6ff;border:1px solid #c8dcf5;border-radius:12px;"
        "color:#6987b0;font-size:11px;padding:8px 11px;}"
        "QLabel#creator{color:#88a1c5;font-size:11px;font-weight:700;}"
        "QFrame#homeCard{background:rgba(255,255,255,205);border:1px solid #bed5f2;"
        "border-radius:22px;}"
        "QPushButton#closeButton{border:0;background:rgba(255,255,255,150);border-radius:15px;"
        "color:#7290bd;font-size:18px;font-weight:700;}"
        "QPushButton#closeButton:hover{background:#ffffff;color:#245fc0;}"
        "QPushButton#languageButton{border:1px solid #bdd3ef;border-radius:10px;"
        "background:rgba(255,255,255,135);color:#6685b1;font-size:11px;font-weight:700;}"
        "QPushButton#languageButton:hover{background:#ffffff;color:#2c64bd;}"
        "QPushButton#languageButton[selected=\"true\"]{background:#5d91ed;color:white;"
        "border-color:#4c82df;}"
        "QPushButton.action{border:1px solid #a8c8f2;border-radius:17px;"
        "background:rgba(255,255,255,215);color:#245bb5;text-align:left;"
        "font-size:15px;font-weight:750;padding:11px 18px;}"
        "QPushButton.action:hover{background:#ffffff;border-color:#68a1ec;}"
        "QPushButton#primaryAction{background:#5c91ed;color:white;border-color:#4d82dc;}"
        "QPushButton#primaryAction:hover{background:#3f7fe4;}"
        "QPushButton#refreshAction{background:#f2eaff;color:#7954ba;border-color:#ccb9ed;}"
        "QPushButton#refreshAction:hover{background:#ffffff;border-color:#a989dd;}"
        "QPushButton.dialogButton{min-height:38px;border:1px solid #8fb4e9;border-radius:19px;"
        "background:#fafdff;color:#2d64ba;font-size:14px;font-weight:700;padding:0 22px;}"
        "QPushButton.dialogButton:hover{background:#e1efff;}"
        "QPushButton#startRefresh{background:#5d91ed;color:white;border-color:#4c82df;}"
        "QPushButton#startRefresh:hover{background:#397de4;}"
    );
}

QLabel *makeLabel(const QString &text, const char *name, QWidget *parent)
{
    auto *label = new QLabel(text, parent);
    label->setObjectName(QString::fromLatin1(name));
    return label;
}

QPoint globalMousePosition(QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->globalPosition().toPoint();
#else
    return event->globalPos();
#endif
}

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

}

LauncherWindow::LauncherWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(QStringLiteral("盘姬的 DVD 小屋"));
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(1024, 650);
    setStyleSheet(launcherStyle());

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(28, 20, 28, 26);
    root->setSpacing(0);

    auto *top = new QHBoxLayout;
    m_brandLabel = makeLabel(QString(), "brand", this);
    m_editionLabel = makeLabel(QString(), "edition", this);
    m_editionLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_chineseButton = makeLanguageButton(QStringLiteral("中"), UiLanguage::Chinese);
    m_englishButton = makeLanguageButton(QStringLiteral("EN"), UiLanguage::English);
    m_japaneseButton = makeLanguageButton(QStringLiteral("日"), UiLanguage::Japanese);
    auto *close = new QPushButton(QStringLiteral("×"), this);
    close->setObjectName(QStringLiteral("closeButton"));
    close->setFixedSize(30, 30);
    close->setCursor(Qt::PointingHandCursor);
    connect(close, &QPushButton::clicked, this, &QWidget::close);
    top->addWidget(m_brandLabel);
    top->addSpacing(10);
    top->addWidget(m_editionLabel);
    top->addStretch();
    top->addWidget(m_chineseButton);
    top->addSpacing(5);
    top->addWidget(m_englishButton);
    top->addSpacing(5);
    top->addWidget(m_japaneseButton);
    top->addSpacing(12);
    top->addWidget(close);
    root->addLayout(top);
    root->addSpacing(7);

    auto *body = new QHBoxLayout;
    body->setSpacing(18);

    auto *petColumn = new QVBoxLayout;
    petColumn->setContentsMargins(3, 0, 0, 1);
    auto *mascot = new QLabel(this);
    mascot->setAlignment(Qt::AlignCenter);
    const QPixmap mascotPixmap(QStringLiteral(":/dvd-princess-mascot.png"));
    mascot->setPixmap(mascotPixmap.scaled(490, 490, Qt::KeepAspectRatio,
                                          Qt::SmoothTransformation));
    petColumn->addWidget(mascot, 1);
    m_speechLabel = makeLabel(QString(), "speech", this);
    m_speechLabel->setAlignment(Qt::AlignCenter);
    petColumn->addWidget(m_speechLabel);
    body->addLayout(petColumn, 53);

    auto *homeCard = new QFrame(this);
    homeCard->setObjectName(QStringLiteral("homeCard"));
    auto *actions = new QVBoxLayout(homeCard);
    actions->setContentsMargins(25, 23, 25, 20);
    actions->setSpacing(11);
    m_eyebrowLabel = makeLabel(QStringLiteral("DVD SCREEN FRIEND  ·  MOEDISK"), "eyebrow", homeCard);
    m_titleLabel = makeLabel(QString(), "title", homeCard);
    m_introLabel = makeLabel(QString(), "intro", homeCard);
    m_introLabel->setWordWrap(true);
    actions->addWidget(m_eyebrowLabel);
    actions->addWidget(m_titleLabel);
    actions->addWidget(m_introLabel);
    actions->addSpacing(7);

    m_windowedButton = makeActionButton(QString(), QString(), QStringLiteral("primaryAction"));
    m_fullScreenButton = makeActionButton(QString(), QString(), QString());
    m_refreshButton = makeActionButton(QString(), QString(), QStringLiteral("refreshAction"));
    connect(m_windowedButton, &QPushButton::clicked, this, [this] { openDvdWindow(false); });
    connect(m_fullScreenButton, &QPushButton::clicked, this, [this] { openDvdWindow(true); });
    connect(m_refreshButton, &QPushButton::clicked, this, &LauncherWindow::openRefreshRateGuide);
    actions->addWidget(m_windowedButton);
    actions->addWidget(m_fullScreenButton);
    actions->addWidget(m_refreshButton);
    actions->addStretch();

    m_screenInfoLabel = makeLabel(QString(), "screenInfo", this);
    m_screenInfoLabel->setWordWrap(true);
    m_creatorLabel = makeLabel(QString(), "creator", this);
    m_creatorLabel->setAlignment(Qt::AlignCenter);
    actions->addWidget(m_screenInfoLabel);
    actions->addWidget(m_creatorLabel);
    body->addWidget(homeCard, 47);
    root->addLayout(body, 1);

    const QString savedLanguage = QSettings().value(QStringLiteral("ui/language")).toString();
    UiLanguage initialLanguage = UiLanguage::Chinese;
    if (savedLanguage == QStringLiteral("en")) {
        initialLanguage = UiLanguage::English;
    } else if (savedLanguage == QStringLiteral("ja")) {
        initialLanguage = UiLanguage::Japanese;
    } else if (savedLanguage.isEmpty()) {
        if (QLocale::system().language() == QLocale::Japanese) {
            initialLanguage = UiLanguage::Japanese;
        } else if (QLocale::system().language() != QLocale::Chinese) {
            initialLanguage = UiLanguage::English;
        }
    }
    applyLanguage(initialLanguage, false);
}

QPushButton *LauncherWindow::makeLanguageButton(const QString &text, UiLanguage language)
{
    auto *button = new QPushButton(text, this);
    button->setObjectName(QStringLiteral("languageButton"));
    button->setFixedSize(37, 26);
    button->setCursor(Qt::PointingHandCursor);
    connect(button, &QPushButton::clicked, this,
            [this, language] { applyLanguage(language); });
    return button;
}

void LauncherWindow::applyLanguage(UiLanguage language, bool persist)
{
    m_language = language;
    if (persist) {
        const QString code = language == UiLanguage::English ? QStringLiteral("en")
            : language == UiLanguage::Japanese ? QStringLiteral("ja") : QStringLiteral("zh");
        QSettings().setValue(QStringLiteral("ui/language"), code);
    }

    setWindowTitle(localized(language,
        QStringLiteral("盘姬的 DVD 小屋"), QStringLiteral("Princess' DVD Corner"),
        QStringLiteral("盤姫のDVDルーム")));
    m_brandLabel->setText(localized(language,
        QStringLiteral("✦  盘姬工具箱"), QStringLiteral("✦  Princess Toolbox"),
        QStringLiteral("✦  盤姫ツールボックス")));
    m_editionLabel->setText(localized(language,
        QStringLiteral("DVD 版"), QStringLiteral("DVD Edition"), QStringLiteral("DVD版")));
    m_speechLabel->setText(localized(language,
        QStringLiteral("DVD 撞到角落的瞬间超治愈～\n也可以让我陪你看看屏幕跑得顺不顺！"),
        QStringLiteral("That perfect corner hit is oddly satisfying~\nI can also help you check how smoothly your display moves!"),
        QStringLiteral("DVDロゴが角にぴったり当たる瞬間って癒やされる～\n画面がなめらかに動くか、一緒に見てみよう！")));
    m_titleLabel->setText(localized(language,
        QStringLiteral("今天想怎么玩？"), QStringLiteral("What shall we play today?"),
        QStringLiteral("今日は何して遊ぶ？")));
    m_introLabel->setText(localized(language,
        QStringLiteral("让 DVD 标志散散步，或者和盘姬一起\n看看屏幕跑得够不够顺。"),
        QStringLiteral("Let the DVD logo roam around, or let Princess\nhelp you check how smoothly the screen moves."),
        QStringLiteral("DVDロゴをお散歩させる？ それとも盤姫と一緒に\n画面のなめらかさをチェックする？")));
    m_windowedButton->setText(localized(language,
        QStringLiteral("▶  打开 DVD 动画\n窗口模式 · 可用 F11 切换全屏"),
        QStringLiteral("▶  Open DVD animation\nWindowed mode · F11 toggles fullscreen"),
        QStringLiteral("▶  DVDアニメを開く\nウィンドウモード · F11で全画面切替")));
    m_fullScreenButton->setText(localized(language,
        QStringLiteral("▣  全屏看 DVD 弹来弹去\n沉浸模式 · Esc 返回窗口"),
        QStringLiteral("▣  Watch DVD bounce fullscreen\nImmersive mode · Esc returns to window"),
        QStringLiteral("▣  DVDを全画面で楽しむ\n没入モード · Escでウィンドウに戻る")));
    m_refreshButton->setText(localized(language,
        QStringLiteral("✦  测试屏幕刷新率\n动态追踪 · 帧时间 · 顺滑度观察"),
        QStringLiteral("✦  Test display refresh rate\nMotion tracking · Frame time · Smoothness"),
        QStringLiteral("✦  画面のリフレッシュレートをテスト\nモーショントラッキング · フレーム時間 · 滑らかさ")));
    m_creatorLabel->setText(language == UiLanguage::Chinese
        ? QStringLiteral("@本地磁盘姬 作品") : QStringLiteral("@made by MoeDisk"));
    updateScreenInfo();
    updateLanguageButtonState();
}

void LauncherWindow::updateLanguageButtonState()
{
    const QList<QPair<QPushButton *, UiLanguage> > buttons = {
        qMakePair(m_chineseButton, UiLanguage::Chinese),
        qMakePair(m_englishButton, UiLanguage::English),
        qMakePair(m_japaneseButton, UiLanguage::Japanese)
    };
    for (const auto &entry : buttons) {
        entry.first->setProperty("selected", entry.second == m_language);
        entry.first->style()->unpolish(entry.first);
        entry.first->style()->polish(entry.first);
    }
}

void LauncherWindow::updateScreenInfo()
{
    QScreen *current = screen() ? screen() : QGuiApplication::primaryScreen();
    if (!current) {
        m_screenInfoLabel->setText(localized(m_language,
            QStringLiteral("没有读到当前屏幕信息"),
            QStringLiteral("No display information available"),
            QStringLiteral("画面情報を取得できませんでした")));
        return;
    }
    const QString format = localized(m_language,
        QStringLiteral("当前屏幕：%1 × %2  ·  系统报告 %3 Hz"),
        QStringLiteral("Current display: %1 × %2  ·  System reports %3 Hz"),
        QStringLiteral("現在の画面：%1 × %2  ·  システム報告 %3 Hz"));
    m_screenInfoLabel->setText(format.arg(current->size().width())
        .arg(current->size().height()).arg(current->refreshRate(), 0, 'f', 2));
}

QPushButton *LauncherWindow::makeActionButton(const QString &title, const QString &subtitle,
                                              const QString &objectName)
{
    auto *button = new QPushButton(QStringLiteral("%1\n%2").arg(title, subtitle), this);
    button->setProperty("class", QStringLiteral("action"));
    button->setStyleSheet(QStringLiteral(
        "QPushButton{border:1px solid #a8c8f2;border-radius:17px;"
        "background:rgba(255,255,255,215);color:#245bb5;text-align:left;"
        "font-size:15px;font-weight:700;padding:10px 18px;}"
        "QPushButton:hover{background:#ffffff;border-color:#68a1ec;}"));
    if (!objectName.isEmpty()) {
        button->setObjectName(objectName);
        if (objectName == QStringLiteral("primaryAction")) {
            button->setStyleSheet(QStringLiteral(
                "QPushButton{border:1px solid #4d82dc;border-radius:17px;background:#5c91ed;"
                "color:white;text-align:left;font-size:15px;font-weight:700;padding:10px 18px;}"
                "QPushButton:hover{background:#3f7fe4;}"));
        } else if (objectName == QStringLiteral("refreshAction")) {
            button->setStyleSheet(QStringLiteral(
                "QPushButton{border:1px solid #ccb9ed;border-radius:17px;background:#f2eaff;"
                "color:#7954ba;text-align:left;font-size:15px;font-weight:700;padding:10px 18px;}"
                "QPushButton:hover{background:#ffffff;border-color:#a989dd;}"));
        }
    }
    button->setMinimumHeight(66);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

void LauncherWindow::openDvdWindow(bool fullScreen)
{
    auto *saver = new SaverWindow(SaverWindow::RunMode::Standalone);
    saver->setAttribute(Qt::WA_DeleteOnClose);
    saver->setWindowTitle(QStringLiteral("MoeDiskScreenSaver"));
    saver->resize(960, 640);
    if (QScreen *target = screen()) {
        saver->winId();
        if (saver->windowHandle()) {
            saver->windowHandle()->setScreen(target);
        }
    }
    if (fullScreen) {
        saver->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        saver->showFullScreen();
    } else {
        saver->show();
    }
    saver->raise();
    saver->activateWindow();
}

void LauncherWindow::openRefreshRateGuide()
{
    QDialog dialog(this);
    dialog.setWindowTitle(localized(m_language,
        QStringLiteral("刷新率测试 · 盘姬陪你看顺滑度"),
        QStringLiteral("Refresh rate test · Princess checks smoothness with you"),
        QStringLiteral("リフレッシュレートテスト · 盤姫と滑らかさを確認")));
    dialog.setFixedSize(780, 458);
    dialog.setStyleSheet(launcherStyle() + QStringLiteral(
        "QDialog{background:qlineargradient(x1:0,y1:0,x2:1,y2:1,stop:0 #f9fcff,"
        "stop:.55 #e8f4ff,stop:1 #e4dcff);color:#2457ce;}"
        "QFrame#guideCard{background:rgba(255,255,255,225);border:1px solid #aecbf1;"
        "border-radius:18px;}"
        "QLabel#guideTitle{font-size:20px;font-weight:800;color:#2456ad;}"
        "QLabel#guideText{font-size:14px;color:#4d72a8;}"));

    auto *layout = new QHBoxLayout(&dialog);
    layout->setContentsMargins(18, 18, 18, 18);
    layout->setSpacing(18);
    auto *picture = new QLabel(&dialog);
    picture->setFixedSize(324, 420);
    picture->setAlignment(Qt::AlignCenter);
    const QPixmap art(QStringLiteral(":/dvd-princess-mascot.png"));
    picture->setPixmap(art.scaled(318, 414, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    layout->addWidget(picture);

    auto *card = new QFrame(&dialog);
    card->setObjectName(QStringLiteral("guideCard"));
    auto *content = new QVBoxLayout(card);
    content->setContentsMargins(24, 23, 24, 20);
    content->setSpacing(11);
    auto *title = makeLabel(localized(m_language,
        QStringLiteral("和盘姬一起追帧吧 ✦"),
        QStringLiteral("Let's chase frames together ✦"),
        QStringLiteral("盤姫と一緒にフレームを追いかけよう ✦")), "guideTitle", card);
    title->setAlignment(Qt::AlignCenter);
    auto *text = makeLabel(localized(m_language,
        QStringLiteral(
            "全屏测试会让 DVD 标志以不同速度横向移动，\n"
            "方便观察拖影、跳帧和运动是否顺滑。\n\n"
            "顶部会显示系统报告的刷新率；\n"
            "绘制采样与帧时间用于判断程序是否稳定，\n"
            "不等同于仪器测得的面板真实刷新率。\n\n"
            "左键 / 空格：切换测试画面\n"
            "右键或 Esc：结束测试"),
        QStringLiteral(
            "The fullscreen test moves DVD logos at several speeds\n"
            "to reveal blur, skipped frames, and motion smoothness.\n\n"
            "The top bar shows the refresh rate reported by the system.\n"
            "Render sampling and frame time indicate app stability;\n"
            "they are not instrument measurements of the panel.\n\n"
            "Left click / Space: change test view\n"
            "Right click or Esc: finish"),
        QStringLiteral(
            "全画面でDVDロゴを複数の速度で横に動かし、\n"
            "残像、コマ落ち、動きの滑らかさを確認します。\n\n"
            "上部にはシステム報告のリフレッシュレートを表示。\n"
            "描画サンプルとフレーム時間はアプリの安定性を見る値で、\n"
            "パネルを測定器で計測した値ではありません。\n\n"
            "左クリック / Space：テスト画面を切替\n"
            "右クリックまたは Esc：終了")), "guideText", card);
    text->setAlignment(Qt::AlignCenter);
    text->setWordWrap(true);
    auto *buttons = new QHBoxLayout;
    auto *cancel = new QPushButton(localized(m_language,
        QStringLiteral("先等等"), QStringLiteral("Maybe later"), QStringLiteral("あとで")), card);
    auto *start = new QPushButton(localized(m_language,
        QStringLiteral("开始追帧 ✦"), QStringLiteral("Start frame chase ✦"),
        QStringLiteral("追跡スタート ✦")), card);
    cancel->setProperty("class", QStringLiteral("dialogButton"));
    start->setProperty("class", QStringLiteral("dialogButton"));
    cancel->setStyleSheet(QStringLiteral(
        "QPushButton{min-height:38px;border:1px solid #8fb4e9;border-radius:19px;"
        "background:#fafdff;color:#2d64ba;font-size:14px;font-weight:700;padding:0 20px;}"
        "QPushButton:hover{background:#e1efff;}"));
    start->setStyleSheet(QStringLiteral(
        "QPushButton{min-height:38px;border:1px solid #4c82df;border-radius:19px;"
        "background:#5d91ed;color:white;font-size:14px;font-weight:700;padding:0 20px;}"
        "QPushButton:hover{background:#397de4;}"));
    buttons->addWidget(cancel);
    buttons->addWidget(start);
    content->addWidget(title);
    content->addWidget(text, 1);
    content->addLayout(buttons);
    layout->addWidget(card, 1);

    connect(cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    connect(start, &QPushButton::clicked, &dialog, &QDialog::accept);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    QScreen *target = screen() ? screen() : QGuiApplication::primaryScreen();
    auto *test = new RefreshTestWindow(target, m_language);
    test->setAttribute(Qt::WA_DeleteOnClose);
    test->winId();
    if (target && test->windowHandle()) {
        test->windowHandle()->setScreen(target);
        test->setGeometry(target->geometry());
    }
    test->showFullScreen();
    test->raise();
    test->activateWindow();
    test->setFocus(Qt::ActiveWindowFocusReason);
}

void LauncherWindow::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath shadow;
    shadow.addRoundedRect(rect().adjusted(8, 9, -8, -7), 27, 27);
    painter.fillPath(shadow, QColor(39, 84, 145, 42));
    QPainterPath panel;
    panel.addRoundedRect(rect().adjusted(6, 5, -10, -11), 26, 26);
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0.0, QColor("#f7fcff"));
    gradient.setColorAt(0.52, QColor("#e3f2ff"));
    gradient.setColorAt(1.0, QColor("#e8e0ff"));
    painter.fillPath(panel, gradient);
    painter.setPen(QPen(QColor(147, 187, 235, 180), 1.2));
    painter.drawPath(panel);
    painter.setPen(QPen(QColor(255, 255, 255, 190), 2));
    painter.drawArc(QRectF(20, 104, 335, 335), 28 * 16, 116 * 16);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 170));
    painter.drawEllipse(QPointF(69, 92), 5, 5);
    painter.drawEllipse(QPointF(446, 517), 7, 7);
    painter.drawEllipse(QPointF(477, 489), 3, 3);
    painter.setPen(QColor(255, 255, 255, 205));
    painter.setFont(QFont(QStringLiteral("Arial"), 17, QFont::Bold));
    painter.drawText(QPointF(55, 145), QStringLiteral("✦"));
    painter.drawText(QPointF(469, 177), QStringLiteral("✦"));
}

void LauncherWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->pos().y() < 56) {
        m_dragging = true;
        m_dragOffset = globalMousePosition(event) - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void LauncherWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(globalMousePosition(event) - m_dragOffset);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void LauncherWindow::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QWidget::mouseReleaseEvent(event);
}
