#include "palettemodel.h"

#include <QDebug>

PaletteModel::PaletteModel(const QImage &image, const QSize swatchSize, const int columns, const int size, QObject *const parent) :
    QAbstractListModel(parent),
    mImage(image),
    mSwatchSize(swatchSize),
    mColumns(columns),
    mSize(size)
{
    Q_ASSERT(!image.isNull());
    Q_ASSERT(!swatchSize.isEmpty());
    Q_ASSERT(columns > 0);
}

PaletteModel::PaletteModel(QImage::Format format, const QSize swatchSize, const int columns, const int size, QObject *const parent) :
    PaletteModel(QImage(QSize(columns, (size + columns - 1) / columns), format), swatchSize, columns, size, parent)
{
}

const QImage &PaletteModel::image() const
{
    return mImage;
}

QSize PaletteModel::swatchSize() const
{
    return mSwatchSize;
}

void PaletteModel::setSwatchSize(const QSize swatchSize)
{
    Q_ASSERT(!swatchSize.isEmpty());

    if (mSwatchSize == swatchSize)
        return;

    mSwatchSize = swatchSize;
    emit swatchSizeChanged(mSwatchSize);
}

int PaletteModel::columns() const
{
    return mColumns;
}

void PaletteModel::setColumns(const int columns)
{
    Q_ASSERT(columns > 0);

    if (mColumns == columns)
        return;

    mColumns = columns;
    emit columnsChanged(mColumns);
}

int PaletteModel::size() const
{
    return mSize;
}

void PaletteModel::setColour(const int index, const QColor &colour)
{
    mImage.setPixelColor(indexToCoord(index), colour);
}

QColor PaletteModel::colour(const int index) const
{
    return mImage.pixelColor(indexToCoord(index));
}

void PaletteModel::setSize(const int size)
{
    if (mSize == size)
        return;

    mSize = size;
    emit sizeChanged(mSize);
}

int PaletteModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return size();
    else
        return 0;
}

QVariant PaletteModel::data(const QModelIndex &index, const int role) const
{
    if (index.row() >= 0 && index.row() < rowCount(index.parent())) {
        if (role == ColourRole)
            return colour(index.row());
    }

    return QVariant();
}

QHash<int, QByteArray> PaletteModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {ColourRole, "colour"}
    };
    return roles;
}
