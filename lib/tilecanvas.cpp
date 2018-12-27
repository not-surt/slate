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
    mTileIndicesVisible(false)
{
    qCDebug(lcImageCanvasLifecycle) << "constructing TileCanvas" << this;
}

TileCanvas::~TileCanvas()
{
    qCDebug(lcImageCanvasLifecycle) << "destructing TileCanvas" << this;
}

const QImage *TileCanvas::currentProjectImage() const
{
    return mTilesetProject->tileset()->image();
}

QImage *TileCanvas::currentProjectImage()
{
    return const_cast<QImage *>(const_cast<const TileCanvas *>(this)->currentProjectImage());
}

const QImage *TileCanvas::imageForLayerAt(int layerIndex) const
{
    Q_ASSERT(layerIndex == -1);
    return mTilesetProject->tileset()->image();
}

QImage *TileCanvas::imageForLayerAt(int layerIndex)
{
    return const_cast<QImage *>(const_cast<const TileCanvas *>(this)->imageForLayerAt(layerIndex));
}

void TileCanvas::paintContent(QPainter *const painter, const QRect rect)
{
    const QSize &tileSize = mTilesetProject->tileset()->tileSize();
    const QRect tilesRect = sceneRectToTileRect(rect).intersected(mTilesetProject->tileBounds());

    for (int y = tilesRect.top(); y <= tilesRect.bottom(); ++y) {
        for (int x = tilesRect.left(); x <= tilesRect.right(); ++x) {
            const QPoint pos{x * tileSize.width(), y * tileSize.height()};
            const TileObject *tile = mTilesetProject->tileObjectAt(QPoint(x, y));
            if (tile) {
                painter->drawImage(pos, *tile->tileset()->image(), tile->sourceRect());
            }
        }
    }
}

void TileCanvas::paintOverlay(QPainter *const painter, const QRect rect) const
{
    Q_ASSERT(mTilesetProject);

    const QSize &tileSize = mTilesetProject->tileset()->tileSize();
    const QRect tilesRect = sceneRectToTileRect(rect).intersected(mTilesetProject->tileBounds());

    if (mGridVisible) {
        QPen pen;
        pen.setWidth(0);
        pen.setColor(mGridColour);
        painter->setPen(pen);

        for (int y = tilesRect.top(); y <= tilesRect.bottom(); ++y) {
            for (int x = tilesRect.left(); x <= tilesRect.right(); ++x) {
                // Draw top edge for tile
                painter->drawLine(x * tileSize.width(), y * tileSize.height(), (x + 1) * tileSize.width(), y * tileSize.height());
                // Draw left edge for tile
                painter->drawLine(x * tileSize.width(), y * tileSize.height(), x * tileSize.width(), (y + 1) * tileSize.height());
            }
        }
        // Draw bottom-most edges
        for (int x = tilesRect.left(), y = tilesRect.bottom() + 1; x <= tilesRect.right(); ++x) {
            painter->drawLine(x * tileSize.width(), y * tileSize.height(), (x + 1) * tileSize.width(), y * tileSize.height());
        }
        // Draw right-most edges
        for (int y = tilesRect.top(), x = tilesRect.right() + 1; y <= tilesRect.bottom(); ++y) {
            painter->drawLine(x * tileSize.width(), y * tileSize.height(), x * tileSize.width(), (y + 1) * tileSize.height());
        }
    }

    // Draw tile indices
    if (mGridVisible/*mTileIndicesVisible*/) {
        const int flags = Qt::AlignRight | Qt::AlignTop;
        QFont font("Courier");
        painter->setFont(font);
        const qreal scale = 16.0 / qreal(painter->fontMetrics().height());
        font.setPointSizeF(font.pointSizeF() * scale);
        painter->setFont(font);
        for (int y = tilesRect.top(); y <= tilesRect.bottom(); ++y) {
            for (int x = tilesRect.left(); x <= tilesRect.right(); ++x) {
                const TileObject *tile = mTilesetProject->tileObjectAt(QPoint(x, y));
                if (tile) {
                    const QString string = QString::number(tile->id())/*.rightJustified(3, '0')*/;
                    const QRect rect = painter->transform().mapRect(QRect(QPoint(x * tileSize.width(), y * tileSize.height()), tileSize));
//                    painter->save();
//                    painter->resetTransform();
//                    painter->setRenderHints(QPainter::HighQualityAntialiasing);
//                    QPainterPath path;
//                    path.addText(rect.topLeft(), painter->font(), string);////
//                    painter->setPen(Qt::black);
//                    painter->setBrush(Qt::white);
//                    painter->drawPath(path);
//                    painter->restore();
                    painter->save();
                    painter->resetTransform();
                    painter->setPen(Qt::black);
                    painter->drawText(rect.translated(1, 1), flags, string);
                    painter->setPen(Qt::lightGray);
                    painter->drawText(rect, flags, string);
                    painter->restore();
                }
            }
        }
    }

    // Draw tile cursor
    if (mMode == TileMode && tilesRect.contains(mCursorSceneTile)) {
        const QRect rect = painter->transform().mapRect(QRect(QPoint(mCursorSceneTile.x() * tileSize.width(), mCursorSceneTile.y() * tileSize.height()), tileSize));
        painter->save();
        painter->resetTransform();
        Utils::strokeRectWithDashes(painter, rect);
        painter->restore();
    }
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
    emit modeChanged();
}

