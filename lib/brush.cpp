#include "brush.h"

#include <QPainter>
#include <QtMath>
#include <QSet>

Brush::Brush(const Brush::Type type, const QSizeF &size , const qreal angle, const qreal hardness, const qreal opacity, const QImage &image, const QPointF handle) :
     mType(type),
     mSize(size),
     mAngle(angle),
     mHardness(hardness),
     mOpacity(opacity),
     mImage(image),
     mHandle(handle)
{
}

Brush::Brush(const Brush &other) :
    mType(other.mType),
    mSize(other.mSize),
    mAngle(other.mAngle),
    mHardness(other.mHardness),
    mOpacity(other.mOpacity),
    mImage(other.mImage),
    mHandle(other.mHandle)
{
}

bool Brush::operator==(const Brush &other) const
{
    return mType == other.mType &&
        mSize == other.mSize &&
        qFuzzyCompare(mAngle, other.mAngle) &&
        qFuzzyCompare(mHardness, other.mHardness) &&
        qFuzzyCompare(mOpacity, other.mOpacity) &&
        mImage == other.mImage &&
        mHandle == other.mHandle;
}

bool Brush::operator!=(const Brush &other) const
{
    return !(*this == other);
}

Brush &Brush::operator=(const Brush &other)
{
    mType = other.mType;
    mSize = other.mSize;
    mAngle = other.mAngle;
    mHardness = other.mHardness;
    mOpacity = other.mOpacity;
    mImage = other.mImage;
    mHandle = other.mHandle;
    return *this;
}

BrushManager::BrushManager(const Brush &brush, QObject *const parent) :
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

QTransform BrushManager::transform() const
{
    QTransform transform;
    transform.translate(-mBrush.mHandle.x(), -mBrush.mHandle.y());
    return transform;
}

QRectF BrushManager::bounds(const QPointF pos, const qreal scale, const qreal rotation) const
{
    return QRectF(QPointF(-mBrush.mHandle.x() * mBrush.mSize.width(), -mBrush.mHandle.y() * mBrush.mSize.height()) * scale, mBrush.mSize * scale).translated(pos);
}

const Brush &BrushManager::brush() const
{
    return mBrush;
}

Brush::Type BrushManager::type() const
{
    return mBrush.mType;
}

QSizeF BrushManager::size() const
{
    return mBrush.mSize;
}

qreal BrushManager::angle() const
{
    return mBrush.mAngle;
}

qreal BrushManager::hardness() const
{
    return mBrush.mHardness;
}

qreal BrushManager::opacity() const
{
    return mBrush.mOpacity;
}

QImage BrushManager::image() const
{
    return mBrush.mImage;
}

bool BrushManager::singleColour() const
{
    return mBrush.mSingleColour;
}

QPointF BrushManager::handle() const
{
    return mBrush.mHandle;
}

void BrushManager::setBrush(const Brush &brush)
{
    if (mBrush == brush)
        return;

    mBrush = brush;
    emit brushChanged(mBrush);
}

void BrushManager::setType(Brush::Type type)
{
    if (mBrush.mType == type)
        return;

    mBrush.mType = type;
    emit typeChanged(mBrush.mType);
}

void BrushManager::setSize(QSizeF size)
{
    if (mBrush.mSize == size)
        return;

    mBrush.mSize = size;
    emit sizeChanged(mBrush.mSize);
}

void BrushManager::setAngle(qreal angle)
{
    if (qFuzzyCompare(mBrush.mAngle, angle))
        return;

    mBrush.mAngle = angle;
    emit angleChanged(mBrush.mAngle);
}

void BrushManager::setHardness(qreal hardness)
{
    if (qFuzzyCompare(mBrush.mHardness, hardness))
        return;

    mBrush.mHardness = hardness;
    emit hardnessChanged(mBrush.mHardness);
}

void BrushManager::setOpacity(qreal opacity)
{
    if (qFuzzyCompare(mBrush.mOpacity, opacity))
        return;

    mBrush.mOpacity = opacity;
    emit opacityChanged(mBrush.mOpacity);
}

void BrushManager::setImage(QImage image)
{
    if (mBrush.mImage == image)
        return;

    mBrush.mImage = image;
    emit imageChanged(mBrush.mImage);
}

void BrushManager::setSingleColour(bool singleColour)
{
    if (mBrush.mSingleColour == singleColour)
        return;

    mBrush.mSingleColour = singleColour;
    emit singleColourChanged(mBrush.mSingleColour);
}

void BrushManager::setHandle(QPointF handle)
{
    if (mBrush.mHandle == handle)
        return;

    mBrush.mHandle = handle;
    emit handleChanged(mBrush.mHandle);
}
