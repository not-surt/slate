#ifndef TILESETMODEL_H
#define TILESETMODEL_H

#include <QAbstractListModel>
#include <QImage>

#include "slate-global.h"

class SLATE_EXPORT TileSetModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QImage image READ image)
    Q_PROPERTY(QSize tileSize READ tileSize WRITE setTileSize NOTIFY tileSizeChanged)
    Q_PROPERTY(int columns READ columns WRITE setColumns NOTIFY columnsChanged)
    Q_PROPERTY(int size READ size WRITE setSize NOTIFY sizeChanged)

public:
    enum TileSetModelRoles {
        ImageRole = Qt::UserRole + 1,
    };

    TileSetModel(const QImage &image, const QSize tileSize, const int columns, const int size, QObject *const parent = nullptr);
    TileSetModel(QImage::Format format, const QSize tileSize, const int columns, const int size = 0, QObject *const parent = nullptr);

    const QImage &image() const;
    QImage &image();
    QSize tileSize() const;
    int columns() const;
    int size() const;

    inline int rows() const {
        return (mSize + mColumns - 1) / mColumns;
    }
    inline int indexFromTileCoord(const QPoint &tileCoord) const {
        Q_ASSERT(imageRectTiles().contains(tileCoord));
        const int index = tileCoord.y() * columns() + tileCoord.x();
        Q_ASSERT(index < mSize);
        return index;
    }
    inline QPoint tileCoordFromIndex(const int index) const {
        Q_ASSERT(index < mSize);
        const QPoint tileCoord = QPoint(index % mColumns, index / mColumns);
        Q_ASSERT(imageRectTiles().contains(tileCoord));
        return tileCoord;
    }

    QImage tileImage(const int index) const;
    QRect tileRect() const;
    QPoint tileCoordPos(const QPoint &tileCoord) const;
    QRect tileCoordRect(const QPoint &tileCoord) const;
    QSize imageSizeTiles() const;
    QRect imageRectTiles() const;
    QSize imageSize() const;
    QRect imageRect() const;

    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, const int role) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

public slots:
    void setTileSize(const QSize tileSize);
    void setColumns(const int columns);
    void setSize(const int size);

signals:
    void tileSizeChanged(const QSize tileSize);
    void columnsChanged(const int columns);
    void sizeChanged(const int size);

protected:
    QImage mImage;
    QSize mTileSize;
    int mColumns;
    int mSize;
};

#endif // TILESETMODEL_H
