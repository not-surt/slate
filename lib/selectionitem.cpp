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

#include "selectionitem.h"

#include "canvaspane.h"
#include "imagecanvas.h"
#include "utils.h"

#include <QPainter>

SelectionItem::SelectionItem(ImageCanvas *canvas) :
    QQuickPaintedItem(canvas),
    mCanvas(canvas)
{
    setObjectName("SelectionItem");
    setVisible(false);

    connect(canvas, &ImageCanvas::hasSelectionChanged, [=]() { setVisible(mCanvas->hasSelection()); });
}

void SelectionItem::paint(QPainter *painter)
{
    for (int i = 0; i < mCanvas->panes().size(); ++i) {
        if (mCanvas->panes()[i]->visible())
            drawPane(painter, mCanvas->panes()[i]);
    }
}

void SelectionItem::drawPane(QPainter *painter, const CanvasPane *pane)
{
    if (!mCanvas->hasSelection())
        return;

    painter->save();
    const QTransform transform = pane->transform();
    painter->setTransform(transform);
    painter->setClipRect(transform.inverted().map(pane->geometry()).boundingRect());

    const QRect rect = painter->transform().mapRect(mCanvas->selectionArea());
    painter->save();
    painter->resetTransform();
    Utils::strokeRectWithDashes(painter, rect);
    painter->restore();

    painter->restore();
}
