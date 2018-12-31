#ifndef IMAGEPAINTER_H
#define IMAGEPAINTER_H

#include <QQuickPaintedItem>

#include <QImage>

#include "slate-global.h"

class QPainter;

class SLATE_EXPORT ImagePainter : public QQuickPaintedItem
{
    Q_OBJECT

    Q_PROPERTY(QImage image READ image WRITE setImage NOTIFY imageChanged)

public:
    ImagePainter(QQuickItem *const parent = nullptr);

    QImage image() const;

    virtual void paint(QPainter *const painter) override;

public slots:
    void setImage(const QImage &image);

signals:
    void imageChanged(const QImage &image);

private:
    QImage mImage;
};

#endif // IMAGEPAINTER_H
