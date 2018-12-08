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

#include "commands.h"

Q_LOGGING_CATEGORY(lcApplyPixelLineCommand, "app.undo.applyPixelLineCommand")

ApplyPixelLineCommand::ApplyPixelLineCommand(ImageCanvas *canvas, const QPointF point1, const QPointF point2,
        const QPointF &newLastPixelPenReleaseScenePos, const QPointF &oldLastPixelPenReleaseScenePos,
        const QPainter::CompositionMode mode, QUndoCommand *parent) :
    QUndoCommand(parent),
    mCanvas(canvas),
    mNewLastPixelPenReleaseScenePos(newLastPixelPenReleaseScenePos),
    mOldLastPixelPenReleaseScenePos(oldLastPixelPenReleaseScenePos),
    mMode(mode),
    mStroke({point1, point2}), mOldStroke(),
    mDrawBounds(ImageCanvas::strokeBounds(mStroke, mCanvas->toolSize())),
    mImageRegion(), mUndoBuffer(), mRedoBuffer(), mImageBounds(),
    needDraw(true)
{
    qCDebug(lcApplyPixelLineCommand) << "constructed" << this;
}

ApplyPixelLineCommand::~ApplyPixelLineCommand()
{
    qCDebug(lcApplyPixelLineCommand) << "destructed" << this;
}

void ApplyPixelLineCommand::undo()
{
    qCDebug(lcApplyPixelLineCommand) << "undoing" << this;

    // Copy image from undo buffer
    QPainter painter(mCanvas->currentProjectImage());
    for (auto rect : mImageRegion.rects()) {
        undoRect(painter, rect);
    }
    mCanvas->requestContentPaint();

    mCanvas->mLastPixelPenPressScenePositionF = mOldLastPixelPenReleaseScenePos;
}

void ApplyPixelLineCommand::redo()
{
    qCDebug(lcApplyPixelLineCommand) << "redoing" << this;

    qDebug() << "DRAW" << mDrawBounds;
    qDebug() << mCanvas->subImageInstancesInBounds(mDrawBounds);

    mDrawBounds = ImageCanvas::strokeBounds(mStroke, mCanvas->toolSize());////////////////////////////

    // First "redo" so draw and store undo/redo buffers
    if (needDraw) {
        // Find intersections of stroke and subimages
        struct Intersection {
            QRect bounds;
            QPoint drawOffset;
        };
        QList<Intersection> subImageIntersections;
        for (auto const &subImage : mCanvas->subImageInstancesInBounds(mDrawBounds)) {
            const QPoint subImageOffset = subImage.bounds.topLeft() - subImage.position;
            const QRect subImageIntersection = mDrawBounds.translated(subImageOffset).intersected(subImage.bounds);
            if (!subImageIntersection.isEmpty()) {
                subImageIntersections.append({subImageIntersection, subImageOffset});
            }
        }

        if (!subImageIntersections.isEmpty()) {
            QPainter painter;

            // Restore area to be drawn from undo buffer
            painter.begin(mCanvas->currentProjectImage());
            for (auto const &intersection : subImageIntersections) {
                for (auto rect : mImageRegion.intersected(intersection.bounds).rects()) {
                    undoRect(painter, rect);
                }
                mImageRegion = mImageRegion.subtracted(intersection.bounds);
            }
            painter.end();

            // Resize buffers if bounds changed
            const QRect oldImageBounds = mImageBounds;
            // Add rects to region
            for (auto const &intersection : subImageIntersections) {
                mImageRegion = mImageRegion.united(intersection.bounds);
            }
            mImageBounds = mImageRegion.boundingRect();
            if (mImageBounds != oldImageBounds) {
                const QPoint bufferOffset = mImageBounds.topLeft() - oldImageBounds.topLeft();

                if (mUndoBuffer.isNull()) mUndoBuffer = QImage(mImageBounds.size(), mCanvas->currentProjectImage()->format());
                else mUndoBuffer = mUndoBuffer.copy(mImageBounds.translated(bufferOffset));

                if (mRedoBuffer.isNull()) mRedoBuffer = QImage(mImageBounds.size(), mCanvas->currentProjectImage()->format());
                else mRedoBuffer = mRedoBuffer.copy(mImageBounds.translated(bufferOffset));
            }

            // Store original image in undo buffer
            painter.begin(&mUndoBuffer);
            for (auto const &intersection : subImageIntersections) {
                painter.setCompositionMode(QPainter::CompositionMode_Source);
                painter.drawImage(intersection.bounds.topLeft() - mImageBounds.topLeft(), *mCanvas->currentProjectImage(), intersection.bounds);
            }
            painter.end();

            // Draw stroke
            painter.begin(mCanvas->currentProjectImage());
            for (auto const &intersection : subImageIntersections) {
                painter.save();
                painter.setClipRect(intersection.bounds);
                painter.translate(intersection.drawOffset);
                mCanvas->drawStroke(&painter, mStroke, mMode);
                painter.restore();
            }
            painter.end();

            // Store changed image in redo buffer
            painter.begin(&mRedoBuffer);
            for (auto const &intersection : subImageIntersections) {
                painter.setCompositionMode(QPainter::CompositionMode_Source);
                painter.drawImage(intersection.bounds.topLeft() - mImageBounds.topLeft(), *mCanvas->currentProjectImage(), intersection.bounds);
            }
            painter.end();
        }

        needDraw = false;
    }
    // Otherwise copy image from redo buffer
    else {
        QPainter painter(mCanvas->currentProjectImage());
        for (auto rect : mImageRegion.rects()) {
            redoRect(painter, rect);
        }
    }
    mCanvas->requestContentPaint();

    mCanvas->mLastPixelPenPressScenePositionF = mNewLastPixelPenReleaseScenePos;
}

int ApplyPixelLineCommand::id() const
{
    return ApplyPixelLineCommandId;
}

bool ApplyPixelLineCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id()) return false;
    const ApplyPixelLineCommand *newCommand = static_cast<const ApplyPixelLineCommand*>(other);
    if (newCommand->mStroke.first() != mStroke.last() || newCommand->mCanvas != mCanvas) return false;

    // Undo new stroke
    QPainter painter;
    painter.begin(mCanvas->currentProjectImage());
    for (auto rect : newCommand->mImageRegion.rects()) {
        newCommand->undoRect(painter, rect);
    }
    painter.end();

    // Merge new stroke with old stroke
    mDrawBounds = newCommand->mDrawBounds;
    mStroke.append(newCommand->mStroke.mid(1));
    qDebug() << "mDrawBounds" << mDrawBounds;

    // Redraw merged stroke
    needDraw = true;
    redo();

    return true;
}

void ApplyPixelLineCommand::undoRect(QPainter &painter, const QRect &rect) const
{
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(rect, mUndoBuffer, rect.translated(-mImageBounds.topLeft()));
}

void ApplyPixelLineCommand::redoRect(QPainter &painter, const QRect &rect) const
{
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.drawImage(rect, mRedoBuffer, rect.translated(-mImageBounds.topLeft()));
}

QDebug operator<<(QDebug debug, const ApplyPixelLineCommand *command)
{
    debug.nospace() << "(ApplyPixelLineCommand"
//        << " layerIndex=" << command->mLayerIndex
//        << ", lineRect" << command->mLineRect
        << ", newLastPixelPenReleaseScenePos=" << command->mNewLastPixelPenReleaseScenePos
        << ", oldLastPixelPenReleaseScenePos=" << command->mOldLastPixelPenReleaseScenePos
        << ")";
    return debug.space();
}
