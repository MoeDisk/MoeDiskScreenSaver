#include "logoloader.h"

QImage LogoLoader::loadLogo()
{
    QImage image(QStringLiteral(":/DVDVideo360.png"));
    if (image.isNull()) {
        return QImage();
    }
    return image.convertToFormat(QImage::Format_ARGB32);
}

void LogoLoader::recolorLogo(QImage &image, const QColor &color)
{
    if (image.format() != QImage::Format_ARGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }

    const int r = color.red();
    const int g = color.green();
    const int b = color.blue();

    for (int y = 0; y < image.height(); ++y) {
        QRgb *line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(line[x]) > 0) {
                line[x] = qRgba(r, g, b, qAlpha(line[x]));
            }
        }
    }
}
