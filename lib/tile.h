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

#ifndef TILE_H
#define TILE_H

#include <QDebug>
#include <QImage>
#include <QRect>

#include "slate-global.h"

class Tileset;

class SLATE_EXPORT TileObject
{
public:
    TileObject(const int id = -1, const Tileset *const tileset = nullptr, const QRect &sourceRect = QRect());

    bool isValid() const;
    int id() const;
    QRect sourceRect() const;
    QColor pixelColor(int x, int y) const;
    QColor pixelColor(const QPoint &position) const;
    const Tileset *tileset() const;
    QImage image() const;

    static int invalidId();

private:
    int mId;
    QRect mSourceRect;
    const Tileset *mTileset;
};

QDebug operator<<(QDebug debug, const TileObject &tile);

#endif // TILE_H
