#ifndef PLATFORMARGS_H
#define PLATFORMARGS_H

#include <QStringList>
#include <QtGlobal>

enum class ScreenSaverCommand {
    Run,
    Configure,
    Preview,
    Standalone
};

struct ScreenSaverOptions {
    ScreenSaverCommand command = ScreenSaverCommand::Standalone;
    quintptr parentWindowId = 0;
};

ScreenSaverOptions parseScreenSaverArgs(const QStringList &arguments);
int runScreenSaverApplication(int &argc, char **argv);

#endif
