#ifndef BRUSH_H
#define BRUSH_H

#include <QObject>
#include <QImage>
#include <QTransform>
#include <QDebug>

#include "slate-global.h"

class QPainter;

struct SLATE_EXPORT Brush {
    Q_GADGET

public:
    enum Type {
        SquareType,
        CircleType,
        ImageType,
    };
    Q_ENUM(Type)

    Brush(const Brush::Type type = SquareType, const QSizeF &size = {1, 1}, const qreal angle = 0.0, const qreal hardness = 1.0, const qreal opacity = 1.0, const QImage &image = QImage(), const QPointF handle = {0.5, 0.5});
    Brush(const Brush &other);

    bool operator==(const Brush &other) const;
    bool operator!=(const Brush &other) const;
    Brush &operator=(const Brush &other);

    Type type;
    QSizeF size;
    qreal angle;
    qreal hardness;
    qreal opacity;
    QImage image;
    bool singleColour;
    QPointF handle;
};

inline QDebug operator<<(QDebug debug, const Brush &brush)
{
    debug.nospace() << "Brush(" << brush.type << ", " << brush.size << ", " << brush.angle << ", " << brush.hardness << ", " << brush.opacity << ", " << brush.image << ", " << brush.handle << ")";
    return debug.space();
}

class SLATE_EXPORT BrushManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(Brush::Type type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QSizeF size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(qreal angle READ angle WRITE setAngle NOTIFY angleChanged)
    Q_PROPERTY(qreal hardness READ hardness WRITE setHardness NOTIFY hardnessChanged)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)
    Q_PROPERTY(bool singleColour READ singleColour WRITE setSingleColour NOTIFY singleColourChanged)
    Q_PROPERTY(QPointF handle READ handle WRITE setHandle NOTIFY handleChanged)

public:
    BrushManager(Brush *const brush = nullptr, QObject *const parent = nullptr);
    BrushManager(const BrushManager &other);

    bool operator==(const BrushManager &other) const;
    bool operator!=(const BrushManager &other) const;
    BrushManager &operator=(const BrushManager &other);

    QTransform transform() const;

    QRectF bounds(const QPointF pos = {0.0, 0.0}, const qreal scale = 1.0, const qreal rotation = 0.0) const;

    void draw(QPainter *const painter, const QColor &colour, const QPointF pos = {0.0, 0.0}, const qreal scale = 1.0, const qreal rotation = 0.0) const;

    const Brush *brush() const;
    Brush::Type type() const;
    QSizeF size() const;
    qreal angle() const;
    qreal hardness() const;
    qreal opacity() const;
    QImage image() const;
    bool singleColour() const;
    QPointF handle() const;

public slots:
    void setBrush(Brush *const brush);
    void setType(Brush::Type type);
    void setSize(QSizeF size);
    void setAngle(qreal angle);
    void setHardness(qreal hardness);
    void setOpacity(qreal opacity);
    void setImage(QImage image);
    void setSingleColour(bool singleColour);
    void setHandle(QPointF handle);

signals:
    void brushChanged(const Brush *brush);
    void typeChanged(Brush::Type type);
    void sizeChanged(QSizeF size);
    void angleChanged(qreal angle);
    void hardnessChanged(qreal hardness);
    void opacityChanged(qreal opacity);
    void imageChanged(QImage image);
    void singleColourChanged(bool singleColour);
    void handleChanged(QPointF handle);

protected:
    Brush *mBrush;
};

#endif // BRUSH_H
