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
    return &mTilesetProject->tileset()->image();
}

QImage *TileCanvas::currentProjectImage()
{
    return const_cast<QImage *>(const_cast<const TileCanvas *>(this)->currentProjectImage());
}

const QImage *TileCanvas::imageForLayerAt(int layerIndex) const
{
    Q_ASSERT(layerIndex == -1);
    return &mTilesetProject->tileset()->image();
}

QImage *TileCanvas::imageForLayerAt(int layerIndex)
{
    return const_cast<QImage *>(const_cast<const TileCanvas *>(this)->imageForLayerAt(layerIndex));
}

void TileCanvas::paintContent(QPainter *const painter, const QRect &rect)
{
    const QSize &tileSize = mTilesetProject->tileset()->tileSize();
    const QRect tilesRect = sceneRectToTileRect(rect).intersected(mTilesetProject->tileBounds());

    for (int y = tilesRect.top(); y <= tilesRect.bottom(); ++y) {
        for (int x = tilesRect.left(); x <= tilesRect.right(); ++x) {
            const QPoint pos{x * tileSize.width(), y * tileSize.height()};
            const TileObject *tile = mTilesetProject->tileObjectAt(QPoint(x, y));
            if (tile) {
                painter->drawImage(pos, tile->tileset()->image(), tile->sourceRect());
            }
        }
    }
}

void TileCanvas::paintOverlay(QPainter *const painter, const QRect &rect) const
{
    Q_ASSERT(mTilesetProject);

    const QSize &tileSize = mTilesetProject->tileset()->tileSize();
    const QRect tilesRect = sceneRectToTileRect(rect).intersected(mTilesetProject->tileBounds());

    if (mGridVisible) {
        QPen pen;
        pen.setWidth(0);
        pen.setColor(mGridColour);
        painter->setPen(pen);

        // Draw horizontal edges
        for (int y = tilesRect.top(); y <= tilesRect.bottom() + 1; ++y) {
            painter->drawLine(tilesRect.left() * tileSize.width(), y * tileSize.height(), (tilesRect.right() + 1) * tileSize.width(), y * tileSize.height());
        }
        // Draw vertical edges
        for (int x = tilesRect.left(); x <= tilesRect.right() + 1; ++x) {
            painter->drawLine(x * tileSize.width(), tilesRect.top() * tileSize.height(), x * tileSize.width(), (tilesRect.bottom() + 1) * tileSize.height());
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

//    // Draw tile cursor
//    if (mMode == TileMode && tilesRect.contains(mCursorSceneTile)) {
//        const QRect rect = painter->transform().mapRect(QRect(QPoint(mCursorSceneTile.x() * tileSize.width(), mCursorSceneTile.y() * tileSize.height()), tileSize));
//        painter->save();
//        painter->resetTransform();
//        Utils::strokeRectWithDashes(painter, rect);
//        painter->restore();
//    }
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

QPointF TileCanvas::pressSceneTilePos() const
{
    return QPointF(mPressScenePos.x() / qreal(mTilesetProject->tileSize().width()),
                   mPressScenePos.y() / qreal(mTilesetProject->tileSize().height()));
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
    return QPointF(mCursorScenePos.x() / qreal(mTilesetProject->tileSize().width()),
                   mCursorScenePos.y() / qreal(mTilesetProject->tileSize().height()));
}

QPoint TileCanvas::cursorSceneTileCoord() const
{
    return QPoint(qFloor(cursorSceneTilePos().x()), qFloor(cursorSceneTilePos().y()));
}

QPoint TileCanvas::cursorSceneTileCorner() const
{
    return QPoint(qRound(cursorSceneTilePos().x()), qRound(cursorSceneTilePos().y()));
}

void TileCanvas::connectSignals()
{
    ImageCanvas::connectSignals();

    mTilesetProject = qobject_cast<TilesetProject*>(mProject);
    Q_ASSERT_X(mTilesetProject, Q_FUNC_INFO, "Non-tileset project set on TileCanvas");

    connect(mTilesetProject, &TilesetProject::tilesCleared, this, &TileCanvas::requestContentPaint);
}

void TileCanvas::disconnectSignals()
{
    ImageCanvas::disconnectSignals();

    disconnect(mTilesetProject, &TilesetProject::tilesCleared, this, &TileCanvas::requestContentPaint);

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
    const int newTileId = mCurrentTileIndex;
    // Don't do anything if the tiles are the same.
    if (newTileId == previousTileId) {
        return candidateData;
    }

    const int xTile = scenePos.x() / mTilesetProject->tileSize().width();
    const int yTile = scenePos.y() / mTilesetProject->tileSize().height();
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

    QUndoCommand *command = nullptr;

    const QPoint pixelCoord = cursorScenePixelCoord();
    const QPoint tileCoord = cursorSceneTileCoord();
    TileObject *tile = mTilesetProject->tileObjectAtPixel(pixelCoord);
    const int previousTileId = tile ? tile->id() : TileObject::invalidId();

    switch (mTool) {
    case PenTool: {
        if (mMode == TileMode) {
            const int newTileId = mCurrentTileIndex;
            command = new ApplyTilePenCommand(this, tileCoord, previousTileId, newTileId);
            command->setText(QLatin1String("TilePenTool"));
        }
        break;
    }
    case EyeDropperTool: {
        if (mMode == TileMode) {
            mCurrentTileIndex = mTilesetProject->tile(tileCoord);
        }
        break;
    }
    case EraserTool: {
        if (mMode == TileMode) {
            command = new ApplyTileEraserCommand(this, tileCoord, previousTileId);
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
    return QPoint(scenePos.x() % mTilesetProject->tileSize().width(),
                  scenePos.y() % mTilesetProject->tileSize().height());
}

QRect TileCanvas::sceneRectToTileRect(const QRect &sceneRect) const
{
    return QRect(QPoint(Utils::divFloor(sceneRect.left(), mTilesetProject->tileSize().width()),
                        Utils::divFloor(sceneRect.top(), mTilesetProject->tileSize().height())),
                 QPoint(Utils::divFloor(sceneRect.right(), mTilesetProject->tileSize().width()),
                        Utils::divFloor(sceneRect.bottom(), mTilesetProject->tileSize().height())));
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
                instances.append({tile->id(), {x * mTilesetProject->tileSize().width(), y * mTilesetProject->tileSize().height()}});
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
    mTilesetProject->tileset()->image().setPixelColor(tilsetPixelPos.x(), tilsetPixelPos.y(), colour);
    requestContentPaint();
}

void TileCanvas::applyTilePenTool(const QPoint &tilePos, int id)
{
    mTilesetProject->setTile(tilePos, id);
    requestContentPaint();
}

void TileCanvas::onLoadedChanged()
{
    updateWindowCursorShape();
}
