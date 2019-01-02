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

#include "canvaspaneitem.h"

#include "canvaspane.h"
#include "guide.h"
#include "imagecanvas.h"
#include "project.h"
#include "utils.h"

#include <QPainter>

/*
    This class is a purely visual respresentation of a canvas pane;
    ImageCanvas contains all of the state that will be painted, and this class paints it.
*/

CanvasPaneItem::CanvasPaneItem(QQuickItem *parent) :
    QQuickPaintedItem(parent)
{
    setObjectName("CanvasPaneItem");
    setRenderTarget(FramebufferObject);
}

CanvasPaneItem::~CanvasPaneItem()
{
}

ImageCanvas *CanvasPaneItem::canvas() const
{
    return mCanvas;
}

void CanvasPaneItem::setCanvas(ImageCanvas *canvas)
{
    if (canvas == mCanvas)
        return;

    if (mCanvas)
        disconnectFromCanvas();

    mCanvas = canvas;

    if (mCanvas)
        connectToCanvas();

    emit canvasChanged();
}

CanvasPane *CanvasPaneItem::pane() const
{
    return mPane;
}

void CanvasPaneItem::setPane(CanvasPane *pane)
{
    if (pane == mPane)
        return;

    mPane = pane;
    emit paneChanged();
}

int CanvasPaneItem::paneIndex() const
{
    return mPaneIndex;
}

void CanvasPaneItem::setPaneIndex(int paneIndex)
{
    if (paneIndex == mPaneIndex)
        return;

    mPaneIndex = paneIndex;
    emit paneIndexChanged();
}

void CanvasPaneItem::itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value)
{
    if (change == ItemVisibleHasChanged) {
        if (value.boolValue)
            connectToCanvas();
        else
            disconnectFromCanvas();
    }

    QQuickItem::itemChange(change, value);
}

void CanvasPaneItem::connectToCanvas()
{
    connect(mCanvas, &ImageCanvas::contentPaintRequested, this, &CanvasPaneItem::onContentPaintRequested);
}

void CanvasPaneItem::disconnectFromCanvas()
{
    mCanvas->disconnect(this);
}

void CanvasPaneItem::onContentPaintRequested(int paneIndex)
{
    if (paneIndex == -1 || paneIndex == mPaneIndex) {
        // Only schedule a re-paint if we were the pane it was requested for.
        update();
    }
}

void CanvasPaneItem::paint(QPainter *painter)
{
    if (!mCanvas->project() || !mCanvas->project()->hasLoaded())
        return;

    painter->save();
    const QTransform transform = mPane->transform();
    painter->setTransform(transform);
    const QRect sceneRect = transform.inverted().mapRect(QRectF(QPointF(0.0, 0.0), mPane->geometry().size())).toAlignedRect();
    painter->setClipRect(sceneRect);

    mCanvas->paintBackground(painter, sceneRect);
    mCanvas->paintContentWithPreview(painter, sceneRect);
    mCanvas->paintOverlay(painter, sceneRect);

    painter->restore();
}
