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

#include "tilecanvas.h"

#include <QCursor>
#include <QLoggingCategory>
#include <QPainter>
#include <QQuickWindow>
#include <QtMath>

#include "applypixelfillcommand.h"
#include "applypixellinecommand.h"
#include "applytilecanvaspixelfillcommand.h"
#include "applytileerasercommand.h"
#include "applytilefillcommand.h"
#include "applytilepencommand.h"
#include "fillalgorithms.h"
#include "tileset.h"
#include "tilesetproject.h"
#include "utils.h"

TileCanvas::TileCanvas() :
    ImageCanvas (),
    mTilesetProject(nullptr),
    mCursorTilePixelX(0),
    mCursorTilePixelY(0),
    mMode(TileMode),
    mPenTile(nullptr),
    mTilePenPreview(false)
{
    qCDebug(lcImageCanvasLifecycle) << "constructing TileCanvas" << this;
}

TileCanvas::~TileCanvas()
{
    qCDebug(lcImageCanvasLifecycle) << "destructing TileCanvas" << this;
}

QImage *TileCanvas::currentProjectImage()
{
    return mTilesetProject->tileset()->image();
}

const QImage *TileCanvas::currentProjectImage() const
{
    return const_cast<TileCanvas *>(this)->currentProjectImage();
}

QImage *TileCanvas::imageForLayerAt(int layerIndex)
{
    Q_ASSERT(layerIndex == -1);
    return mTilesetProject->tileset()->image();
}

const QImage *TileCanvas::imageForLayerAt(int layerIndex) const
{
    return const_cast<TileCanvas *>(this)->imageForLayerAt(layerIndex);
}

TileCanvas::Mode TileCanvas::mode() const
{
    return mMode;
}

void TileCanvas::setMode(const Mode &mode)
{
    if (mode == mMode)
        return;

    mMode = mode;
    updateTilePenPreview();
    emit modeChanged();
}

Tile *TileCanvas::penTile() const
{
    return mPenTile;
}

void TileCanvas::setPenTile(Tile *penTile)
{
    if (penTile == mPenTile)
        return;

    mPenTile = penTile;
    emit penTileChanged();
}

int TileCanvas::cursorTilePixelX() const
{
    return mCursorTilePixelX;
}

void TileCanvas::setCursorTilePixelX(int cursorTilePixelX)
{
    if (cursorTilePixelX == mCursorTilePixelX)
        return;

    mCursorTilePixelX = cursorTilePixelX;
    emit cursorTilePixelXChanged();
}

int TileCanvas::cursorTilePixelY() const
{
    return mCursorTilePixelY;
}

void TileCanvas::setCursorTilePixelY(int cursorTilePixelY)
{
    if (cursorTilePixelY == mCursorTilePixelY)
        return;

    mCursorTilePixelY = cursorTilePixelY;
    emit cursorTilePixelYChanged();
}

void TileCanvas::reset()
{
//    mFirstPane.reset();
//    mSecondPane.reset();
//    setCurrentPane(nullptr);
//    mSplitter.setPosition(mFirstPane.size());
//    mSplitter.setPressed(false);
//    mSplitter.setHovered(false);
//    setCursorX(0);
//    setCursorY(0);
//    mCursorPaneX = 0;
//    mCursorPaneY = 0;
//    mCursorSceneX = 0;
//    mCursorSceneY = 0;
    setCursorTilePixelX(0);
    setCursorTilePixelY(0);
//    mContainsMouse = false;
//    mMouseButtonPressed = Qt::NoButton;
//    mPressPosition = QPoint(0, 0);
//    mCurrentPaneOffsetBeforePress = QPoint(0, 0);
//    setAltPressed(false);
//    mToolBeforeAltPressed = PenTool;
//    mSpacePressed = false;
//    mHasBlankCursor = false;

//    // Things that we don't want to set, as they
//    // don't really need to be reset each time:
//    // - mode
//    // - tool
//    // - toolSize

//    requestContentPaint();
    ImageCanvas::reset();
}

void TileCanvas::swatchLeft()
{
    QPoint tilePos = mTilesetProject->tileIdToTilePos(mPenTile->id());
    tilePos.setX(qMax(0, tilePos.x() - 1));
    Tile *newTile = mTilesetProject->tilesetTileAtTilePos(tilePos);
    setPenTile(newTile);
}

