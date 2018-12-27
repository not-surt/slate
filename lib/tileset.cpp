/*
    Copyright 2016, Mitch Curtis

    This file is part of Slate.

    Slate is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Slate is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Slate. If not, see <http://www.gnu.org/licenses/>.
*/

#include "tileset.h"

#include <QPainter>

#include "utils.h"

Tileset::Tileset(const QString &fileName, int tilesWide, int tilesHigh, QObject *parent) :
    QObject(parent),
    mFileName(fileName),
    mImage(fileName),
    mSize(tilesWide, tilesHigh),
    mTileSize(mImage.width() / mSize.width(), mImage.height() / mSize.height())
{
    qDebug() << mImage.size() << mSize << mTileSize;////////////////////////
}

bool Tileset::isValid() const
{
    return !mImage.isNull();
}

QString Tileset::fileName() const
{
    return mFileName;
}

void Tileset::setFileName(const QString &fileName)
{
    if (fileName == mFileName)
        return;

    mFileName = fileName;
}

const QImage *Tileset::image() const
{
    return &mImage;
}

QImage *Tileset::image()
{
    return &mImage;
}

void Tileset::setSize(QSize size)
{
    if (mSize == size)
        return;

    mSize = size;
    emit sizeChanged();
}

QSize Tileset::size() const
{
    return mSize;
}

void Tileset::setTileSize(QSize tileSize)
{
    if (mTileSize == tileSize)
        return;

    mTileSize = tileSize;
    emit tileSizeChanged();
}

QSize Tileset::tileSize() const
{
    return mTileSize;
}

void Tileset::setPixelColor(int x, int y, const QColor &colour)
{
    mImage.setPixelColor(x, y, colour);
    emit imageChanged();
}

void Tileset::copy(const QPoint &sourceTopLeft, const QPoint &targetTopLeft)
{
    if (!validTopLeft(sourceTopLeft) || !validTopLeft(targetTopLeft)) {
        return;
    }

    for (int y = 0; y < tileSize().height(); ++y) {
        for (int x = 0; x < tileSize().width(); ++x) {
            const QColor sourceColour = mImage.pixelColor(sourceTopLeft + QPoint(x, y));
            mImage.setPixelColor(targetTopLeft.x() + x, targetTopLeft.y() + y, sourceColour);
        }
    }
    emit imageChanged();
}

void Tileset::rotateCounterClockwise(const QPoint &tileTopLeft)
{
    rotate(tileTopLeft, -90);
}

void Tileset::rotateClockwise(const QPoint &tileTopLeft)
{
    rotate(tileTopLeft, 90);
}

QSize Tileset::pixelSize() const
{
    return QSize(size().width() * tileSize().width(), size().height() * tileSize().height());
}

void Tileset::resize(const QSize size, const QSize tileSize)
{

}

// It's easier to just allow calling code to modify the tileset image through
// the pointer (e.g. by painting with QPainter) and then call this than force
// them to use setPixelColour().
void Tileset::notifyImageChanged()
{
    emit imageChanged();
}

bool Tileset::validTopLeft(const QPoint &topLeft) const
{
    const QRect imageRect(0, 0, mImage.width(), mImage.height());
    if (!imageRect.contains(QRect(topLeft, mTileSize))) {
        qWarning() << "Tileset top-left" << topLeft << "is outside of tileset bounds" << imageRect;
        return false;
    }

    if (topLeft.x() % mTileSize.width() != 0 || topLeft.y() % mTileSize.height() != 0) {
        qWarning() << "Tileset top-left" << topLeft << "is not a multiple of the tile size";
        return false;
    }

    return true;
}

void Tileset::rotate(const QPoint &tileTopLeft, int angle)
{
    if (!validTopLeft(tileTopLeft)) {
        return;
    }

    const QImage tileImage = mImage.copy(QRect(tileTopLeft, tileSize()));
    const QImage rotatedImage = Utils::rotate(tileImage, angle);
    QPainter painter(&mImage);
    // Make sure that we clear the previous tile image before painting on the newly rotated one.
    for (int y = 0; y < tileImage.height(); ++y) {
        for (int x = 0; x < tileImage.width(); ++x) {
            mImage.setPixelColor(tileTopLeft.x() + x, tileTopLeft.y() + y, Qt::transparent);
        }
    }
    painter.drawImage(tileTopLeft, rotatedImage);
    emit imageChanged();
}
