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

StrokePoint StrokePoint::snapped(const QVector2D snapOffset) const
{
    return StrokePoint{{qRound(pos.x() + snapOffset.x()) - snapOffset.x(), qRound(pos.y() + snapOffset.y()) - snapOffset.y()}, pressure};
}

QPoint StrokePoint::pixel() const
{
    return QPoint{qFloor(pos.x()), qFloor(pos.y())};
}

StrokePoint StrokePoint::lerp(const StrokePoint &from, const StrokePoint &to, const float pos)
{
    const QVector2D point =  Utils::lerp(from.pos, to.pos, pos);
    const float pressure = Utils::lerp(from.pressure, to.pressure, pos);
    return StrokePoint{point, pressure};
}

float Stroke::applySegment(std::function<void(const StrokePoint &)> func, const StrokePoint &from, const StrokePoint &to, const float offset, const bool stepOffsetOnly)
{
    const QVector2D posDelta = {to.pos.x() - from.pos.x(), to.pos.y() - from.pos.y()};
    const float steps = qMax(qMax(qAbs(posDelta.x()), qAbs(posDelta.y())), 1.0f);
    const float step = 1.0f / steps;
    float pos = offset * step;
    while (pos < 1.0f/* || qFuzzyCompare(pos, 1.0f)*/) {
        if (!stepOffsetOnly) {
            func(StrokePoint::lerp(from, to, pos));
        }
        pos += step;
    }
//    qDebug() << pos << (offset + qFloor(steps)) * step;
    return (pos - 1.0f) * steps;
}

StrokePoint Stroke::snapped(const int index, const QVector2D snapOffset, const bool snapToPixel) const
{
    if (!snapToPixel) return at(index);
    else return at(index).snapped(snapOffset);
}

void Stroke::apply(std::function<void(const StrokePoint &)> func, const Brush &brush, const QRect &clipRect, const bool snapToPixel) const
{
    if (isEmpty()) return;

    int i = 0;
    StrokePoint from = snapped(i, QVector2D(brush.handle), snapToPixel), to;
    if (length() > 1) {
        ++i;
    }
    float segmentOffset = 0.0;

//    auto arePixelsNeighbors = [](const QPoint &point0, const QPoint &point1) {
//        return qAbs(point1.x() - point0.x()) <= 1 &&
//               qAbs(point1.y() - point0.y()) <= 1;
//    };
//    QVector<QPoint> pixels(3);
//    pixels.append(from.pos.toPoint());
//    if (pixels[pixels.length() - 1] == pixels[pixels.length() - 2] && pixels.length() > 1) {

//    }

    while (i < length()) {
        do {
            to = snapped(i, QVector2D(brush.handle), snapToPixel);
            ++i;
        } while (to == from && i < length());
//        const QRect segmentBounds = Stroke{from, to}.bounds(brush, scaleMin, scaleMax);
//        const bool drawSegment = clipRect.isValid() && clipRect.intersects(segmentBounds);
        const bool drawSegment = true;
        segmentOffset = applySegment(func, from, to, segmentOffset, !drawSegment);
        from = to;
    }
//    painter->restore();
}

QRect Stroke::bounds(const Brush &brush, const float scaleMin, const float scaleMax)
{
    QRectF bounds;
    for (auto point : *this) {
        bounds = bounds.united(brush.bounds(point.pos.toPointF(), scaleMin + point.pressure * (scaleMax - scaleMin)));
    }
    return bounds.toAlignedRect();
}
