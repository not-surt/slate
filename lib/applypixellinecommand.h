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

#ifndef APPLYPIXELLINECOMMAND_H
#define APPLYPIXELLINECOMMAND_H

#include <QDebug>
#include <QPointF>
#include <QUndoCommand>

#include "imagecanvas.h"
#include "slate-global.h"

class SLATE_EXPORT ApplyPixelLineCommand : public QUndoCommand
{
public:
    ApplyPixelLineCommand(ImageCanvas *const canvas, const int layerIndex, const QPointF point1, const QPointF point2,
        const QPointF &newLastPixelPenReleaseScenePos, const QPointF &oldLastPixelPenReleaseScenePos,
        const QPainter::CompositionMode compositionMode, const bool allowMerge, const QUndoCommand *const previousCommand = nullptr, QUndoCommand *const parent = nullptr);
    virtual ~ApplyPixelLineCommand() override;

    void undo() override;
    void redo() override;

    int id() const override;
    bool mergeWith(const QUndoCommand *const command) override;

private:
    friend QDebug operator<<(QDebug debug, const ApplyPixelLineCommand *command);

    bool canMerge(const QUndoCommand *const command) const;

    void undoRect(QPainter &painter, const QRect &rect) const;
    void redoRect(QPainter &painter, const QRect &rect) const;
    void resizeUndoRedoBuffers(const QRect &rect);

    ImageCanvas *mCanvas;
    int mLayerIndex;
    QPointF mNewLastPixelPenReleaseScenePos;
    QPointF mOldLastPixelPenReleaseScenePos;
    QPainter::CompositionMode mCompositionMode;

    QList<QPointF> mStroke;
    QRect mDrawBounds;
    QRegion mBufferRegion;
    QImage mUndoBuffer;
    QImage mRedoBuffer;
    QRect mBufferBounds;

    bool mAllowMerge;
    const QUndoCommand *const mPreviousCommand;

    bool needDraw;
};


#endif // APPLYPIXELLINECOMMAND_H
