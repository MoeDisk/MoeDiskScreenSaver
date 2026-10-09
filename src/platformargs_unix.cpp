#include "platformargs.h"

#include <QCommandLineParser>
#include <QCoreApplication>

namespace {

bool isWindowsStyleFlag(const QString &arg, QChar code, ScreenSaverCommand *out)
{
    const QString lower = arg.trimmed().toLower();
    if (!lower.startsWith(QLatin1Char('/')) && !lower.startsWith(QLatin1Char('-'))) {
        return false;
    }
    if (lower.size() < 2) {
        return false;
    }
    const QChar c = lower.at(1);
    if (c != code && c != QChar(code.toUpper())) {
        return false;
    }
    if (lower.size() == 2) {
        switch (code.toLatin1()) {
        case 's':
            *out = ScreenSaverCommand::Run;
            return true;
        case 'p':
            *out = ScreenSaverCommand::Preview;
            return true;
        case 'c':
            *out = ScreenSaverCommand::Configure;
            return true;
        default:
            return false;
        }
    }
    return false;
}

}

ScreenSaverOptions parseScreenSaverArgs(const QStringList &arguments)
{
    ScreenSaverOptions options;

    if (arguments.size() >= 2) {
        ScreenSaverCommand legacy = ScreenSaverCommand::Standalone;
        if (isWindowsStyleFlag(arguments.at(1), QLatin1Char('s'), &legacy)) {
            options.command = legacy;
            return options;
        }
        if (isWindowsStyleFlag(arguments.at(1), QLatin1Char('p'), &legacy)) {
            options.command = legacy;
            return options;
        }
        if (isWindowsStyleFlag(arguments.at(1), QLatin1Char('c'), &legacy)) {
            options.command = legacy;
            return options;
        }
    }

    QCommandLineParser parser;
    parser.setApplicationDescription(
        QObject::tr("Bouncing DVD logo screen saver (MoeDisk)."));
    parser.addHelpOption();
    parser.addVersionOption();

    const QCommandLineOption screensaver(
        QStringList() << QStringLiteral("s") << QStringLiteral("screensaver"),
        QObject::tr("Full-screen screen saver (exit on mouse or key)."));
    const QCommandLineOption preview(
        QStringList() << QStringLiteral("p") << QStringLiteral("preview"),
        QObject::tr("Preview in a resizable window."));
    const QCommandLineOption settings(
        QStringList() << QStringLiteral("c") << QStringLiteral("settings")
                      << QStringLiteral("configure"),
        QObject::tr("Show settings dialog."));
    const QCommandLineOption windowed(
        QStringList() << QStringLiteral("w") << QStringLiteral("window"),
        QObject::tr("Windowed mode with keyboard controls (default)."));

    parser.addOption(screensaver);
    parser.addOption(preview);
    parser.addOption(settings);
    parser.addOption(windowed);

    parser.process(arguments);

    if (parser.isSet(screensaver)) {
        options.command = ScreenSaverCommand::Run;
    } else if (parser.isSet(preview)) {
        options.command = ScreenSaverCommand::Preview;
    } else if (parser.isSet(settings)) {
        options.command = ScreenSaverCommand::Configure;
    } else {
        options.command = ScreenSaverCommand::Standalone;
    }

    return options;
}
