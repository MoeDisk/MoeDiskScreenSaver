#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>

class ConfigDialog : public QDialog {
    Q_OBJECT
public:
    explicit ConfigDialog(QWidget *parent = nullptr);
};

#endif