bool TileCanvas::tileIndicesVisible() const
{
    return mTileIndicesVisible;
}

void TileCanvas::setTileIndicesVisible(bool tileIndicesVisible)
{
    if (tileIndicesVisible == mTileIndicesVisible)
        return;

    mTileIndicesVisible = tileIndicesVisible;
    emit modeChanged();
}

TileObject *TileCanvas::penTile() const
{
    return mPenTile;
}

void TileCanvas::setPenTile(TileObject *penTile)
{
    if (penTile == mPenTile)
        return;

    mPenTile = penTile;
    emit penTileChanged();
}

QPointF TileCanvas::pressSceneTilePos() const
{
    return QPointF(mPressScenePos.x() / qreal(mTilesetProject->tileWidth()),
                   mPressScenePos.y() / qreal(mTilesetProject->tileHeight()));
}

QPoint TileCanvas::pressSceneTileCoord() const
{
    return QPoint(qFloor(pressSceneTilePos().x()), qFloor(pressSceneTilePos().y()));
}

QPoint TileCanvas::pressSceneTileCorner() const
{
    return QPoint(qRound(pressSceneTilePos().x()), qRound(pressSceneTilePos().y()));
}

QPointF TileCanvas::cursorSceneTilePos() const
{
    return QPointF(mCursorScenePos.x() / qreal(mTilesetProject->tileWidth()),
                   mCursorScenePos.y() / qreal(mTilesetProject->tileHeight()));
}

QPoint TileCanvas::cursorSceneTileCoord() const
{
    return QPoint(qFloor(cursorSceneTilePos().x()), qFloor(cursorSceneTilePos().y()));
}

QPoint TileCanvas::cursorSceneTileCorner() const
{
    return QPoint(qRound(cursorSceneTilePos().x()), qRound(cursorSceneTilePos().y()));
}

void TileCanvas::swatchLeft()
{
    QPoint tilePos = mTilesetProject->tileIdToTilePos(mPenTile->id());
    tilePos.setX(qMax(0, tilePos.x() - 1));
    TileObject *newTile = mTilesetProject->tilesetTileAtTilePos(tilePos);
    setPenTile(newTile);
}

void TileCanvas::swatchRight()
{
    QPoint tilePos = mTilesetProject->tileIdToTilePos(mPenTile->id());
    tilePos.setX(qMin(tilePos.x() + 1, mTilesetProject->tileset()->size().width() - 1));
    TileObject *newTile = mTilesetProject->tilesetTileAtTilePos(tilePos);
    setPenTile(newTile);
}

void TileCanvas::swatchUp()
{
    QPoint tilePos = mTilesetProject->tileIdToTilePos(mPenTile->id());
    tilePos.setY(qMax(0, tilePos.y() - 1));
    TileObject *newTile = mTilesetProject->tilesetTileAtTilePos(tilePos);
    setPenTile(newTile);
}

