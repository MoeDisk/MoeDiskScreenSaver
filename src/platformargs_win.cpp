#include "platformargs.h"

#include <QCoreApplication>

namespace {

quintptr parentWindowIdFromArguments(const QString &flag, const QStringList &arguments)
{
    QString value = flag.mid(2).trimmed();
    if (value.startsWith(QLatin1Char(':'))) {
        value.remove(0, 1);
    }
    if (value.isEmpty() && arguments.size() >= 3) {
        value = arguments.at(2).trimmed();
    }
    bool ok = false;
    const qulonglong id = value.toULongLong(&ok);
    return ok ? static_cast<quintptr>(id) : 0;
}

}

ScreenSaverOptions parseScreenSaverArgs(const QStringList &arguments)
{
    ScreenSaverOptions options;

    if (arguments.size() < 2) {
        const QString path = QCoreApplication::applicationFilePath();
        if (path.endsWith(QStringLiteral(".scr"), Qt::CaseInsensitive)) {
            options.command = ScreenSaverCommand::Run;
        } else {
            options.command = ScreenSaverCommand::Standalone;
        }
        return options;
    }

    const QString flag = arguments.at(1).trimmed().toLower();
    if (flag == QStringLiteral("--screensaver")) {
        options.command = ScreenSaverCommand::Run;
    } else if (flag == QStringLiteral("--preview")) {
        options.command = ScreenSaverCommand::Preview;
        options.parentWindowId = parentWindowIdFromArguments(QString(), arguments);
    } else if (flag == QStringLiteral("--settings")
               || flag == QStringLiteral("--configure")) {
        options.command = ScreenSaverCommand::Configure;
    } else if (flag.startsWith(QLatin1Char('/')) || flag.startsWith(QLatin1Char('-'))) {
        const QChar code = flag.size() > 1 ? flag.at(1) : QChar();
        switch (code.toLatin1()) {
        case 's':
            options.command = ScreenSaverCommand::Run;
            break;
        case 'p':
            options.command = ScreenSaverCommand::Preview;
            options.parentWindowId = parentWindowIdFromArguments(flag, arguments);
            break;
        case 'c':
            options.command = ScreenSaverCommand::Configure;
            options.parentWindowId = parentWindowIdFromArguments(flag, arguments);
            break;
        default:
            options.command = ScreenSaverCommand::Standalone;
            break;
        }
    } else {
        options.command = ScreenSaverCommand::Standalone;
    }

    return options;
}
