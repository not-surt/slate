#include "tilesetmodel.h"

#include <QDebug>

TileSetModel::TileSetModel(const QImage &image, const QSize tileSize, const int columns, const int size, QObject *const parent) :
    QAbstractListModel(parent),
    mImage(image),
    mTileSize(tileSize),
    mColumns(columns),
    mSize(size)
{
    Q_ASSERT(!image.isNull());
    Q_ASSERT(!tileSize.isEmpty());
    Q_ASSERT(columns > 0);
}

TileSetModel::TileSetModel(QImage::Format format, const QSize tileSize, const int columns, const int size, QObject *const parent) :
    TileSetModel(QImage(QSize(tileSize.width() * columns, tileSize.height()), format), tileSize, columns, size, parent)
{
}

const QImage &TileSetModel::image() const
{
    return mImage;
}

QImage &TileSetModel::image()
{
    return mImage;
}

QSize TileSetModel::tileSize() const
{
    return mTileSize;
}

void TileSetModel::setTileSize(const QSize tileSize)
{
    Q_ASSERT(!tileSize.isEmpty());

    if (mTileSize == tileSize)
        return;

    mTileSize = tileSize;
    emit tileSizeChanged(mTileSize);
}

int TileSetModel::columns() const
{
    return mColumns;
}

void TileSetModel::setColumns(const int columns)
{
    Q_ASSERT(columns > 0);

    if (mColumns == columns)
        return;

    mColumns = columns;
    emit columnsChanged(mColumns);
}

int TileSetModel::size() const
{
    return mSize;
}

QImage TileSetModel::tileImage(const int index) const
{
    const QRect rect = tileCoordRect(tileCoordFromIndex(index));
    return mImage.copy(rect);
}

QRect TileSetModel::tileRect() const
{
    return QRect(QPoint(0, 0), mTileSize);
}

QPoint TileSetModel::tileCoordPos(const QPoint &tileCoord) const
{
    return QPoint(tileCoord.x() * mTileSize.width(), tileCoord.y() * mTileSize.height());
}

QRect TileSetModel::tileCoordRect(const QPoint &tileCoord) const
{
    return QRect(tileCoordPos(tileCoord), mTileSize);
}

QSize TileSetModel::imageSizeTiles() const
{
    return QSize(mColumns, rows());
}

QRect TileSetModel::imageRectTiles() const
{
    return QRect(QPoint(0, 0), imageSizeTiles());
}

QSize TileSetModel::imageSize() const
{
    const QSize sizeTiles = imageSizeTiles();
    return QSize(sizeTiles.width() * tileSize().width(), sizeTiles.height() * tileSize().height());
}

QRect TileSetModel::imageRect() const
{
    return QRect(QPoint(0, 0), imageSize());
}

void TileSetModel::setSize(const int size)
{
    if (mSize == size)
        return;

    mSize = size;
    emit sizeChanged(mSize);
}

int TileSetModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return size();
    else
        return 0;
}

QVariant TileSetModel::data(const QModelIndex &index, const int role) const
{
    if (index.row() >= 0 && index.row() < rowCount(index.parent())) {
        if (role == ImageRole)
            return tileImage(index.row());
    }

    return QVariant();
}

QHash<int, QByteArray> TileSetModel::roleNames() const
{
    static const QHash<int, QByteArray> roles{
        {ImageRole, "tileImage"}
    };
    return roles;
}
