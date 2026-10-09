#include "configdialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

ConfigDialog::ConfigDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("MoeDiskScreenSaver"));
    resize(360, 180);

    auto *layout = new QVBoxLayout(this);
    auto *info = new QLabel(
        tr("Classic bouncing DVD logo (MoeDisk).\n\n"
           "Windows: use .scr with /s, /p, /c, or run the .exe windowed.\n"
           "Linux / macOS: run with --screensaver for full screen;\n"
           "--preview, --settings, or no flags for windowed mode.\n\n"
           "Keys in windowed mode: Space = mode, arrows, 0-9 = speed."),
        this);
    info->setWordWrap(true);
    layout->addWidget(info);

    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    layout->addWidget(buttons);
}
