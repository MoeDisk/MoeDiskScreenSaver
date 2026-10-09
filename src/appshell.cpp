#include "platformargs.h"

#include "configdialog.h"
#include "launcherwindow.h"
#include "saverdebug.h"
#include "saverwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QScreen>
#include <QWidget>
#include <QWindow>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

static void enableHighDpi()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
}

static void setupBundledQtPlugins(char **argv)
{
#if defined(Q_OS_MACOS)
    if (!argv || !argv[0]) {
        return;
    }
    const QString macosDir = QFileInfo(QString::fromLocal8Bit(argv[0])).absolutePath();
    const QString contentsDir = QDir(macosDir).absoluteFilePath(QStringLiteral(".."));
    const QString plugIns = QDir(contentsDir).absoluteFilePath(QStringLiteral("PlugIns"));
    const QString frameworks = QDir(contentsDir).absoluteFilePath(QStringLiteral("Frameworks"));
    qputenv("QT_PLUGIN_PATH", QFile::encodeName(plugIns));
    qputenv("QT_QPA_PLATFORM", "cocoa");
    QCoreApplication::addLibraryPath(plugIns);
    QCoreApplication::addLibraryPath(frameworks);
    saverDebugLog(QStringLiteral("QT_PLUGIN_PATH=") + plugIns);
#endif
}

#if defined(Q_OS_MACOS)
static bool isMacSaverBundlePath(const QString &applicationDirPath)
{
    const QDir macosDir(applicationDirPath);
    const QString bundlePath = QDir(macosDir.absoluteFilePath(QStringLiteral("..")))
                                   .absoluteFilePath(QStringLiteral(".."));
    return bundlePath.endsWith(QStringLiteral(".saver"), Qt::CaseInsensitive);
}
#endif

static void presentScreenSaver(SaverWindow *saver, QScreen *screen)
{
    saver->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    if (screen) {
        saver->setGeometry(screen->geometry());
    }
    saver->winId();
    if (QWindow *window = saver->windowHandle()) {
        if (screen) {
            window->setScreen(screen);
        }
    }
    saver->showFullScreen();
    saver->raise();
    saver->activateWindow();
    saver->repaint();
}

static void showScreenSaverOnAllScreens()
{
    const QList<QScreen *> screens = QGuiApplication::screens();
    saverDebugLog(QStringLiteral("screens=") + QString::number(screens.size()));

    if (screens.isEmpty()) {
        auto *saver = new SaverWindow(SaverWindow::RunMode::ScreenSaver);
        presentScreenSaver(saver, QGuiApplication::primaryScreen());
        return;
    }

    for (QScreen *screen : screens) {
        auto *saver = new SaverWindow(SaverWindow::RunMode::ScreenSaver);
        presentScreenSaver(saver, screen);
    }
}

int runScreenSaverApplication(int &argc, char **argv)
{
    enableHighDpi();
    setupBundledQtPlugins(argv);

    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("MoeDiskScreenSaver"));
    QApplication::setOrganizationName(QStringLiteral("MoeDisk"));
    QApplication::setApplicationVersion(QStringLiteral("1.0.1"));
    QApplication::setWindowIcon(QIcon(QStringLiteral(":/dvd-princess-icon.png")));

    const ScreenSaverOptions options = parseScreenSaverArgs(app.arguments());

    ScreenSaverCommand mode = options.command;
#if defined(Q_OS_MACOS)
    if (mode == ScreenSaverCommand::Standalone
        && isMacSaverBundlePath(QCoreApplication::applicationDirPath())) {
        mode = ScreenSaverCommand::Run;
        saverDebugLog(QStringLiteral("macOS .saver bundle → Run mode"));
    }
#endif
    saverDebugLog(QStringLiteral("mode=") + QString::number(static_cast<int>(mode)));

    switch (mode) {
    case ScreenSaverCommand::Configure: {
        ConfigDialog dialog;
#ifdef Q_OS_WIN
        if (options.parentWindowId) {
            dialog.winId();
            SetParent(reinterpret_cast<HWND>(dialog.winId()),
                      reinterpret_cast<HWND>(options.parentWindowId));
            RECT rc;
            GetClientRect(reinterpret_cast<HWND>(options.parentWindowId), &rc);
            dialog.setGeometry(0, 0, rc.right - rc.left, rc.bottom - rc.top);
        }
#endif
        dialog.exec();
        return 0;
    }
    case ScreenSaverCommand::Preview: {
        SaverWindow saver(SaverWindow::RunMode::Preview);
#ifdef Q_OS_WIN
        if (options.parentWindowId) {
            saver.attachPreviewWindow(static_cast<WId>(options.parentWindowId));
            saver.show();
        } else
#endif
        {
            saver.setWindowTitle(QStringLiteral("MoeDiskScreenSaver"));
            saver.resize(640, 480);
            saver.show();
        }
        return app.exec();
    }
    case ScreenSaverCommand::Run:
        showScreenSaverOnAllScreens();
        return app.exec();
    case ScreenSaverCommand::Standalone:
    default: {
        LauncherWindow launcher;
        launcher.show();
        return app.exec();
    }
    }
}
