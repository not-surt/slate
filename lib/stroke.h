#ifndef STROKE_H
#define STROKE_H

#include <QPointF>
#include <QVector>
#include <QPainter>
#include <QtMath>
#include <functional>
#include <QQuaternion>

#include "brush.h"

struct StrokePoint {
    bool operator==(const StrokePoint &other) const;
    bool operator!=(const StrokePoint &other) const;

    StrokePoint snapped(const QPointF snapOffset = {0.0, 0.0}) const;

    QPoint pixel() const;

    static StrokePoint lerp(const StrokePoint &from, const StrokePoint &to, const qreal pos);

    QPointF pos;
    qreal pressure;
//    QQuaternion quaternion;
};

inline QDebug operator<<(QDebug debug, const StrokePoint &point)
{
    debug.nospace() << "StrokePoint(" << point.pos << ", " << point.pressure << ")";
    return debug.space();
}

class Stroke : public QVector<StrokePoint> {
public:
    using QVector::QVector;
    Stroke(const QVector<StrokePoint> &vector = QVector<StrokePoint>()) : QVector<StrokePoint>(vector) {}

    static qreal applySegment(std::function<void(const StrokePoint &)> render, const StrokePoint &from, const StrokePoint &to, const qreal stepOffset = 0.0, const bool stepOffsetOnly = false);

    StrokePoint snapped(const int index, const QPointF snapOffset = {0.0, 0.0}, const bool snapToPixel = true) const;

    void apply(std::function<void(const StrokePoint &)> render, const Brush &brush, const QRect &clipRect = QRect(), const bool snapToPixel = false) const;

    QRect bounds(const Brush &brush, const qreal scaleMin, const qreal scaleMax);
};

#endif // STROKE_H