void TileCanvas::swatchRight()
{
    QPoint tilePos = mTilesetProject->tileIdToTilePos(mPenTile->id());
    tilePos.setX(qMin(tilePos.x() + 1, mTilesetProject->tileset()->tilesWide() - 1));
    Tile *newTile = mTilesetProject->tilesetTileAtTilePos(tilePos);
    setPenTile(newTile);
}

void TileCanvas::swatchUp()
{
    QPoint tilePos = mTilesetProject->tileIdToTilePos(mPenTile->id());
    tilePos.setY(qMax(0, tilePos.y() - 1));
    Tile *newTile = mTilesetProject->tilesetTileAtTilePos(tilePos);
    setPenTile(newTile);
}

void TileCanvas::swatchDown()
{
    QPoint tilePos = mTilesetProject->tileIdToTilePos(mPenTile->id());
    tilePos.setY(qMin(tilePos.y() + 1, mTilesetProject->tileset()->tilesHigh() - 1));
    Tile *newTile = mTilesetProject->tilesetTileAtTilePos(tilePos);
    setPenTile(newTile);
}

void TileCanvas::onTilesetChanged(Tileset *oldTileset, Tileset *newTileset)
{
    if (oldTileset) {
        disconnect(oldTileset, &Tileset::imageChanged, this, &TileCanvas::requestContentPaint);
    }

    if (newTileset) {
        connect(newTileset, &Tileset::imageChanged, this, &TileCanvas::requestContentPaint);
    }
}

void TileCanvas::connectSignals()
{
    ImageCanvas::connectSignals();

    mTilesetProject = qobject_cast<TilesetProject*>(mProject);
    Q_ASSERT_X(mTilesetProject, Q_FUNC_INFO, "Non-tileset project set on TileCanvas");

    connect(mTilesetProject, &TilesetProject::tilesCleared, this, &TileCanvas::requestContentPaint);
    connect(mTilesetProject, &TilesetProject::tilesetChanged, this, &TileCanvas::onTilesetChanged);

    setPenTile(mTilesetProject->tilesetTileAt(0, 0));

    // If the project already has this, we won't get the signal, so force it.
    if (mTilesetProject->tileset())
        onTilesetChanged(nullptr, mTilesetProject->tileset());
}

void TileCanvas::disconnectSignals()
{
    ImageCanvas::disconnectSignals();

    disconnect(mTilesetProject, &TilesetProject::tilesCleared, this, &TileCanvas::requestContentPaint);
    disconnect(mTilesetProject, &TilesetProject::tilesetChanged, this, &TileCanvas::onTilesetChanged);

    setPenTile(nullptr);

    mTilesetProject = nullptr;
}

void TileCanvas::toolChange()
{
    updateTilePenPreview();
}

bool TileCanvas::supportsSelectionTool() const
{
    return false;
}

QImage TileCanvas::getComposedImage()
{
    const QSize size(mTilesetProject->tilesWide() * mTilesetProject->tileWidth(), mTilesetProject->tilesHigh() * mTilesetProject->tileHeight());
    QImage image(size,  QImage::Format_ARGB32);
    image.fill(qRgba(0, 0, 0, 0));
    QPainter painter(&image);

    const QRect tilesRect{QPoint{0, 0}, QSize{mTilesetProject->tilesWide(), mTilesetProject->tilesHigh()}};

    for (int y = tilesRect.top(); y <= tilesRect.bottom(); ++y) {
        for (int x = tilesRect.left(); x <= tilesRect.right(); ++x) {
            const QPoint pos(x * mTilesetProject->tileWidth(), y * mTilesetProject->tileHeight());
            const Tile *tile = mTilesetProject->tileAtTilePos(QPoint(x, y));
            if (tile) {
                painter.drawImage(pos, *tile->tileset()->image(), tile->sourceRect());
            }
        }
    }

    return image;
}

