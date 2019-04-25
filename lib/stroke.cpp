#include "stroke.h"
#include "utils.h"

bool StrokePoint::operator==(const StrokePoint &other) const
{
    return pos == other.pos && qFuzzyCompare(pressure, other.pressure);
}

bool StrokePoint::operator!=(const StrokePoint &other) const
{
    return !(*this == other);
}

StrokePoint StrokePoint::snapped(const QPointF snapOffset) const
{
    return StrokePoint{{qRound(pos.x() + snapOffset.x()) - snapOffset.x(), qRound(pos.y() + snapOffset.y()) - snapOffset.y()}, pressure};
}

QPoint StrokePoint::pixel() const
{
    return QPoint{qFloor(pos.x()), qFloor(pos.y())};
}

StrokePoint StrokePoint::lerp(const StrokePoint &from, const StrokePoint &to, const qreal pos)
{
    const QPointF point =  Utils::lerp(from.pos, to.pos, pos);
    const qreal pressure = Utils::lerp(from.pressure, to.pressure, pos);
    return StrokePoint{point, pressure};
}

qreal Stroke::applySegment(std::function<void(const StrokePoint &)> func, const StrokePoint &from, const StrokePoint &to, const qreal offset, const bool stepOffsetOnly)
{
    const QPointF posDelta = {to.pos.x() - from.pos.x(), to.pos.y() - from.pos.y()};
    const qreal steps = qMax(qMax(qAbs(posDelta.x()), qAbs(posDelta.y())), 1.0);
    const qreal step = 1.0 / steps;
    qreal pos = offset * step;
    while (pos < 1.0/* || qFuzzyCompare(pos, 1.0)*/) {
        if (!stepOffsetOnly) {
            func(StrokePoint::lerp(from, to, pos));
        }
        pos += step;
    }
//    qDebug() << pos << (offset + qFloor(steps)) * step;
    return (pos - 1.0) * steps;
}

StrokePoint Stroke::snapped(const int index, const QPointF snapOffset, const bool snapToPixel) const
{
    if (!snapToPixel) return at(index);
    else return at(index).snapped(snapOffset);
}

void Stroke::apply(std::function<void(const StrokePoint &)> func, const BrushManager &brush, const bool snapToPixel) const
{
    if (isEmpty()) return;

    int i = 0;
    StrokePoint from = snapped(i, brush.handle(), snapToPixel), to;
    if (length() > 1) {
        ++i;
    }
    qreal segmentOffset = 0.0;

    auto arePixelsNeighbors = [](const QPoint &point0, const QPoint &point1) {
        return qAbs(point1.x() - point0.x()) <= 1 &&
               qAbs(point1.y() - point0.y()) <= 1;
    };
    QVector<QPoint> pixels(3);
    pixels.append(from.pos.toPoint());
//    if (pixels[pixels.length() - 1] == pixels[pixels.length() - 2] && pixels.length() > 1) {

//    }

    while (i < length()) {
        do {
            to = snapped(i, brush.handle(), snapToPixel);
            ++i;
        } while (to == from && i < length());
//        const QRect clipRect = painter->clipBoundingRect().toAlignedRect();
//        const QRect segmentBounds = Stroke{from, to}.bounds(brush, scaleMin, scaleMax);
//        const bool drawSegment = clipRect.isValid() && clipRect.intersects(segmentBounds);
        const bool drawSegment = true;
        segmentOffset = applySegment(func, from, to, segmentOffset, !drawSegment);
        from = to;
    }
//    painter->restore();
}

QRect Stroke::bounds(const BrushManager &brush, const qreal scaleMin, const qreal scaleMax)
{
    QRectF bounds;
    for (auto point : *this) {
        bounds = bounds.united(brush.bounds(point.pos, scaleMin + point.pressure * (scaleMax - scaleMin)));
    }
    return bounds.toAlignedRect();
}
