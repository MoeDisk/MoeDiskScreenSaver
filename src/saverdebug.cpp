#include "saverdebug.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QIODevice>
#include <QtGlobal>

void saverDebugLog(const QString &message)
{
    if (!qEnvironmentVariableIsSet("MOEDISK_SAVER_DEBUG")) {
        return;
    }
    QFile file(QStringLiteral("/tmp/moedisk-screensaver.log"));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        return;
    }
    const QString line = QDateTime::currentDateTime().toString(Qt::ISODate)
        + QLatin1Char(' ') + message + QLatin1Char('\n');
    file.write(line.toUtf8());
}