TileCanvas::PixelCandidateData TileCanvas::fillPixelCandidates() const
{
    PixelCandidateData candidateData;

    const QPoint tilePos = QPoint(mCursorSceneX, mCursorSceneY);
    Tile *tile = mTilesetProject->tileAt(tilePos);
    if (!tile) {
        return candidateData;
    }

    const QPoint tilePixelPos = scenePosToTilePixelPos(tilePos);
    const QPoint tileTopLeftScenePos = tilePos - tilePixelPos;
    const QColor previousColour = tile->pixelColor(tilePixelPos);
    // Don't do anything if the colours are the same.
    if (previousColour == penColour()) {
        return candidateData;
    }

    QVector<QPoint> tilePixelPositions;
    tilesetPixelFloodFill(tile, tilePixelPos, previousColour, penColour(), tilePixelPositions);

    for (const QPoint &pixelPos : tilePixelPositions) {
        candidateData.scenePositions.append(tileTopLeftScenePos + pixelPos);
    }

    candidateData.previousColours.append(previousColour);
    return candidateData;
}

ImageCanvas::PixelCandidateData TileCanvas::greedyFillPixelCandidates() const
{
    // TODO
    return PixelCandidateData();
}

TileCanvas::TileCandidateData TileCanvas::fillTileCandidates() const
{
    TileCandidateData candidateData;

    const QPoint scenePos = QPoint(mCursorSceneX, mCursorSceneY);
    const Tile *tile = mTilesetProject->tileAt(scenePos);
    const int previousTileId = tile ? tile->id() : Tile::invalidId();
    const int newTileId = mPenTile ? mPenTile->id() : -1;
    // Don't do anything if the tiles are the same.
    if (newTileId == previousTileId) {
        return candidateData;
    }

    const int xTile = scenePos.x() / mTilesetProject->tileWidth();
    const int yTile = scenePos.y() / mTilesetProject->tileHeight();
    tilesetTileFloodFill(mTilesetProject, tile, QPoint(xTile, yTile), previousTileId, newTileId, candidateData.tilePositions);

    candidateData.previousTile = previousTileId;
    candidateData.newTileId = newTileId;
    return candidateData;
}

void TileCanvas::applyCurrentTool(QUndoStack *const alternateStack)
{
    if (areToolsForbidden())
        return;

    // Use image canvas tool if compatible
    if (mMode == PixelMode && QSet<Tool>{PenTool, EraserTool, EyeDropperTool}.contains(mTool)) {
        ImageCanvas::applyCurrentTool(alternateStack);
        return;
    }

    // For tool preview use alternate undo stack if given
    QUndoStack *const stack = alternateStack ? alternateStack : mProject->undoStack();
    QUndoCommand *command = nullptr;

    switch (mTool) {
    case PenTool: {
        if (mMode == TileMode) {
            const QPoint scenePos = QPoint(mCursorSceneX, mCursorSceneY);
            const Tile *tile = mTilesetProject->tileAt(scenePos);
            const int previousTileId = tile ? tile->id() : Tile::invalidId();
            const int newTileId = mPenTile ? mPenTile->id() : -1;
            // Don't do anything if the tiles are the same.
            if (newTileId == previousTileId) {
                return;
            }

            const int xTile = Utils::divFloor(scenePos.x(), mTilesetProject->tileWidth());
            const int yTile = Utils::divFloor(scenePos.y(), mTilesetProject->tileHeight());
            command = new ApplyTilePenCommand(this, QPoint(xTile, yTile), previousTileId, newTileId);
            command->setText(QLatin1String("TilePenTool"));
        }
        break;
    }
    case EyeDropperTool: {
        if (mMode == TileMode) {
            const QPoint tilePos = QPoint(mCursorSceneX, mCursorSceneY);
            Tile *tile = mTilesetProject->tileAt(tilePos);
            if (tile) {
                setPenTile(tile);
            }
        }
        break;
    }
    case EraserTool: {
        if (mMode == TileMode) {
            const QPoint scenePos = QPoint(mCursorSceneX, mCursorSceneY);
            const Tile *tile = mTilesetProject->tileAt(scenePos);
            const int previousTileId = tile ? tile->id() : Tile::invalidId();
            if (previousTileId == Tile::invalidId()) {
                return;
            }

            const int xTile = scenePos.x() / mTilesetProject->tileWidth();
            const int yTile = scenePos.y() / mTilesetProject->tileHeight();

            command = new ApplyTileEraserCommand(this, QPoint(xTile, yTile), previousTileId);
            command->setText(QLatin1String("PixelEraserTool"));
        }
        break;
    }
    case FillTool: {
        if (mMode == PixelMode) {
            const PixelCandidateData candidateData = fillPixelCandidates();
            if (candidateData.scenePositions.isEmpty()) {
                return;
            }

            command = new ApplyTileCanvasPixelFillCommand(this, candidateData.scenePositions, candidateData.previousColours.first(), penColour());
            command->setText(QLatin1String("PixelFillTool"));
        } else {
            const TileCandidateData candidateData = fillTileCandidates();
            if (candidateData.tilePositions.isEmpty()) {
                return;
            }

            command = new ApplyTileFillCommand(this, candidateData.tilePositions, candidateData.previousTile, candidateData.newTileId);
            command->setText(QLatin1String("TileFillTool"));
        }
        break;
    }
    default:
        break;
    }

    if (command) {
        if (alternateStack)
            alternateStack->push(command);
        else
            mProject->addChange(command);
    }
}

