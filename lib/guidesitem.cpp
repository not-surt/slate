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

#include "guidesitem.h"

#include "canvaspane.h"
#include "guide.h"
#include "imagecanvas.h"
#include "project.h"

#include <QPainter>

GuidesItem::GuidesItem(ImageCanvas *canvas) :
    QQuickPaintedItem(canvas),
    mCanvas(canvas)
{
    setObjectName("GuidesItem");
    setRenderTarget(FramebufferObject);
}

GuidesItem::~GuidesItem()
{
}

void GuidesItem::paint(QPainter *painter)
{
    for (int i = 0; i < mCanvas->panes().size(); ++i) {
        if (mCanvas->panes()[i]->visible()) drawPane(painter, mCanvas->panes()[i]);
    }
}

void GuidesItem::drawPane(QPainter *painter, const CanvasPane *pane)
{
    if (!mCanvas->project()->hasLoaded())
        return;

    painter->save();
    const QTransform transform = pane->transform();
    painter->setTransform(transform);
    painter->setClipRect(transform.inverted().map(pane->geometry()).boundingRect());

    QPen pen;
    pen.setColor(Qt::gray);
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);

    // Draw the guides.
    QVector<Guide> guides = mCanvas->project()->guides();
    for (int i = 0; i < guides.size(); ++i) {
        const Guide guide = guides.at(i);
        drawGuide(painter, pane, &guide, i);
    }

    // Draw the guide that's being dragged from the ruler, if any.
    if (mCanvas->pressedRuler()) {
        const bool horizontal = mCanvas->pressedRuler()->orientation() == Qt::Horizontal;
        const Guide guide(horizontal ? mCanvas->cursorScenePixelCorner().y() : mCanvas->cursorScenePixelCorner().x(), mCanvas->pressedRuler()->orientation());
        drawGuide(painter, pane, &guide, -1);
    }

    painter->restore();
}

void GuidesItem::drawGuide(QPainter *painter, const CanvasPane *pane, const Guide *guide, int guideIndex)
{
    // If this is an existing guide that is currently being dragged, draw it in its dragged position.
    const bool draggingExistingGuide = mCanvas->pressedGuideIndex() != -1 && mCanvas->pressedGuideIndex() == guideIndex;
    const bool vertical = guide->orientation() == Qt::Vertical;
    const int guidePosition = draggingExistingGuide ? (vertical ? mCanvas->cursorScenePixelCorner().x() : mCanvas->cursorScenePixelCorner().y()) : guide->position();

    painter->save();
    QPen pen;
    pen.setColor(Qt::cyan);
    pen.setWidth(0);
    painter->setPen(pen);

    const QRect visibleSceneArea = pane->transform().inverted().map(pane->geometry()).boundingRect().toAlignedRect();

    if (vertical) {
        // Don't bother drawing it if it's not visible within the scene.
        // Use visibleSceneArea.x()/y() to ensure that that coordinate is within the
        // scene area, as we don't care about it and are only testing the other coordinate.
        if (visibleSceneArea.contains(QPoint(guidePosition, visibleSceneArea.y()))) {
            // Don't need to account for the vertical offset anymore, as vertical guides go across the whole height of the pane.
            painter->drawLine(QLineF(guidePosition, visibleSceneArea.top(), guidePosition, visibleSceneArea.bottom()));
        }
    } else {
        if (visibleSceneArea.contains(QPoint(visibleSceneArea.x(), guidePosition))) {
            // Don't need to account for the horizontal offset anymore, as horizontal guides go across the whole width of the pane.
            painter->drawLine(QLineF(visibleSceneArea.left(), guidePosition, visibleSceneArea.right(), guidePosition));
        }
    }
    painter->restore();
}
