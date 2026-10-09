#ifndef REFRESHTESTWINDOW_H
#define REFRESHTESTWINDOW_H

#include "uilanguage.h"

#include <QElapsedTimer>
#include <QImage>
#include <QPointer>
#include <QTimer>
#include <QVector>
#include <QWidget>

class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QScreen;
class QShowEvent;

class RefreshTestWindow final : public QWidget {
    Q_OBJECT
public:
    explicit RefreshTestWindow(QScreen *targetScreen, UiLanguage language,
                               QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private:
    void scheduleFrame();
    void nextMode();
    void drawMotionTest(QPainter &p, const QRectF &area, double seconds);
    void drawPacingTest(QPainter &p, const QRectF &area, double seconds);
    void drawFrameGraph(QPainter &p, const QRectF &area);

    QPointer<QScreen> m_targetScreen;
    UiLanguage m_language = UiLanguage::Chinese;
    QTimer m_timer;
    QElapsedTimer m_clock;
    qint64 m_nextFrameNs = 0;
    qint64 m_lastPaintNs = 0;
    qint64 m_fpsWindowNs = 0;
    qint64 m_frameIntervalNs = 16666667;
    int m_framesInWindow = 0;
    int m_mode = 0;
    double m_targetHz = 60.0;
    double m_measuredFps = 0.0;
    double m_lastFrameMs = 0.0;
    QVector<double> m_frameTimes;
    QVector<QImage> m_logos;
};

#endif
