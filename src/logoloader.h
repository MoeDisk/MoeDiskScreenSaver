#ifndef LOGOLOADER_H
#define LOGOLOADER_H

#include <QImage>
#include <QColor>

class LogoLoader {
public:
    static QImage loadLogo();
    static void recolorLogo(QImage &image, const QColor &color);
};

#endif