QPoint TileCanvas::scenePosToTilePixelPos(const QPoint &scenePos) const
{
    return QPoint(scenePos.x() % mTilesetProject->tileWidth(),
                  scenePos.y() % mTilesetProject->tileHeight());
}

QRect TileCanvas::sceneRectToTileRect(const QRect &sceneRect) const
{
    return QRect(QPoint(Utils::divFloor(sceneRect.left(), mTilesetProject->tileWidth()), Utils::divFloor(sceneRect.top(), mTilesetProject->tileHeight())),
                 QPoint(Utils::divFloor(sceneRect.right(), mTilesetProject->tileWidth()), Utils::divFloor(sceneRect.bottom(), mTilesetProject->tileHeight())));
}

ImageCanvas::SubImage TileCanvas::getSubImage(const int index) const
{
    const Tile *const tile = mTilesetProject->tilesetTileAtId(index);
    Q_ASSERT(tile);

    return {0, tile->sourceRect(), {0, 0}};
}

QList<ImageCanvas::SubImageInstance> TileCanvas::subImageInstancesInBounds(const QRect &bounds) const
{
    const QRect tileRect = sceneRectToTileRect(bounds);
    QList<ImageCanvas::SubImageInstance> instances;
    for (int y = tileRect.top(); y <= tileRect.bottom(); ++y) {
        for (int x = tileRect.left(); x <= tileRect.right(); ++x) {
            const Tile *const tile = mTilesetProject->tileAtTilePos({x, y});
            if (tile) {
                instances.append({tile->id(), {x * mTilesetProject->tileWidth(), y * mTilesetProject->tileHeight()}});
            }
        }
    }
    return instances;
}

// This function actually operates on the image.
void TileCanvas::applyPixelPenTool(int layerIndex, const QPoint &scenePos, const QColor &colour, bool markAsLastRelease)
{
    Q_ASSERT(layerIndex == -1);

    Tile *tile = mTilesetProject->tileAt(scenePos);
    Q_ASSERT_X(tile, Q_FUNC_INFO, qPrintable(QString::fromLatin1(
        "No tile at scene pos {%1, %2}").arg(scenePos.x()).arg(scenePos.y())));
    const QPoint pixelPos = scenePosToTilePixelPos(scenePos);
    const QPoint tilsetPixelPos = tile->sourceRect().topLeft() + pixelPos;
    mTilesetProject->tileset()->setPixelColor(tilsetPixelPos.x(), tilsetPixelPos.y(), colour);
    if (markAsLastRelease)
        mLastPixelPenPressScenePosition = scenePos;
    requestContentPaint();
}

void TileCanvas::applyTilePenTool(const QPoint &tilePos, int id)
{
    mTilesetProject->setTileAtPixelPos(tilePos, id);
    requestContentPaint();
}

