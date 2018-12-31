#ifndef PALETTEMODEL_H
#define PALETTEMODEL_H

#include <QAbstractListModel>
#include <QImage>

#include "slate-global.h"

class SLATE_EXPORT PaletteModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QImage image READ image)
    Q_PROPERTY(QSize swatchSize READ swatchSize WRITE setSwatchSize NOTIFY swatchSizeChanged)
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    Q_PROPERTY(int size READ size WRITE setSize NOTIFY sizeChanged)

public:
    enum PaletteModelRoles {
        ColourRole = Qt::UserRole + 1,
    };

    PaletteModel(const QImage &image, const QSize swatchSize, const int columns, const int size, QObject *const parent = nullptr);
    PaletteModel(QImage::Format format, const QSize swatchSize, const int columns, const int size = 0, QObject *const parent = nullptr);

    const QImage &image() const;
    QSize swatchSize() const;
    int columns() const;
    int size() const;

    void setColour(const int index, const QColor &colour);
    QColor colour(const int index) const;

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, const int role) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

public slots:
    void setSwatchSize(const QSize swatchSize);
    void setColumns(const int columns);
    void setSize(const int size);

signals:
    void swatchSizeChanged(const QSize swatchSize);
    void columnsChanged(const int columns);
    void sizeChanged(const int size);

protected:
    inline QPoint indexToCoord(const int index) const {
        return QPoint(index % mColumns, index / mColumns);
    }
    inline int indexToCoord(const QPoint coord) const {
        return coord.y() * mColumns + coord.x();
    }

    QImage mImage;
    QSize mSwatchSize;
    int mColumns;
    int mSize;
};

#endif // PALETTEMODEL_H
