/*
    Copyright 2017, Mitch Curtis

    This file is part of Slate.

    Slate is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Slate is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Slate. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TILSEETPROJECT_H
#define TILSEETPROJECT_H

#include <QHash>
#include <QObject>
#include <QPoint>
#include <QTemporaryDir>
#include <QUrl>
#include <QVector>

#include "project.h"
#include "slate-global.h"
#include "tile.h"
#include "tileset.h"

class SLATE_EXPORT TilesetProject : public Project
{
    Q_OBJECT
    Q_PROPERTY(int tilesWide READ tilesWide NOTIFY tilesWideChanged)
    Q_PROPERTY(int tilesHigh READ tilesHigh NOTIFY tilesHighChanged)
    Q_PROPERTY(int tileWidth READ tileWidth NOTIFY tileWidthChanged)
    Q_PROPERTY(int tileHeight READ tileHeight NOTIFY tileHeightChanged)
    Q_PROPERTY(QUrl tilesetUrl READ tilesetUrl NOTIFY tilesetUrlChanged)
    Q_PROPERTY(Tileset *tileset READ tileset NOTIFY tilesetChanged)

public:
    TilesetProject();
    ~TilesetProject() override;

    Type type() const override;
    int tilesWide() const;
    void setTilesWide(int tilesWide);
    int tilesHigh() const;
    void setTilesHigh(int tilesHigh);
    int tileWidth() const;
    int tileHeight() const;

    QSize tileSize() const;
    void setSize(const QSize &newSize) override;
    QSize size() const override;
    QSize pixelSize() const;
    QRect bounds() const override;
    QRect tileBounds() const;

    virtual QImage exportedImage() const override;

    QUrl tilesetUrl() const;
    Tileset *tileset() const;

    const TileObject *tileObjectAtPixel(const QPoint &pixel) const;
    TileObject *tileObjectAtPixel(const QPoint &pixel);
    QVector<int> tiles() const;

    QPointF pixelToTile(const QPointF pixel) const;
    QPoint pixelToTile(const QPoint pixel) const;

    bool isTilePosWithinBounds(const QPoint &tilePos) const;
    int tileIndexAt(const QPoint &tilePos) const;
    void setTileIndexAt(const QPoint &tilePos, const int index);
    const TileObject *tileObjectAtIndex(const int index) const;
    TileObject *tileObjectAtIndex(const int index);
    const TileObject *tileObjectAt(const QPoint &tilePos) const;
    TileObject *tileObjectAt(const QPoint &tilePos);

    Q_INVOKABLE TileObject *tilesetTileAt(int xInPixels, int yInPixels);
    Q_INVOKABLE void duplicateTile(TileObject *sourceTile, int xInPixels, int yInPixels);
    Q_INVOKABLE void rotateTileCounterClockwise(TileObject *tile);
    Q_INVOKABLE void rotateTileClockwise(TileObject *tile);

    QPoint tileIdToTilePos(int tileId) const;
    TileObject *tilesetTileAtTilePos(const QPoint &tilePos) const;
    TileObject *tilesetTileAtId(int id);

    // Sets all tiles to -1.
    void clearTiles();

signals:
    void tilesWideChanged();
    void tilesHighChanged();
    void tileWidthChanged();
    void tileHeightChanged();
    void tilesetUrlChanged();
    void tilesetChanged(Tileset *oldTileset, Tileset *newTileset);
    void tilesCleared();

public slots:
    void createNew(QUrl tilesetUrl, int tileWidth, int tileHeight,
        int tilesetTilesWide, int tilesetTilesHigh,
        int canvasTilesWide, int canvasTilesHigh,
        bool transparentBackground);

protected:
    void doLoad(const QUrl &url) override;
    void doClose() override;
    void doSaveAs(const QUrl &url) override;

private:
    friend class ChangeTileCanvasSizeCommand;

    int tileIdFromPosInTileset(int x, int y) const;
    int tileIdFromTilePosInTileset(int column, int row) const;

    void createTilesetTiles(int tilesetTilesWide, int tilesetTilesHigh);
    void setTileWidth(int tileWidth);
    void setTileHeight(int tileHeight);
    void setTilesetUrl(const QUrl &tilesetUrl);
    void setTileset(Tileset *tileset);
    void changeSize(const QSize &size, const QVector<int> &tiles = QVector<int>());

    int mTilesWide;
    int mTilesHigh;
    int mTileWidth;
    int mTileHeight;
    QUrl mTilesetUrl;
    QVector<int> mTiles;
    QHash<int, TileObject*> mTileDatabase;
    QImage mTileMapImage;
    Tileset* mTileset;

    static const QImage::Format TileMapImageFormat = QImage::Format_ARGB32;

    typedef uint16_t TileIndex;
    typedef union {
        uint32_t value;
        struct {
            TileIndex index;
            uint8_t theme;
            uint8_t attribs;
        } parts;
    } TileValue;

    static const TileIndex TileIndexInvalid = 0xffffu;
    static const TileIndex TileIndexMax = TileIndexInvalid - 1;

    static const uint32_t TileMaskIndex = 0xffffu;
    static const uint32_t TileMaskTheme = 0xffu;
    static const uint32_t TileMaskThemeShift = 16;
    static const uint32_t TileMaskAttribs = 0xffu;
    static const uint32_t TileMaskAttribsShift = 24;

    inline TileIndex tileIndex(const TileValue tile) { return tile.value & TileMaskIndex; }

    inline uint8_t tileTheme(const TileValue tile) { return (tile.value >> TileMaskThemeShift) & TileMaskTheme; }

    enum class TileAttribs : uint8_t {
        FlipH     = 0b00000001u,
        FlipV     = 0b00000010u,
        Transpose = 0b00000100u,
        Priority  = 0b00001000u,
    };
    inline uint8_t tileAttribs(const TileValue tile) { return (tile.value >> TileMaskAttribsShift) & TileMaskAttribs; }

    inline TileValue tileValue(const TileIndex index, const uint8_t theme, const uint8_t attribs) {
        return {(index & TileMaskIndex) |
               ((theme & TileMaskTheme) << TileMaskThemeShift) |
               ((attribs & TileMaskAttribs) << TileMaskAttribsShift)};
     }
    void TileValueSetIndex(TileValue *const tile, const TileIndex index) {
        tile->value = (tile->value | ~TileMaskIndex) | (index & TileMaskIndex);
    }
    void TileValueSetTheme(TileValue *const tile, const uint8_t theme) {
        tile->value = (tile->value | ~(TileMaskTheme << TileMaskThemeShift)) | ((theme & TileMaskTheme) << TileMaskThemeShift);
    }
    void TileValueSetAttribs(TileValue *const tile, const uint8_t attribs) {
        tile->value = (tile->value | ~(TileMaskAttribs << TileMaskAttribsShift)) | ((attribs & TileMaskAttribs) << TileMaskAttribsShift);
    }

    inline bool tileIsValid(const TileValue tile) { return tileIndex(tile) != TileIndexInvalid; }
};

#endif // TILSEETPROJECT_H