void TileCanvas::updateCursorPos(const QPoint &eventPos)
{
    setCursorX(eventPos.x());
    setCursorY(eventPos.y());
    // Don't change current panes if panning, as the mouse position should
    // be allowed to go outside of the original pane.
    if (!mSpacePressed) {
        setCurrentPane(hoveredPane(eventPos));
    }

    const int firstPaneWidth = paneWidth(0);
    const bool inFirstPane = eventPos.x() <= firstPaneWidth;
    mCursorPaneX = eventPos.x() - (!inFirstPane ? firstPaneWidth : 0);
    mCursorPaneY = eventPos.y();

    const QSize zoomedTileSize = mCurrentPane->zoomedSize(mTilesetProject->tileSize());
    if (zoomedTileSize.isEmpty()) {
        mCursorSceneFX = -1;
        mCursorSceneFY = -1;
        mCursorSceneX = -1;
        mCursorSceneY = -1;
        setCursorTilePixelX(-1);
        setCursorTilePixelY(-1);
        // We could do this once at the beginning of the function, but we
        // try to avoid unnecessary property changes.
        setCursorPixelColour(QColor(Qt::black));
        return;
    }

    // We need the position as floating point numbers so that pen sizes > 1 work properly.
    const int zoomLevel = mCurrentPane->integerZoomLevel();
    mCursorSceneFX = qreal(mCursorPaneX - mCurrentPane->integerOffset().x()) / mTilesetProject->tileWidth() / zoomLevel * mTilesetProject->tileWidth();
    mCursorSceneFY = qreal(mCursorPaneY - mCurrentPane->integerOffset().y()) / mTilesetProject->tileHeight() / zoomLevel * mTilesetProject->tileHeight();
    mCursorSceneX = mCursorSceneFX;
    mCursorSceneY = mCursorSceneFY;

    if (mCursorSceneX < 0 || mCursorSceneX >= mTilesetProject->widthInPixels()
        || mCursorSceneY < 0 || mCursorSceneY >= mTilesetProject->heightInPixels()) {
        setCursorTilePixelX(-1);
        setCursorTilePixelY(-1);

        setTilePenPreview(false);

        setCursorPixelColour(QColor(Qt::black));
    } else {
        const QPoint cursorPixelPos = scenePosToTilePixelPos(QPoint(mCursorSceneX, mCursorSceneY));
        setCursorTilePixelX(cursorPixelPos.x());
        setCursorTilePixelY(cursorPixelPos.y());

        updateTilePenPreview();

        const QPoint cursorScenePos = QPoint(mCursorSceneX, mCursorSceneY);
        const Tile *tile = mTilesetProject->tileAt(cursorScenePos);
        if (!tile) {
            setCursorPixelColour(QColor(Qt::black));
        } else {
            const QPoint tilePixelPos = scenePosToTilePixelPos(cursorScenePos);
            setCursorPixelColour(tile->pixelColor(tilePixelPos));
        }

        if (mTilePenPreview) {
            requestContentPaint();
        }
    }
}

void TileCanvas::onLoadedChanged()
{
    if (mTilesetProject->hasLoaded()) {
        centrePanes();

        setPenTile(mTilesetProject->tilesetTileAt(0, 0));
    } else {
        setPenTile(nullptr);
    }

    updateWindowCursorShape();
}

QColor TileCanvas::penColour() const
{
    return mMouseButtonPressed == Qt::LeftButton ? mPenForegroundColour : mPenBackgroundColour;
}

void TileCanvas::setHasBlankCursor(bool hasCustomCursor)
{
    if (hasCustomCursor == mHasBlankCursor)
        return;

    mHasBlankCursor = hasCustomCursor;
    emit hasBlankCursorChanged();
}

void TileCanvas::updateTilePenPreview()
{
    setTilePenPreview(mMode == TileMode && mTool == PenTool);
}

void TileCanvas::setTilePenPreview(bool tilePenPreview)
{
    if (tilePenPreview == mTilePenPreview)
        return;

    mTilePenPreview = tilePenPreview;
    requestContentPaint();
}

void TileCanvas::hoverLeaveEvent(QHoverEvent *event)
{
    ImageCanvas::hoverLeaveEvent(event);

    if (!mTilesetProject->hasLoaded())
        return;

    // Don't reset the cursor position here, because it looks jarring.
    setCursorTilePixelX(-1);
    setCursorTilePixelY(-1);
}

void TileCanvas::focusInEvent(QFocusEvent *event)
{
    ImageCanvas::focusInEvent(event);
    updateTilePenPreview();
}

void TileCanvas::focusOutEvent(QFocusEvent *event)
{
    ImageCanvas::focusOutEvent(event);
    setTilePenPreview(false);
}
