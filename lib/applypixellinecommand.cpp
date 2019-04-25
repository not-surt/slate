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

#include "applypixellinecommand.h"

#include <QLoggingCategory>
#include <QPainter>
#include <QImage>
#include <QtMath>

#include "commands.h"
#include "utils.h"

Q_LOGGING_CATEGORY(lcApplyPixelLineCommand, "app.undo.applyPixelLineCommand")

ApplyPixelLineCommand::ApplyPixelLineCommand(ImageCanvas *const canvas, const int layerIndex, const Stroke &stroke,
        const QPainter::CompositionMode compositionMode, const QUndoCommand *const previousCommand, QUndoCommand *const parent) :
    QUndoCommand(parent),
    mCanvas(canvas), mLayerIndex(layerIndex),
    mOldStroke(mCanvas->mOldStroke),
    mCompositionMode(compositionMode),
    mEditingContext(nullptr),
    mStroke(stroke), mStrokeUpdateStartIndex(0),
    mBufferRegion(), mUndoBuffer(), mRedoBuffer(), mStrokeBuffer(), mBufferBounds(),
    mPreviousCommand(previousCommand), mNeedDraw(true)
{
    qCDebug(lcApplyPixelLineCommand) << "constructed" << this;
}

ApplyPixelLineCommand::~ApplyPixelLineCommand()
{
    delete mEditingContext;
    qCDebug(lcApplyPixelLineCommand) << "destructed" << this;
}

void ApplyPixelLineCommand::undo()
{
    qCDebug(lcApplyPixelLineCommand) << "undoing" << this;

    // Copy image from undo buffer
    QPainter painter(mCanvas->imageForLayerAt(mLayerIndex));
    for (auto const &rect : mBufferRegion) {
        undoRect(painter, rect);
    }

    mCanvas->requestContentPaint();
    mCanvas->mOldStroke = mOldStroke;
}

void ApplyPixelLineCommand::redo()
{
    qCDebug(lcApplyPixelLineCommand) << "redoing" << this;

    // Merging so initially skip drawing then draw after merge
    if (canMerge(mPreviousCommand)) {
        // Do nothing
        return;
    }

    // First "redo" so draw and store undo/redo buffers
    if (mNeedDraw) {
        // Copy canvas's editing context
        mEditingContext = new EditingContextManager(*mCanvas->editingContext());

        // Find intersections of subimages with draw area
        QMap<int, QRegion> subImageRegions;
        QRect mDrawBounds = Stroke(mStroke.mid(mStrokeUpdateStartIndex)).bounds(*mCanvas->mEditingContext->brush(), mCanvas->mEditingContext->brushScalingMin(), mCanvas->mEditingContext->brushScalingMax());
        for (auto const &instance : mCanvas->subImageInstancesInBounds(mDrawBounds)) {
            const ImageCanvas::SubImage subImage = mCanvas->getSubImage(instance.index);
            const QPoint instanceDrawOffset = subImage.bounds.topLeft() - instance.position;
            subImageRegions[instance.index] += mDrawBounds.translated(instanceDrawOffset).intersected(subImage.bounds);
        }

        // Add subimage regions to draw region
        QRegion bufferDrawRegion;
        for (auto const &region : subImageRegions) {
            bufferDrawRegion += region;
        }

        // Find offsets of subimage instances within stroke area
        QMap<int, QVector<QPoint>> instanceOffsets;
        for (auto const &instance : mCanvas->subImageInstancesInBounds(mStroke.bounds(*mCanvas->mEditingContext->brush(), mCanvas->mEditingContext->brushScalingMin(), mCanvas->mEditingContext->brushScalingMax()))) {
            const ImageCanvas::SubImage subImage = mCanvas->getSubImage(instance.index);
            const QPoint instanceDrawOffset = subImage.bounds.topLeft() - instance.position;
            if (subImageRegions.contains(instance.index)) instanceOffsets[instance.index] += instanceDrawOffset;
        }

        if (!subImageRegions.isEmpty()) {
            QPainter painter;

            const QRegion bufferRegionIntersection = bufferDrawRegion.intersected(mBufferRegion);

            // Restore area to be drawn from undo buffer
            painter.begin(mCanvas->imageForLayerAt(mLayerIndex));
            for (auto const &rect : bufferRegionIntersection) {
                undoRect(painter, rect);
            }
            painter.end();
            mBufferRegion = mBufferRegion.subtracted(bufferRegionIntersection);

            // Add draw region to buffer region
            const QRect oldBufferBounds = mBufferBounds;
            mBufferRegion = mBufferRegion.united(bufferDrawRegion);
            mBufferBounds = mBufferRegion.boundingRect();
            // Resize undo/redo buffers if bounds changed
            if (mBufferBounds != oldBufferBounds) {
                if (mUndoBuffer.isNull()) {
                    mUndoBuffer = QImage(mBufferBounds.size(), mCanvas->imageForLayerAt(mLayerIndex)->format());
                    mRedoBuffer = QImage(mBufferBounds.size(), mCanvas->imageForLayerAt(mLayerIndex)->format());
                    mStrokeBuffer = QImage(mBufferBounds.size(), mCanvas->imageForLayerAt(mLayerIndex)->format());
                }
                else {
                    const QRect copyRect = QRect(mBufferBounds.topLeft() - oldBufferBounds.topLeft(), mBufferBounds.size());
                    mUndoBuffer = mUndoBuffer.copy(copyRect);
                    mRedoBuffer = mRedoBuffer.copy(copyRect);
                    mStrokeBuffer = mRedoBuffer.copy(copyRect);
                }
            }

            // Store original image in undo buffer
            painter.begin(&mUndoBuffer);
            for (auto const &rect : bufferDrawRegion) {
                painter.setCompositionMode(QPainter::CompositionMode_Source);
                painter.drawImage(rect.topLeft() - mBufferBounds.topLeft(), *mCanvas->imageForLayerAt(mLayerIndex), rect);
            }
            painter.end();

            // Draw stroke to image
            {
                Utils::ContextGrabber grabber(&mCanvas->mStrokeRenderer.surface(), &mCanvas->mStrokeRenderer.context());
                mCanvas->mStrokeRenderer.setStroke(mStroke);
                mCanvas->mStrokeRenderer.setEditingContext(*mCanvas->mEditingContext);
                mCanvas->mStrokeRenderer.setImage(mCanvas->imageForLayerAt(mLayerIndex));
                mCanvas->mStrokeRenderer.upload(bufferDrawRegion.boundingRect());
                for (auto const &key : subImageRegions.keys()) {
                    for (auto const &rect : subImageRegions[key].rects()) {
//                        mCanvas->mStrokeRenderer.upload(rect);
                        for (auto const &offset : instanceOffsets[key]) {
                            mCanvas->mStrokeRenderer.render(QTransform().translate(offset.x(), offset.y()), rect);
                        }
//                        mCanvas->mStrokeRenderer.download(rect);
                    }
                }
                mCanvas->mStrokeRenderer.download(bufferDrawRegion.boundingRect());
            }

            // Store changed image in redo buffer
            painter.begin(&mRedoBuffer);
            for (auto const &rect : bufferDrawRegion) {
                painter.setCompositionMode(QPainter::CompositionMode_Source);
                painter.drawImage(rect.topLeft() - mBufferBounds.topLeft(), *mCanvas->imageForLayerAt(mLayerIndex), rect);
            }
            painter.end();
        }

        mNeedDraw = false;
    }

    // Otherwise copy image from redo buffer
    else {
        QPainter painter(mCanvas->imageForLayerAt(mLayerIndex));
        for (auto const &rect : mBufferRegion) {
            redoRect(painter, rect);
        }
    }

    mCanvas->requestContentPaint();
    mCanvas->mOldStroke = mStroke;
}

