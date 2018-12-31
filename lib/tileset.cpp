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

Tileset::Tileset(const QString &fileName, const QImage &image, const QSize tileSize, const int columns, const int size, QObject *const parent) :
    TileSetModel(image, tileSize, columns, size, parent),
    mFileName(fileName)
{
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
