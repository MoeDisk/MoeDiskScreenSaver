#ifndef MOVINGLOGO_H
#define MOVINGLOGO_H

#include "geometry.h"

#include <QColor>
#include <QElapsedTimer>
#include <QImage>
#include <QRandomGenerator>
#include <QVector>

enum class MoveMode {
    Normal,
    Opposite,
    AllCorners
};

class MovingLogo {
public:
    explicit MovingLogo(const QImage &image, const QVector<QColor> &colors);

    void animate();
    void rescale(const RectDbl &bounds, double scale);
    void nextMode();
    void nextColor();
    void placeInRandomSpot();

    QImage image;
    QVector<QColor> colors;
    MoveMode mode = MoveMode::Normal;
    bool moveRight = true;
    bool moveDown = true;
    double speed = 1.0;
    RectDbl rect;
    RectDbl bounds;

private:
    int m_colorIdx = -1;
    QElapsedTimer m_watch;
    double m_scale = 6.0;
};

#endif
