#include "brush.h"

#include <QPainter>
#include <QtMath>
#include <QSet>

Brush::Brush(const Brush::Type type, const QSizeF &size , const qreal angle, const qreal hardness, const qreal opacity, const QImage &image, const QPointF handle) :
     type(type),
     size(size),
     angle(angle),
     hardness(hardness),
     opacity(opacity),
     image(image),
     handle(handle)
{
}

Brush::Brush(const Brush &other) :
    type(other.type),
    size(other.size),
    angle(other.angle),
    hardness(other.hardness),
    opacity(other.opacity),
    image(other.image),
    handle(other.handle)
{
}

bool Brush::operator==(const Brush &other) const
{
    return type == other.type &&
        size == other.size &&
        qFuzzyCompare(angle, other.angle) &&
        qFuzzyCompare(hardness, other.hardness) &&
        qFuzzyCompare(opacity, other.opacity) &&
        image == other.image &&
        handle == other.handle;
}

bool Brush::operator!=(const Brush &other) const
{
    return !(*this == other);
}

Brush &Brush::operator=(const Brush &other)
{
    type = other.type;
    size = other.size;
    angle = other.angle;
    hardness = other.hardness;
    opacity = other.opacity;
    image = other.image;
    handle = other.handle;
    return *this;
}

QTransform Brush::transform() const
{
    QTransform transform;
    transform.translate(-handle.x(), -handle.y());
    return transform;
}

QRectF Brush::bounds(const QPointF pos, const qreal scale, const qreal rotation) const
{
    return QRectF(QPointF(-handle.x() * size.width(), -handle.y() * size.height()) * scale, size * scale).translated(pos);
}

BrushManager::BrushManager(Brush *const brush, QObject *const parent) :
    QObject(parent),
    mBrush(brush)
{
}

BrushManager::BrushManager(const BrushManager &other) :
    QObject(),
    mBrush(other.mBrush)
{
}

bool BrushManager::operator==(const BrushManager &other) const
{
    return mBrush == other.mBrush;
}

bool BrushManager::operator!=(const BrushManager &other) const
{
    return !(*this == other);
}

BrushManager &BrushManager::operator=(const BrushManager &other)
{
    mBrush = other.mBrush;
    return *this;
}

const Brush *BrushManager::brush() const
{
    return mBrush;
}

Brush::Type BrushManager::type() const
{
    return mBrush->type;
}

QSizeF BrushManager::size() const
{
    return mBrush->size;
}

qreal BrushManager::angle() const
{
    return mBrush->angle;
}

qreal BrushManager::hardness() const
{
    return mBrush->hardness;
}

qreal BrushManager::opacity() const
{
    return mBrush->opacity;
}

QImage BrushManager::image() const
{
    return mBrush->image;
}

bool BrushManager::singleColour() const
{
    return mBrush->singleColour;
}

QPointF BrushManager::handle() const
{
    return mBrush->handle;
}

void BrushManager::setBrush(Brush *const brush)
{
    if (mBrush == brush)
        return;

    mBrush = brush;
    emit brushChanged(mBrush);
}

void BrushManager::setType(Brush::Type type)
{
    if (mBrush->type == type)
        return;

    mBrush->type = type;
    emit typeChanged(mBrush->type);
}

void BrushManager::setSize(QSizeF size)
{
    if (mBrush->size == size)
        return;

    mBrush->size = size;
    emit sizeChanged(mBrush->size);
}

void BrushManager::setAngle(qreal angle)
{
    if (qFuzzyCompare(mBrush->angle, angle))
        return;

    mBrush->angle = angle;
    emit angleChanged(mBrush->angle);
}

void BrushManager::setHardness(qreal hardness)
{
    if (qFuzzyCompare(mBrush->hardness, hardness))
        return;

    mBrush->hardness = hardness;
    emit hardnessChanged(mBrush->hardness);
}

void BrushManager::setOpacity(qreal opacity)
{
    if (qFuzzyCompare(mBrush->opacity, opacity))
        return;

    mBrush->opacity = opacity;
    emit opacityChanged(mBrush->opacity);
}

void BrushManager::setImage(QImage image)
{
    if (mBrush->image == image)
        return;

    mBrush->image = image;
    emit imageChanged(mBrush->image);
}

void BrushManager::setSingleColour(bool singleColour)
{
    if (mBrush->singleColour == singleColour)
        return;

    mBrush->singleColour = singleColour;
    emit singleColourChanged(mBrush->singleColour);
}

void BrushManager::setHandle(QPointF handle)
{
    if (mBrush->handle == handle)
        return;

    mBrush->handle = handle;
    emit handleChanged(mBrush->handle);
}
