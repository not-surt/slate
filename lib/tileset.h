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

#ifndef TILESET_H
#define TILESET_H

#include <QString>
#include <QImage>

#include "slate-global.h"

#include "tilesetmodel.h"

class SLATE_EXPORT Tileset : public TileSetModel
{
public:
    Tileset(const QString &fileName, const QImage &image, const QSize tileSize, const int columns, const int pixelSize, QObject *const parent = nullptr);

    bool isValid() const;
    QString fileName() const;
    void setFileName(const QString &fileName);

private:
    QString mFileName;
};

#endif // TILESET_H
