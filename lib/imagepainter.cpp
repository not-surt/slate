#include "imagepainter.h"

#include <QPainter>

ImagePainter::ImagePainter(QQuickItem *const parent) :
    QQuickPaintedItem(parent),
    mImage()
{
    setImplicitWidth(mImage.width());
    setImplicitHeight(mImage.height());
}

QImage ImagePainter::image() const
{
    return mImage;
}

void ImagePainter::setImage(const QImage &image)
{
    if (mImage == image)
        return;

    mImage = image;
    emit imageChanged(mImage);

    setImplicitWidth(mImage.width());
    setImplicitHeight(mImage.height());
    update();
}

void ImagePainter::paint(QPainter *const painter)
{
    painter->drawImage(boundingRect(), mImage, mImage.rect());
}
