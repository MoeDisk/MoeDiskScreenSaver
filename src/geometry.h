#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <QtGlobal>
#include <cmath>

struct RectDbl {
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;

    double right() const { return x + width; }
    double bottom() const { return y + height; }
    double diagonal() const { return std::sqrt(width * width + height * height); }
};

#endif
