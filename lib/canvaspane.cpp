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

#include "canvaspane.h"
#include "imagecanvas.h"

#include <QJsonObject>
#include <QLoggingCategory>
#include <QtMath>

Q_LOGGING_CATEGORY(lcCanvasPane, "app.canvasPane")

CanvasPane::CanvasPane(QObject *parent) :
    QObject(parent),
    mGeometry(),
    mVisible(true),
    mSize(0.5),
    mZoomLevel(1.0)
{
}

QRectF CanvasPane::geometry() const
{
    return mGeometry;
}

void CanvasPane::setGeometry(const QRectF &geometry)
{
    if (geometry == mGeometry) return;

    mGeometry = geometry;
    emit geometryChanged();
}

bool CanvasPane::visible() const
{
    return mVisible;
}

void CanvasPane::setVisible(const bool visible)
{
    if (visible == mVisible) return;

    mVisible = visible;
    emit visibleChanged();
}

qreal CanvasPane::zoomLevel() const
{
    return mZoomLevel;
}

void CanvasPane::setZoomLevel(qreal zoomLevel)
{
    if (qFuzzyCompare(zoomLevel, mZoomLevel))
        return;

    mZoomLevel = zoomLevel;
    emit zoomLevelChanged();
}

QTransform CanvasPane::transform() const
{
    // Canvas to scene transform
    QTransform transform;
    const QPointF offset = QPointF(qRound(mGeometry.width() / 2.0), qRound(mGeometry.height() / 2.0))/* - mGeometry.topLeft()*/;
    transform.translate(offset.x(), offset.y());
    transform.scale(mZoomLevel, mZoomLevel);
    const QPointF pan = mOffset;
    transform.translate(pan.x(), pan.y());
    return transform;
}

QPointF CanvasPane::offset() const
{
    return mOffset;
}

void CanvasPane::setOffset(const QPointF &offset)
{
    if (offset == mOffset)
        return;

    qCDebug(lcCanvasPane) << "setting offset of" << objectName() << "to" << offset;

    mOffset = offset;
    emit offsetChanged();
}

void CanvasPane::read(const QJsonObject &json)
{
    setZoomLevel(json.value(QLatin1String("zoomLevel")).toDouble());
    setOffset(QPointF(json.value(QLatin1String("offsetX")).toDouble(), json.value(QLatin1String("offsetY")).toDouble()));
}

void CanvasPane::write(QJsonObject &json) const
{
    json[QLatin1String("size")] = mSize;
    // It's only important that the zoom level is a real while zooming
    // to ensure that zooming is not too quick.
    json[QLatin1String("zoomLevel")] = zoomLevel();
    json[QLatin1String("offsetX")] = mOffset.x();
    json[QLatin1String("offsetY")] = mOffset.y();
}

void CanvasPane::reset()
{
    setZoomLevel(1.0);
    setOffset(QPointF(0.0, 0.0));
}

QDebug operator<<(QDebug debug, const CanvasPane *pane)
{
    QDebugStateSaver stateSaver(debug);
    debug.nospace() << "(CanvasPane objectName=" << pane->objectName()
        << " offset=" << pane->offset()
        << " zoomLevel=" << pane->zoomLevel()
        << " geometry=" << pane->geometry()
        << ")";
    return debug.space();
}
