/*
    Copyright 2018, Mitch Curtis

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

#include "selectioncursorguide.h"

#include "canvaspane.h"
#include "imagecanvas.h"

#include <QPainter>

// TODO: see if we can turn this into two thin items rather than one big one.
// could even just try having the two items as a child of this one

SelectionCursorGuide::SelectionCursorGuide(ImageCanvas *canvas) :
    QQuickPaintedItem(canvas),
    mCanvas(canvas)
{
    setObjectName("selectionCursorGuide");
}

SelectionCursorGuide::~SelectionCursorGuide()
{
}

void SelectionCursorGuide::paint(QPainter *painter)
{
    for (int i = 0; i < mCanvas->panes().size(); ++i) {
        if (mCanvas->panes()[i]->visible())
            drawPane(painter, mCanvas->panes()[i]);
    }
}

void SelectionCursorGuide::drawPane(QPainter *painter, const CanvasPane *pane)
{
    painter->save();
    const QTransform transform = pane->transform();
    painter->setTransform(transform);
    painter->setClipRect(transform.inverted().map(pane->geometry()).boundingRect());

    QPen pen;
    pen.setWidth(0);
    pen.setColor(Qt::gray);
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);

    const QRect visibleSceneArea = pane->transform().inverted().map(pane->geometry()).boundingRect().toAlignedRect();

    // Draw the vertical cursor selection guide.
    painter->drawLine(QLineF(mCanvas->cursorScenePixelCorner().x(), visibleSceneArea.top(), mCanvas->cursorScenePixelCorner().x(), visibleSceneArea.bottom()));

    // Draw the horizontal cursor selection guide.
    painter->drawLine(QLineF(visibleSceneArea.left(), mCanvas->cursorScenePixelCorner().y(), visibleSceneArea.right(),  mCanvas->cursorScenePixelCorner().y()));

    painter->restore();
}