void TileCanvas::swatchDown()
{
    QPoint tilePos = mTilesetProject->tileIdToTilePos(mPenTile->id());
    tilePos.setY(qMin(tilePos.y() + 1, mTilesetProject->tileset()->size().height() - 1));
    TileObject *newTile = mTilesetProject->tilesetTileAtTilePos(tilePos);
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

bool TileCanvas::supportsSelectionTool() const
{
    return false;
}

QImage TileCanvas::getComposedImage()
{
    QImage image(mTilesetProject->pixelSize(),  QImage::Format_ARGB32);
    image.fill(qRgba(0, 0, 0, 0));
    QPainter painter(&image);

    paintContent(&painter, QRect(QPoint(0, 0), mTilesetProject->pixelSize()));

    return image;
}

TileCanvas::PixelCandidateData TileCanvas::fillPixelCandidates() const
{
    PixelCandidateData candidateData;

    const QPoint tilePos = QPoint(cursorScenePixelCoord().x(), cursorScenePixelCoord().y());
    TileObject *tile = mTilesetProject->tileObjectAtPixel(tilePos);
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

    const QPoint scenePos = cursorScenePixelCoord();
    const TileObject *tile = mTilesetProject->tileObjectAtPixel(scenePos);
    const int previousTileId = tile ? tile->id() : TileObject::invalidId();
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
            const QPoint scenePos = cursorScenePixelCoord();
            const TileObject *tile = mTilesetProject->tileObjectAtPixel(scenePos);
            const int previousTileId = tile ? tile->id() : TileObject::invalidId();
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
            const QPoint tilePos = cursorScenePixelCoord();
            TileObject *tile = mTilesetProject->tileObjectAtPixel(tilePos);
            if (tile) {
                setPenTile(tile);
            }
        }
        break;
    }
    case EraserTool: {
        if (mMode == TileMode) {
            const QPoint scenePos = cursorScenePixelCoord();
            const TileObject *tile = mTilesetProject->tileObjectAtPixel(scenePos);
            const int previousTileId = tile ? tile->id() : TileObject::invalidId();
            if (previousTileId == TileObject::invalidId()) {
                return;
            }

            const int xTile = Utils::divFloor(scenePos.x(), mTilesetProject->tileWidth());
            const int yTile = Utils::divFloor(scenePos.y(), mTilesetProject->tileHeight());

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

    if (alternateStack)
        alternateStack->push(command);
    else
        mProject->addChange(command);
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
    const TileObject *const tile = mTilesetProject->tilesetTileAtId(index);
    Q_ASSERT(tile);

    return {0, tile->sourceRect(), {0, 0}};
}

QList<ImageCanvas::SubImageInstance> TileCanvas::subImageInstancesInBounds(const QRect &bounds) const
{
    const QRect tileRect = sceneRectToTileRect(bounds);
    QList<ImageCanvas::SubImageInstance> instances;
    for (int y = tileRect.top(); y <= tileRect.bottom(); ++y) {
        for (int x = tileRect.left(); x <= tileRect.right(); ++x) {
            const TileObject *const tile = mTilesetProject->tileObjectAt({x, y});
            if (tile) {
                instances.append({tile->id(), {x * mTilesetProject->tileWidth(), y * mTilesetProject->tileHeight()}});
            }
        }
    }
    return instances;
}

// This function actually operates on the image.
void TileCanvas::applyPixelPenTool(int layerIndex, const QPoint &scenePos, const QColor &colour)
{
    Q_ASSERT(layerIndex == -1);

    TileObject *tile = mTilesetProject->tileObjectAtPixel(scenePos);
    Q_ASSERT_X(tile, Q_FUNC_INFO, qPrintable(QString::fromLatin1(
        "No tile at scene pos {%1, %2}").arg(scenePos.x()).arg(scenePos.y())));
    const QPoint pixelPos = scenePosToTilePixelPos(scenePos);
    const QPoint tilsetPixelPos = tile->sourceRect().topLeft() + pixelPos;
    mTilesetProject->tileset()->setPixelColor(tilsetPixelPos.x(), tilsetPixelPos.y(), colour);
    requestContentPaint();
}

void TileCanvas::applyTilePenTool(const QPoint &tilePos, int id)
{
    TileObject *const tile = mTilesetProject->tileObjectAtPixel(tilePos);
    mTilesetProject->setTileIndexAt(tilePos, id);
    requestContentPaint();
}

void TileCanvas::onLoadedChanged()
{
    if (mTilesetProject->hasLoaded()) {
        setPenTile(mTilesetProject->tilesetTileAt(0, 0));
    } else {
        setPenTile(nullptr);
    }

    updateWindowCursorShape();
}
