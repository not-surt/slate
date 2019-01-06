#ifndef STROKE_H
#define STROKE_H

#include <QPointF>
#include <QVector>
#include <QPainter>
#include <QtMath>
#include <functional>

#include "brush.h"

struct StrokePoint {
    bool operator==(const StrokePoint &other) const;
    bool operator!=(const StrokePoint &other) const;

    StrokePoint snapped(const QPointF snapOffset = {0.0, 0.0}) const;

    QPoint pixel() const;

    QPointF pos;
    qreal pressure;
};

inline QDebug operator<<(QDebug debug, const StrokePoint &point)
{
    debug.nospace() << "StrokePoint(" << point.pos << ", " << point.pressure << ")";
    return debug.space();
}

class Stroke : public QVector<StrokePoint> {
public:
    using QVector::QVector;
    Stroke(const QVector<StrokePoint> &vector) : QVector<StrokePoint>(vector) {}

    static qreal strokeSegment(std::function<void(const StrokePoint &)> render, QPainter *const painter, const Brush &brush, const QColor &colour, const StrokePoint &point0, const StrokePoint &point1, const qreal scaleMin, const qreal scaleMax, const qreal stepOffset = 0.0, const bool stepOffsetOnly = false);

    StrokePoint snapped(const int index, const QPointF snapOffset = {0.0, 0.0}, const bool snapToPixel = true) const;

    void draw(std::function<void(const StrokePoint &)> render, QPainter *const painter, const Brush &brush, const qreal scaleMin, const qreal scaleMax, const QColor &colour, const QPainter::CompositionMode mode, const bool snapToPixel = false) const;

    QRect bounds(const Brush &brush, const qreal scaleMin, const qreal scaleMax);
};

#endif // STROKE_H
