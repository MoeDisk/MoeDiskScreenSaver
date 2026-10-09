#ifndef SAVERWINDOW_H
#define SAVERWINDOW_H

#include "movinglogo.h"

#include <QTimer>
#include <QWidget>

class SaverWindow : public QWidget {
    Q_OBJECT
public:
    enum class RunMode {
        Standalone,
        ScreenSaver,
        Preview
    };

    explicit SaverWindow(RunMode mode, QWidget *parent = nullptr);
    ~SaverWindow() override;

    void attachPreviewWindow(WId previewHwnd);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private:
    void setupLogo();
    void updateBounds();
    void handleInteractiveKeys(QKeyEvent *event);
    void exitIfScreenSaver();

    RunMode m_mode;
    MovingLogo *m_logo = nullptr;
    QTimer m_timer;
    WId m_previewHwnd = 0;
    static constexpr int kLogoScale = 8;
};

#endif
