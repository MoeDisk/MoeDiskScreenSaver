#include "movinglogo.h"

#include "logoloader.h"

#include <QtMath>
#include <algorithm>

MovingLogo::MovingLogo(const QImage &image, const QVector<QColor> &colors)
    : image(image)
    , colors(colors)
{
    rect.x = 0;
    rect.y = -10;
    rect.width = image.width();
    rect.height = image.height();
    m_watch.start();
}

void MovingLogo::animate()
{
    const double origX = rect.x;
    const double origY = rect.y;
    double x = origX;
    double y = origY;
    bool outOfBounds = false;

    if (rect.right() >= bounds.right()) {
        moveRight = false;
        outOfBounds = true;
    } else if (rect.x <= bounds.x) {
        moveRight = true;
        outOfBounds = true;
    }

    if (rect.bottom() >= bounds.bottom()) {
        moveDown = false;
        outOfBounds = true;
    } else if (rect.y <= bounds.y) {
        moveDown = true;
        outOfBounds = true;
    }

    if (outOfBounds) {
        nextColor();
    }

    switch (mode) {
    case MoveMode::Normal:
        x += moveRight ? speed : -speed;
        y += moveDown ? speed : -speed;
        break;
    case MoveMode::Opposite: {
        const double width = bounds.width - rect.width;
        const double height = bounds.height - rect.height;
        const double theta = std::atan2(height, width);
        const double hyppos = std::sqrt(std::pow(x - bounds.x, 2) + std::pow(y - bounds.y, 2));
        x = (hyppos + (moveRight ? speed : -speed) * 2.0) * std::cos(theta);
        y = (hyppos + (moveDown ? speed : -speed) * 2.0) * std::sin(theta);
        break;
    }
    case MoveMode::AllCorners:
        break;
    }

    const double step = std::max(1.0, static_cast<double>(m_watch.elapsed()) / 8.0);
    const double moveX = (x - origX) * step;
    const double moveY = (y - origY) * step;

    rect.x += moveX;
    rect.y += moveY;

    m_watch.restart();
}

void MovingLogo::rescale(const RectDbl &newBounds, double scale)
{
    bounds = newBounds;
    m_scale = scale;
    const double ratio = static_cast<double>(image.width()) / static_cast<double>(image.height());
    rect.height = bounds.diagonal() / m_scale / ratio;
    rect.width = rect.height * ratio;
    rect.x = std::min(std::max(rect.x, bounds.x), bounds.width - rect.width);
    rect.y = std::min(std::max(rect.y, bounds.y), bounds.height - rect.height);
    animate();
}

void MovingLogo::nextMode()
{
    switch (mode) {
    case MoveMode::Normal:
        mode = MoveMode::Opposite;
        break;
    case MoveMode::Opposite:
        mode = MoveMode::Normal;
        placeInRandomSpot();
        break;
    case MoveMode::AllCorners:
        mode = MoveMode::Normal;
        break;
    }
}

void MovingLogo::nextColor()
{
    if (colors.isEmpty()) {
        return;
    }
    m_colorIdx = (m_colorIdx + 1) % colors.size();
    LogoLoader::recolorLogo(image, colors.at(m_colorIdx));
}

void MovingLogo::placeInRandomSpot()
{
    if (bounds.width <= rect.width || bounds.height <= rect.height) {
        return;
    }
    const double maxX = bounds.width - rect.width;
    const double maxY = bounds.height - rect.height;
    rect.x = std::floor(QRandomGenerator::global()->generateDouble() * maxX + 1.0);
    rect.y = std::floor(QRandomGenerator::global()->generateDouble() * maxY + 1.0);
    animate();
}
