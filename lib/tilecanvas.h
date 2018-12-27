/*
    Copyright 2016, Mitch Curtis

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

#ifndef TILECANVAS_H
#define TILECANVAS_H

#include <QObject>
#include <QImage>
#include <QQuickPaintedItem>
#include <QStack>
#include <QWheelEvent>

#include <QUndoStack>

#include "imagecanvas.h"
#include "slate-global.h"

class Project;
class TileObject;
class Tileset;
class TilesetProject;

class SLATE_EXPORT TileCanvas : public ImageCanvas
{
    Q_OBJECT
    Q_PROPERTY(QPoint cursorSceneTileCoord READ cursorSceneTileCoord NOTIFY cursorScenePosChanged)
    Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
    Q_PROPERTY(TileObject *penTile READ penTile WRITE setPenTile NOTIFY penTileChanged)
    Q_PROPERTY(bool tileIndicesVisible READ tileIndicesVisible WRITE setTileIndicesVisible NOTIFY tileIndicesVisibleChanged)

public:
    enum Mode {
        PixelMode,
        TileMode
    };

    Q_ENUM(Mode)

    TileCanvas();
    ~TileCanvas() override;

    virtual const QImage *currentProjectImage() const override;
    virtual QImage *currentProjectImage() override;

    virtual const QImage *imageForLayerAt(int layerIndex) const override;
    virtual QImage *imageForLayerAt(int layerIndex) override;

    virtual void paintContent(QPainter *const painter, const QRect rect) override;
    virtual void paintOverlay(QPainter *const painter, const QRect rect) const override;

    QPointF pressSceneTilePos() const;
    QPoint pressSceneTileCoord() const;
    QPoint pressSceneTileCorner() const;

    QPointF cursorSceneTilePos() const;
    QPoint cursorSceneTileCoord() const;
    QPoint cursorSceneTileCorner() const;

    Mode mode() const;
    void setMode(const Mode &mode);

    bool tileIndicesVisible() const;
    void setTileIndicesVisible(bool tileIndicesVisible);

    TileObject *penTile() const;
    void setPenTile(TileObject *penTile);

    QPoint scenePosToTilePixelPos(const QPoint &scenePos) const;
    QRect sceneRectToTileRect(const QRect &sceneRect) const;

    virtual SubImage getSubImage(const int index) const override;
    virtual QList<SubImageInstance> subImageInstancesInBounds(const QRect &bounds) const override;

signals:
    void modeChanged();
    void penTileChanged();
    void tileIndicesVisibleChanged();

public slots:
//    void createNew(int width, int height, const QColor &penBackgroundColour);
//    void save(QUrl url = QUrl());
//    void saveAs(const QUrl &url);
    void swatchLeft();
    void swatchRight();
    void swatchUp();
    void swatchDown();
    void onTilesetChanged(Tileset *oldTileset, Tileset *newTileset);

protected:
    void connectSignals() override;
    void disconnectSignals() override;
    bool supportsSelectionTool() const override;
    virtual QImage getComposedImage() override;

    void onLoadedChanged() override;

private:
    friend class ApplyTilePenCommand;
    friend class ApplyTileEraserCommand;
    friend class ApplyTileFillCommand;
    friend class ApplyTileCanvasPixelFillCommand;

    PixelCandidateData fillPixelCandidates() const;
    PixelCandidateData greedyFillPixelCandidates() const;

    struct TileCandidateData
    {
        QVector<QPoint> tilePositions;
        int previousTile;
        int newTileId;

        TileCandidateData() : previousTile(-1), newTileId(-1) {}
    };
    TileCandidateData fillTileCandidates() const;

    void applyCurrentTool(QUndoStack *const alternateStack = nullptr) override;
    void applyPixelPenTool(int layerIndex, const QPoint &scenePos, const QColor &colour) override;
    void applyTilePenTool(const QPoint &tilePos, int id);

    TilesetProject *mTilesetProject;

    // The position of the cursor in scene coordinates relative to the tile that it's in.
    int mCursorTilePixelX;
    int mCursorTilePixelY;
    QPoint mCursorSceneTile;
    Mode mMode;
    TileObject *mPenTile;
    bool mTileIndicesVisible;
};

#endif // TILECANVAS_H
