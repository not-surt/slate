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

#ifndef CANVASPANE_H
#define CANVASPANE_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QSize>
#include <QTransform>

#include "slate-global.h"

class QJsonObject;
class ImageCanvas;

class SLATE_EXPORT CanvasPane : public QObject
{
    Q_OBJECT
    Q_PROPERTY(qreal zoomLevel READ zoomLevel WRITE setZoomLevel NOTIFY zoomLevelChanged)
    Q_PROPERTY(QRectF geometry READ geometry NOTIFY geometryChanged)
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)

public:
    explicit CanvasPane(QObject *parent = nullptr);

    QRectF geometry() const;
    void setGeometry(const QRectF &geometry);

    bool visible() const;
    void setVisible(const bool visible);

    qreal zoomLevel() const;
    void setZoomLevel(qreal zoomLevel);

    QTransform transform() const;

    QPointF offset() const;
    void setOffset(const QPointF &offset);

    void read(const QJsonObject &json);
    void write(QJsonObject &json) const;

    void reset();

signals:
    void zoomLevelChanged();
    void geometryChanged();
    void visibleChanged();
    void sizeChanged();
    void offsetChanged();

private:
    QRectF mGeometry;
    bool mVisible;
    qreal mSize;
    qreal mZoomLevel;
    QPointF mOffset;
};

SLATE_EXPORT QDebug operator<<(QDebug debug, const CanvasPane *pane);

#endif // CANVASPANE_H