int ApplyPixelLineCommand::id() const
{
    return ApplyPixelLineCommandId;
}

bool ApplyPixelLineCommand::mergeWith(const QUndoCommand *const command)
{
    const ApplyPixelLineCommand *const newCommand = dynamic_cast<const ApplyPixelLineCommand *const>(command);
    if (!newCommand || !newCommand->canMerge(this))
        return false;

    // Merge new stroke with old stroke
    mStrokeUpdateStartIndex = mStroke.length() - 1;
    mStroke.append(newCommand->mStroke.mid(1));

    // Draw merged stroke
    mPreviousCommand = nullptr;
    mNeedDraw = true;
    redo();

    return true;
}

bool ApplyPixelLineCommand::canMerge(const QUndoCommand *const command) const
{
    if (!mPreviousCommand || !command || command->id() != id())
        return false;
    const ApplyPixelLineCommand *mergeCommand = static_cast<const ApplyPixelLineCommand*>(command);
    if (mCanvas != mergeCommand->mCanvas || mLayerIndex != mergeCommand->mLayerIndex ||
        *mCanvas->editingContext() != *mergeCommand->mEditingContext ||
        mStroke.first() != mergeCommand->mStroke.last())
        return false;

    return true;
}

void ApplyPixelLineCommand::undoRect(QPainter &painter, const QRect &rect) const
{
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(rect, mUndoBuffer, rect.translated(-mBufferBounds.topLeft()));
}

void ApplyPixelLineCommand::redoRect(QPainter &painter, const QRect &rect) const
{
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(rect, mRedoBuffer, rect.translated(-mBufferBounds.topLeft()));
}

QDebug operator<<(QDebug debug, const ApplyPixelLineCommand *command)
{
    debug.nospace() << "ApplyPixelLineCommand("
        << "layerIndex=" << command->mLayerIndex
        << ", stroke=" << command->mStroke
        << ", oldStroke=" << command->mOldStroke
        << ")";
    return debug.space();
}
