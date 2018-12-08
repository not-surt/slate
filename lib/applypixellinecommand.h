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
    ApplyPixelLineCommand(ImageCanvas *canvas, const QPointF point1, const QPointF point2,
        const QPointF &newLastPixelPenReleaseScenePos, const QPointF &oldLastPixelPenReleaseScenePos,
        const QPainter::CompositionMode mode, QUndoCommand *parent = nullptr);
    ~ApplyPixelLineCommand();

    void undo() override;
    void redo() override;

    int id() const override;
    bool mergeWith(const QUndoCommand *command) override;

private:
    friend QDebug operator<<(QDebug debug, const ApplyPixelLineCommand *command);

    void undoRect(QPainter &painter, const QRect &rect) const;
    void redoRect(QPainter &painter, const QRect &rect) const;
    void resizeUndoRedoBuffers(const QRect &rect);

    ImageCanvas *mCanvas;
    QPointF mNewLastPixelPenReleaseScenePos;
    QPointF mOldLastPixelPenReleaseScenePos;
    QPainter::CompositionMode mMode;

    QList<QPointF> mStroke;
    QList<QPointF> mOldStroke;
    QRect mDrawBounds;
    QRegion mImageRegion;
    QImage mUndoBuffer;
    QImage mRedoBuffer;
    QRect mImageBounds;

    bool needDraw;
};


#endif // APPLYPIXELLINECOMMAND_H
