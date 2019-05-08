#ifndef STROKE_H
#define STROKE_H

#include <QVector2D>
#include <QVector>
#include <QPainter>
#include <QtMath>
#include <functional>
#include <QQuaternion>

#include "brush.h"

struct StrokePoint {
    bool operator==(const StrokePoint &other) const;
    bool operator!=(const StrokePoint &other) const;

    StrokePoint snapped(const QVector2D snapOffset = {0.0, 0.0}) const;

    QPoint pixel() const;

    static StrokePoint lerp(const StrokePoint &from, const StrokePoint &to, const float pos);

    QVector2D pos alignas(8);
    float pressure alignas(8);
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

    static float applySegment(std::function<void(const StrokePoint &)> render, const StrokePoint &from, const StrokePoint &to, const float stepOffset = 0.0, const bool stepOffsetOnly = false);

    StrokePoint snapped(const int index, const QVector2D snapOffset = {0.0, 0.0}, const bool snapToPixel = true) const;

    void apply(std::function<void(const StrokePoint &)> render, const Brush &brush, const QRect &clipRect = QRect(), const bool snapToPixel = false) const;

    QRect bounds(const Brush &brush, const float scaleMin, const float scaleMax);
};

inline QDebug operator<<(QDebug debug, const Stroke &stroke)
{
    debug.nospace() << "Stroke(";
    for (int i = 0; i < stroke.size(); ++i) {
        debug.nospace() << (i != 0 ? ", " : "") << "(" << stroke[i].pos.x() << ", " << stroke[i].pos.y() << ")";
    }
    debug.nospace() << ")";
    return debug.space();
}

#endif // STROKE_H
