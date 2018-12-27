﻿/*
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

#include "imagecanvas.h"

#include <QClipboard>
#include <QCursor>
#include <QEasingCurve>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QtMath>
#include <QBitmap>

#include "addguidecommand.h"
#include "applicationsettings.h"
#include "applygreedypixelfillcommand.h"
#include "applypixelfillcommand.h"
#include "applypixellinecommand.h"
#include "deleteguidecommand.h"
#include "deleteimagecanvasselectioncommand.h"
#include "fillalgorithms.h"
#include "flipimagecanvasselectioncommand.h"
#include "guidesitem.h"
#include "imageproject.h"
#include "modifyimagecanvasselectioncommand.h"
#include "moveguidecommand.h"
#include "pasteimagecanvascommand.h"
#include "project.h"
#include "selectioncursorguide.h"
#include "tileset.h"
#include "utils.h"
#include "brush.h"

Q_LOGGING_CATEGORY(lcImageCanvas, "app.canvas")
Q_LOGGING_CATEGORY(lcImageCanvasCursorShape, "app.canvas.cursorshape")
Q_LOGGING_CATEGORY(lcImageCanvasLifecycle, "app.canvas.lifecycle")
Q_LOGGING_CATEGORY(lcImageCanvasSelection, "app.canvas.selection")
Q_LOGGING_CATEGORY(lcImageCanvasSelectionCursorGuideVisibility, "app.canvas.selection.cursorguidevisibility")
Q_LOGGING_CATEGORY(lcImageCanvasSelectionPreviewImage, "app.canvas.selection.previewimage")

ImageCanvas::ImageCanvas() :
    mProject(nullptr),
    mImageProject(nullptr),
    mBackgroundColour(Qt::gray),
    mRulersVisible(true),
    mGridVisible(false),
    mGridColour(Qt::black),
    mSplitColour(Qt::black),
    mCheckerColour1(QColor::fromRgb(0x7e7e7e)),
    mCheckerColour2(Qt::white),
    mSplitScreen(false),
    mSplitter(this),
    mCurrentPane(&mFirstPane),
    mCurrentPaneIndex(0),
    mPanes(),
    mHorizontalRulers(),
    mVerticalRulers(),
    mFirstHorizontalRuler(nullptr),
    mFirstVerticalRuler(nullptr),
    mSecondHorizontalRuler(nullptr),
    mSecondVerticalRuler(nullptr),
    mPressedRuler(nullptr),
    mGuidesVisible(false),
    mGuidesLocked(false),
    mGuidePositionBeforePress(0),
    mPressedGuideIndex(-1),
    mGuidesItem(nullptr),
    mCursorPos(),
    mCursorPaneX(0),
    mCursorPaneY(0),
    mCursorScenePos(),
    mCursorPixelColour(Qt::black),
    mContainsMouse(false),
    mMouseButtonPressed(Qt::NoButton),
    mLastMouseButtonPressed(Qt::NoButton),
    mToolContinue(false),
    mOldStroke(), mNewStroke(),
    mScrollZoom(false),
    mGesturesEnabled(false),
    mTabletPressure(0.0),
    mIsTabletEvent(false),
    mTool(PenTool),
    mBrushType(Brush::SquareType),
    mToolBlendMode(ReplaceToolBlendMode),
    mLastFillToolUsed(FillTool),
    mLowerToolSize(0),
    mUpperToolSize(1),
    mMaxToolSize(100),
    mToolSizeUsePressure(false),
    mLowerToolOpacity(0.0),
    mUpperToolOpacity(1.0),
    mToolOpacityUsePressure(false),
    mPenForegroundColour(Qt::black),
    mPenBackgroundColour(Qt::white),
    mBrush(),
    mPotentiallySelecting(false),
    mHasSelection(false),
    mMovingSelection(false),
    mIsSelectionFromPaste(false),
    mConfirmingSelectionModification(false),
    mSelectionCursorGuide(nullptr),
    mLastSelectionModification(NoSelectionModification),
    mHasModifiedSelection(false),
    mAltPressed(false),
    mShiftPressed(false),
    mToolBeforeAltPressed(PenTool),
    mSpacePressed(false),
    mHasBlankCursor(false),
    mWindow(nullptr)
{
    setEnabled(false);
    setFlag(QQuickItem::ItemIsFocusScope);

    mFirstPane.setObjectName("firstPane");
    QQmlEngine::setObjectOwnership(&mFirstPane, QQmlEngine::CppOwnership);
    mSecondPane.setObjectName("secondPane");
    QQmlEngine::setObjectOwnership(&mSecondPane, QQmlEngine::CppOwnership);
    mSplitter.setPosition(0.5);

//    mPanes = {new CanvasPane(), new CanvasPane()};
    mPanes = {&mFirstPane, &mSecondPane};

    // We create child items in the body rather than the initialiser list
    // in order to ensure the correct drawing order.
    mGuidesItem = new GuidesItem(this);
    qreal itemZ = 3;
    mGuidesItem->setZ(itemZ++);

    mSelectionItem = new SelectionItem(this);
    mSelectionItem->setZ(itemZ++);

    mSelectionCursorGuide = new SelectionCursorGuide(this);
    mSelectionCursorGuide->setZ(itemZ++);

    mFirstHorizontalRuler = new Ruler(Qt::Horizontal, this);
    mFirstHorizontalRuler->setObjectName("firstHorizontalRuler");
    mFirstHorizontalRuler->setZ(itemZ++);

    mFirstVerticalRuler = new Ruler(Qt::Vertical, this);
    mFirstVerticalRuler->setObjectName("firstVerticalRuler");
    mFirstVerticalRuler->setDrawCorner(true);
    mFirstVerticalRuler->setZ(itemZ++);

    mSecondHorizontalRuler = new Ruler(Qt::Horizontal, this);
    mSecondHorizontalRuler->setObjectName("secondHorizontalRuler");
    mSecondHorizontalRuler->setZ(itemZ++);

    mSecondVerticalRuler = new Ruler(Qt::Vertical, this);
    mSecondVerticalRuler->setObjectName("secondVerticalRuler");
    mSecondVerticalRuler->setDrawCorner(true);
    mSecondVerticalRuler->setZ(itemZ++);

    mHorizontalRulers = {mFirstHorizontalRuler, mSecondHorizontalRuler};
    mVerticalRulers = {mFirstVerticalRuler, mSecondVerticalRuler};

    // Give some defaults so that the range slider handles aren't stuck together.
    mTexturedFillParameters.hue()->setVarianceLowerBound(-0.2);
    mTexturedFillParameters.hue()->setVarianceUpperBound(0.2);
    mTexturedFillParameters.saturation()->setVarianceLowerBound(-0.2);
    mTexturedFillParameters.saturation()->setVarianceUpperBound(0.2);
    mTexturedFillParameters.lightness()->setEnabled(true);
    mTexturedFillParameters.lightness()->setVarianceLowerBound(-0.2);
    mTexturedFillParameters.lightness()->setVarianceUpperBound(0.2);

    for (int i = 0; i < mPanes.size(); ++i) {
        connect(mPanes[i], &CanvasPane::zoomLevelChanged, this, &ImageCanvas::onZoomLevelChanged);
        connect(mPanes[i], &CanvasPane::offsetChanged, this, &ImageCanvas::onPaneOffsetChanged);
        connect(mPanes[i], &CanvasPane::sizeChanged, this, &ImageCanvas::onPaneSizeChanged);
    }
    connect(&mSplitter, &Splitter::positionChanged, this, &ImageCanvas::onSplitterPositionChanged);

    recreateCheckerImage();

    updateBrush();

    // Need to capture tablet events from application and window
    qApp->installEventFilter(this);
    QObject::connect(this, &QQuickItem::windowChanged, this, &ImageCanvas::updateWindow);

    qCDebug(lcImageCanvasLifecycle) << "constructing ImageCanvas" << this;
}

ImageCanvas::~ImageCanvas()
{
//    qDeleteAll(mPanes);

    qCDebug(lcImageCanvasLifecycle) << "destructing ImageCanvas" << this;
}

Project *ImageCanvas::project() const
{
    return mProject;
}

void ImageCanvas::setProject(Project *project)
{
    qCDebug(lcImageCanvas) << "setting project" << project << "on canvas" << this;

    if (project == mProject)
        return;

    if (mProject) {
        disconnectSignals();
    }

    mProject = project;
    mImageProject = qobject_cast<ImageProject*>(mProject);

    if (mProject) {
        connectSignals();

        // Read the canvas data that was stored in the project, if there is any.
        // New projects or projects that don't have their own Slate extension
        // won't have any JSON data.
        QJsonObject *cachedProjectJson = project->cachedProjectJson();

        if (cachedProjectJson->contains("lastFillToolUsed")) {
            const QString lastFillToolUsedAsString = cachedProjectJson->value("lastFillToolUsed").toString();
            setLastFillToolUsed(static_cast<Tool>(
                QMetaEnum::fromType<Tool>().keyToValue(qPrintable(lastFillToolUsedAsString))));
        }

        bool readPanes = false;
        if (cachedProjectJson->contains("firstPane")) {
            mFirstPane.read(cachedProjectJson->value("firstPane").toObject());
            readPanes = true;
        }
        if (cachedProjectJson->contains("secondPane")) {
            mSecondPane.read(cachedProjectJson->value("secondPane").toObject());
            readPanes = true;
        }
        doSetSplitScreen(cachedProjectJson->value("splitScreen").toBool(false));
        mSplitter.setEnabled(cachedProjectJson->value("splitterLocked").toBool(false));
        if (!readPanes) {
            // If there were no panes stored, then the project hasn't been saved yet,
            // so we can do what we want with the panes.
            setDefaultPaneSizes();
        }

        setAcceptedMouseButtons(Qt::AllButtons);
        setAcceptHoverEvents(true);
        setCursor(Qt::BlankCursor);
        setEnabled(true);
        forceActiveFocus();
    } else {
        setAcceptedMouseButtons(Qt::NoButton);
        setAcceptHoverEvents(false);
        setCursor(Qt::ArrowCursor);
        setEnabled(false);
    }

    emit projectChanged();
}

bool ImageCanvas::gridVisible() const
{
    return mGridVisible;
}

void ImageCanvas::setGridVisible(bool gridVisible)
{
    mGridVisible = gridVisible;
    requestContentPaint();
    emit gridVisibleChanged();
}

QColor ImageCanvas::gridColour() const
{
    return mGridColour;
}

void ImageCanvas::setGridColour(const QColor &gridColour)
{
    if (gridColour == mGridColour)
        return;

    mGridColour = gridColour;
    requestContentPaint();
    emit gridColourChanged();
}

bool ImageCanvas::rulersVisible() const
{
    return mRulersVisible;
}

void ImageCanvas::setRulersVisible(bool rulersVisible)
{
    if (rulersVisible == mRulersVisible)
        return;

    mRulersVisible = rulersVisible;

    mHorizontalRulers[0]->setVisible(rulersVisible);
    mVerticalRulers[0]->setVisible(rulersVisible);
    for (int i = 1; i < mPanes.size(); ++i) {
        mHorizontalRulers[i]->setVisible(rulersVisible && mSplitScreen);
        mVerticalRulers[i]->setVisible(rulersVisible && mSplitScreen);
    }

    emit rulersVisibleChanged();
}

bool ImageCanvas::guidesVisible() const
{
    return mGuidesVisible;
}

void ImageCanvas::setGuidesVisible(bool guidesVisible)
{
    if (guidesVisible == mGuidesVisible)
        return;

    mGuidesVisible = guidesVisible;

    mGuidesItem->setVisible(mGuidesVisible);
    if (mGuidesVisible)
        mGuidesItem->update();

    emit guidesVisibleChanged();
}

bool ImageCanvas::guidesLocked() const
{
    return mGuidesLocked;
}

void ImageCanvas::setGuidesLocked(bool guidesLocked)
{
    if (guidesLocked == mGuidesLocked)
        return;

    mGuidesLocked = guidesLocked;
    emit guidesLockedChanged();
}

QColor ImageCanvas::splitColour() const
{
    return mSplitColour;
}

void ImageCanvas::setSplitColour(const QColor &splitColour)
{
    if (splitColour == mSplitColour)
        return;

    mSplitColour = splitColour;
    requestContentPaint();
    emit splitColourChanged();
}

QColor ImageCanvas::checkerColour1() const
{
    return mCheckerColour1;
}

void ImageCanvas::setCheckerColour1(const QColor &colour)
{
    if (colour == mCheckerColour1)
        return;

    mCheckerColour1 = colour;
    recreateCheckerImage();
    emit checkerColour1Changed();
}

QColor ImageCanvas::checkerColour2() const
{
    return mCheckerColour2;
}

void ImageCanvas::setCheckerColour2(const QColor &colour)
{
    if (colour == mCheckerColour2)
        return;

    mCheckerColour2 = colour;
    recreateCheckerImage();
    emit checkerColour2Changed();
}

QColor ImageCanvas::backgroundColour() const
{
    return mBackgroundColour;
}

void ImageCanvas::setBackgroundColour(const QColor &backgroundColour)
{
    if (backgroundColour == mBackgroundColour)
        return;

    mBackgroundColour = backgroundColour;
    requestContentPaint();
    emit backgroundColourChanged();
}

QPoint ImageCanvas::cursorPos() const
{
    return mCursorPos;
}

void ImageCanvas::setCursorPos(const QPoint point)
{
    if (point == mCursorPos)
        return;

    mCursorPos = point;
    emit cursorPosChanged();
}

QPointF ImageCanvas::pressScenePos() const
{
    return mPressScenePosition;
}

QPoint ImageCanvas::pressScenePixel() const
{
    return QPoint(qFloor(mPressScenePosition.x()), qFloor(mPressScenePosition.y()));
}

QPoint ImageCanvas::pressScenePixelCorner() const
{
    return QPoint(qRound(mPressScenePosition.x()), qRound(mPressScenePosition.y()));
}

QPointF ImageCanvas::cursorScenePos() const
{
    return mCursorScenePos;
}

void ImageCanvas::setCursorScenePos(const QPointF point)
{
    if (point == mCursorScenePos)
        return;

    mCursorScenePos = point;
    if (isLineVisible())
        emit lineLengthChanged();
    emit cursorScenePosChanged();
}

QPoint ImageCanvas::cursorScenePixel() const
{
    return QPoint(qFloor(mCursorScenePos.x()), qFloor(mCursorScenePos.y()));
}

QPoint ImageCanvas::cursorScenePixelCorner() const
{
    return QPoint(qRound(mCursorScenePos.x()), qRound(mCursorScenePos.y()));
}

ImageCanvas::Tool ImageCanvas::tool() const
{
    return mTool;
}

void ImageCanvas::setTool(const Tool &tool)
{
    if (tool == mTool)
        return;

    mTool = tool;

    // The selection tool doesn't follow the undo rules, so we have to clear
    // the selected area if a different tool is chosen.
    if (mTool != SelectionTool) {
        if (mIsSelectionFromPaste)
            confirmPasteSelection();
        else
            clearOrConfirmSelection();
    }

    if (mTool == FillTool || mTool == TexturedFillTool)
        setLastFillToolUsed(mTool);

    updateSelectionCursorGuideVisibility();

    toolChange();

    emit toolChanged();
}

Brush::Type ImageCanvas::brushType() const
{
    return mBrushType;
}

void ImageCanvas::setBrushType(const Brush::Type &brushType)
{
    if (brushType == mBrushType)
        return;

    mBrushType = brushType;

    emit brushTypeChanged();
    updateBrush();
}

ImageCanvas::ToolBlendMode ImageCanvas::toolBlendMode() const
{
    return mToolBlendMode;
}

void ImageCanvas::setToolBlendMode(const ImageCanvas::ToolBlendMode &toolBlendMode)
{
    if (toolBlendMode == mToolBlendMode)
        return;

    mToolBlendMode = toolBlendMode;

    emit toolBlendModeChanged();
}

ImageCanvas::Tool ImageCanvas::lastFillToolUsed() const
{
    return mLastFillToolUsed;
}

void ImageCanvas::setLastFillToolUsed(Tool lastFillToolUsed)
{
    Q_ASSERT(lastFillToolUsed == FillTool || lastFillToolUsed == TexturedFillTool);
    qCDebug(lcImageCanvas) << "setting lastFillToolUsed to" << lastFillToolUsed;
    if (mLastFillToolUsed == lastFillToolUsed)
        return;

    mLastFillToolUsed = lastFillToolUsed;
    emit lastFillToolUsedChanged();
}

int ImageCanvas::lowerToolSize() const
{
    return mLowerToolSize;
}

void ImageCanvas::setLowerToolSize(int lowerToolSize)
{
    const int clamped = qBound(0, lowerToolSize, mMaxToolSize);
    if (clamped == mLowerToolSize)
        return;

    mLowerToolSize = clamped;
    emit lowerToolSizeChanged();
    if (mLowerToolSize > mUpperToolSize)
        setUpperToolSize(mLowerToolSize);
}

int ImageCanvas::upperToolSize() const
{
    return mUpperToolSize;
}

void ImageCanvas::setUpperToolSize(int upperToolSize)
{
    const int clamped = qBound(1, upperToolSize, mMaxToolSize);
    if (clamped == mUpperToolSize)
        return;

    mUpperToolSize = clamped;
    emit upperToolSizeChanged();
    if (mUpperToolSize < mLowerToolSize)
        setLowerToolSize(mUpperToolSize);

    updateBrush();
}

int ImageCanvas::maxToolSize() const
{
    return mMaxToolSize;
}

bool ImageCanvas::toolSizeUsePressure() const
{
    return mToolSizeUsePressure;
}

void ImageCanvas::setToolSizeUsePressure(bool toolSizeUsePressure)
{
    if (toolSizeUsePressure == mToolSizeUsePressure)
        return;

    mToolSizeUsePressure = toolSizeUsePressure;

    emit toolSizeUsePressureChanged();
}

QRectF ImageCanvas::brushRect()
{
    return mBrush.bounds();
}

qreal ImageCanvas::lowerToolOpacity() const
{
    return mLowerToolOpacity;
}

void ImageCanvas::setLowerToolOpacity(qreal lowerToolOpacity)
{
    const qreal clamped = qBound(0.0, lowerToolOpacity, 1.0);
    if (qFuzzyCompare(clamped, mLowerToolOpacity))
        return;

    mLowerToolOpacity = clamped;
    emit lowerToolOpacityChanged();
}

qreal ImageCanvas::upperToolOpacity() const
{
    return mUpperToolOpacity;
}

void ImageCanvas::setUpperToolOpacity(qreal upperToolOpacity)
{
    const qreal clamped = qBound(0.0, upperToolOpacity, 1.0);
    if (qFuzzyCompare(clamped, mUpperToolOpacity))
        return;

    mUpperToolOpacity = clamped;
    emit upperToolOpacityChanged();
}

bool ImageCanvas::toolOpacityUsePressure() const {
    return mToolOpacityUsePressure;
}

void ImageCanvas::setToolOpacityUsePressure(bool toolOpacityUsePressure)
{
    if (toolOpacityUsePressure == mToolOpacityUsePressure)
        return;

    mToolOpacityUsePressure = toolOpacityUsePressure;

    emit toolOpacityUsePressureChanged();
}

QColor ImageCanvas::penForegroundColour() const
{
    return mPenForegroundColour;
}

void ImageCanvas::setPenForegroundColour(const QColor &penForegroundColour)
{
    if (!penForegroundColour.isValid()) {
        qWarning() << "Invalid penForegroundColour";
        return;
    }

    const QColor colour = penForegroundColour.toRgb();
    if (colour == mPenForegroundColour)
        return;

    mPenForegroundColour = colour;
    emit penForegroundColourChanged();
}

QColor ImageCanvas::penBackgroundColour() const
{
    return mPenBackgroundColour;
}

void ImageCanvas::setPenBackgroundColour(const QColor &penBackgroundColour)
{
    if (!penBackgroundColour.isValid()) {
        qWarning() << "Invalid penBackgroundColour";
        return;
    }

    const QColor colour = penBackgroundColour.toRgb();
    if (colour == mPenBackgroundColour)
        return;

    mPenBackgroundColour = colour;
    emit penBackgroundColourChanged();
}

TexturedFillParameters *ImageCanvas::texturedFillParameters()
{
    return &mTexturedFillParameters;
}

bool ImageCanvas::hasSelection() const
{
    return mHasSelection;
}

bool ImageCanvas::hasModifiedSelection() const
{
    return mHasModifiedSelection;
}

QRect ImageCanvas::selectionArea() const
{
    return mSelectionArea;
}

void ImageCanvas::setSelectionArea(const QRect &selectionArea)
{
    QRect adjustedSelectionArea = selectionArea;
    if (!mMouseButtonPressed && selectionArea.size().isEmpty())
        adjustedSelectionArea = QRect();

    if (adjustedSelectionArea == mSelectionArea)
        return;

    mSelectionArea = adjustedSelectionArea;
    setHasSelection(!mSelectionArea.isEmpty());
    mSelectionItem->update();
    emit selectionAreaChanged();
}

bool ImageCanvas::isAdjustingImage() const
{
    return !mSelectionContentsBeforeImageAdjustment.isNull();
}

QColor ImageCanvas::cursorPixelColour() const
{
    return mCursorPixelColour;
}

QColor ImageCanvas::invertedCursorPixelColour() const
{
    return invertedColour(mCursorPixelColour);
}

QColor ImageCanvas::invertedColour(const QColor &colour)
{
    // https://stackoverflow.com/a/18142036/904422
    return (0xFFFFFF - colour.rgba()) | 0xFF000000;
}

void ImageCanvas::setCursorPixelColour(const QColor &cursorPixelColour)
{
    if (cursorPixelColour == mCursorPixelColour)
        return;

    mCursorPixelColour = cursorPixelColour;
    emit cursorPixelColourChanged();
}

bool ImageCanvas::isWithinImage(const QPoint &scenePos) const
{
    return scenePos.x() >= 0 && scenePos.x() < currentProjectImage()->width()
            && scenePos.y() >= 0 && scenePos.y() < currentProjectImage()->height();
}

QPoint ImageCanvas::clampToImageBounds(const QPoint &scenePos, bool inclusive) const
{
    return QPoint(qBound(0, scenePos.x(), currentProjectImage()->width() - (inclusive ? 0 : 1)),
                  qBound(0, scenePos.y(), currentProjectImage()->height() - (inclusive ? 0 : 1)));
}

bool ImageCanvas::containsMouse() const
{
    return mContainsMouse;
}

void ImageCanvas::setContainsMouse(bool containsMouse)
{
    if (containsMouse == mContainsMouse)
        return;

    mContainsMouse = containsMouse;
    updateWindowCursorShape();
    // Ensure that the selection cursor guide isn't still drawn when the mouse
    // is outside of us (e.g. over a panel).
    updateSelectionCursorGuideVisibility();

    emit containsMouseChanged();
}

bool ImageCanvas::isSplitScreen() const
{
    return mSplitScreen;
}

void ImageCanvas::setSplitScreen(bool splitScreen)
{
    doSetSplitScreen(splitScreen);
}

bool ImageCanvas::scrollZoom() const
{
    return mScrollZoom;
}

void ImageCanvas::setScrollZoom(bool scrollZoom)
{
    if (scrollZoom == mScrollZoom)
        return;

    mScrollZoom = scrollZoom;
    emit scrollZoomChanged();
}

bool ImageCanvas::areGesturesEnabled() const
{
    return mGesturesEnabled;
}

void ImageCanvas::setGesturesEnabled(bool gesturesEnabled)
{
    if (gesturesEnabled == mGesturesEnabled)
        return;

    mGesturesEnabled = gesturesEnabled;
    emit gesturesEnabledChanged();
}

Ruler *ImageCanvas::pressedRuler() const
{
    return mPressedRuler;
}

int ImageCanvas::pressedGuideIndex() const
{
    return mPressedGuideIndex;
}

CanvasPane *ImageCanvas::firstPane()
{
    return &mFirstPane;
}

const CanvasPane *ImageCanvas::firstPane() const
{
    return &mFirstPane;
}

CanvasPane *ImageCanvas::secondPane()
{
    return &mSecondPane;
}

const CanvasPane *ImageCanvas::secondPane() const
{
    return &mSecondPane;
}

CanvasPane *ImageCanvas::currentPane()
{
    return mCurrentPane;
}

CanvasPane *ImageCanvas::paneAt(int index)
{
    if (index < 0 || index >= mPanes.size())
        return nullptr;

    return mPanes[index];
}

const CanvasPane *ImageCanvas::paneAt(int index) const
{
    return const_cast<ImageCanvas *>(this)->paneAt(index);
}

const QVector<CanvasPane *> &ImageCanvas::panes() const
{
    return mPanes;
}

int ImageCanvas::paneWidth(int index) const
{
    return qRound(mPanes[index]->geometry().width());
}

QColor ImageCanvas::rulerForegroundColour() const
{
    return mHorizontalRulers[0]->foregroundColour();
}

void ImageCanvas::setRulerForegroundColour(const QColor &foregroundColour) const
{
    for (int i = 0; i < mPanes.size(); ++i) {
        mHorizontalRulers[i]->setForegroundColour(foregroundColour);
        mVerticalRulers[i]->setForegroundColour(foregroundColour);
    }
}

QColor ImageCanvas::rulerBackgroundColour() const
{
    return mHorizontalRulers[0]->backgroundColour();
}

void ImageCanvas::setRulerBackgroundColour(const QColor &backgroundColour) const
{
    for (int i = 0; i < mPanes.size(); ++i) {
        mHorizontalRulers[i]->setBackgroundColour(backgroundColour);
        mVerticalRulers[i]->setBackgroundColour(backgroundColour);
    }
}

Splitter *ImageCanvas::splitter()
{
    return &mSplitter;
}

QColor ImageCanvas::mapBackgroundColour() const
{
    return mBackgroundColour;
}

bool ImageCanvas::isAltPressed() const
{
    return mAltPressed;
}

bool ImageCanvas::isLineVisible() const
{
    // Don't show line info in the status bar if there hasn't been a mouse press yet.
    // This is the same as what penColour() does.
    const Qt::MouseButton lastButtonPressed = mMouseButtonPressed == Qt::NoButton ? mLastMouseButtonPressed : mMouseButtonPressed;
    return !mOldStroke.isEmpty() && mShiftPressed && mTool == PenTool && lastButtonPressed != Qt::NoButton;
}

int ImageCanvas::lineLength() const
{
    if (!isLineVisible())
        return 0;

    const QPointF point1 = mOldStroke.last().pixel();
    const QPointF point2 = mCursorScenePos;
    const QLineF line(point1, point2);
    return qRound(line.length());
}

qreal ImageCanvas::lineAngle() const
{
    if (!isLineVisible())
        return 0;

    const QPointF point1 = mOldStroke.last().pixel();
    const QPointF point2 = mCursorScenePos;
    const QLineF line(point1, point2);
    return line.angle();
}

ImageCanvas::SubImage ImageCanvas::getSubImage(const int index) const
{
    Q_ASSERT(index == 0);

    return {0, mProject->bounds(), {0, 0}};
}

QList<ImageCanvas::SubImageInstance> ImageCanvas::subImageInstancesInBounds(const QRect &bounds) const
{
    QList<SubImageInstance> instances;
    if (bounds.intersects(mProject->bounds())) {
        instances.append({0, {0, 0}});
    }
    return instances;
}

QColor ImageCanvas::pickColour(const QPointF point) const
{
    const QPoint pixel{qFloor(point.x()), qFloor(point.y())};
    auto instances = subImageInstancesInBounds(QRect(pixel, QSize(1, 1)));
    if (!instances.isEmpty()) {
        auto instance = instances.last();
        auto subImage = getSubImage(instance.index);
        auto image = imageForLayerAt(mProject->currentLayerIndex());
        const QPoint instanceDrawOffset = subImage.bounds.topLeft() - instance.position;
        return image->pixelColor(pixel + instanceDrawOffset);
    }
    else return QColor();
}

void ImageCanvas::setAltPressed(bool altPressed)
{
    if (altPressed == mAltPressed)
        return;

    mAltPressed = altPressed;
    emit altPressedChanged();
}

void ImageCanvas::setShiftPressed(bool shiftPressed)
{
    if (shiftPressed == mShiftPressed)
        return;

    const bool wasLineVisible = isLineVisible();

    mShiftPressed = shiftPressed;

    if (isLineVisible() != wasLineVisible)
        emit lineVisibleChanged();

    requestContentPaint();
}

void ImageCanvas::connectSignals()
{
    qCDebug(lcImageCanvas) << "connecting signals for" << this << "as we have a new project" << mProject;

    connect(mProject, SIGNAL(loadedChanged()), this, SLOT(onLoadedChanged()));
    connect(mProject, SIGNAL(projectCreated()), this, SLOT(requestContentPaint()));
    connect(mProject, SIGNAL(projectClosed()), this, SLOT(reset()));
    connect(mProject, SIGNAL(sizeChanged()), this, SLOT(requestContentPaint()));
    connect(mProject, SIGNAL(guidesChanged()), this, SLOT(onGuidesChanged()));
    connect(mProject, SIGNAL(readyForWritingToJson(QJsonObject*)),
        this, SLOT(onReadyForWritingToJson(QJsonObject*)));
    connect(mProject, SIGNAL(aboutToBeginMacro(QString)),
        this, SLOT(onAboutToBeginMacro(QString)));

    connect(window(), SIGNAL(activeFocusItemChanged()), this, SLOT(updateWindowCursorShape()));
}

void ImageCanvas::disconnectSignals()
{
    qCDebug(lcImageCanvas) << "disconnecting signals for" << this;

    mProject->disconnect(SIGNAL(loadedChanged()), this, SLOT(onLoadedChanged()));
    mProject->disconnect(SIGNAL(projectCreated()), this, SLOT(requestContentPaint()));
    mProject->disconnect(SIGNAL(projectClosed()), this, SLOT(reset()));
    mProject->disconnect(SIGNAL(sizeChanged()), this, SLOT(requestContentPaint()));
    mProject->disconnect(SIGNAL(guidesChanged()), this, SLOT(onGuidesChanged()));
    mProject->disconnect(SIGNAL(readyForWritingToJson(QJsonObject*)),
        this, SLOT(onReadyForWritingToJson(QJsonObject*)));
    mProject->disconnect(SIGNAL(aboutToBeginMacro(QString)),
        this, SLOT(onAboutToBeginMacro(QString)));

    if (window()) {
        window()->disconnect(SIGNAL(activeFocusItemChanged()), this, SLOT(updateWindowCursorShape()));
    }
}

void ImageCanvas::toolChange()
{
}

bool ImageCanvas::hasBlankCursor() const
{
    return mHasBlankCursor;
}

void ImageCanvas::onSplitterPositionChanged()
{
    updatePaneGeometry();
}

void ImageCanvas::componentComplete()
{
    QQuickItem::componentComplete();

    // For some reason, we need to force this stuff to update when creating a
    // tileset project after a layered image project.
    updateRulerVisibility();
    updatePaneGeometry();

    updateSelectionCursorGuideVisibility();
    mGuidesItem->setVisible(mGuidesVisible);

    resizeChildren();

    requestContentPaint();
}

void ImageCanvas::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);

    if (newGeometry == oldGeometry) return;

    updatePaneGeometry();
    resizeChildren();

    if (mProject)
        updateCursorPos(mCursorPos);
}

void ImageCanvas::resizeChildren()
{
    mSelectionCursorGuide->setWidth(width());
    mSelectionCursorGuide->setHeight(height());

    mGuidesItem->setWidth(qFloor(width()));
    mGuidesItem->setHeight(height());

    mSelectionItem->setWidth(width());
    mSelectionItem->setHeight(height());
}

QImage *ImageCanvas::currentProjectImage()
{
    return mImageProject->image();
}

const QImage *ImageCanvas::currentProjectImage() const
{
    return const_cast<ImageCanvas *>(this)->currentProjectImage();
}

QImage *ImageCanvas::imageForLayerAt(int layerIndex)
{
    Q_ASSERT(layerIndex == -1);
    return mImageProject->image();
}

const QImage *ImageCanvas::imageForLayerAt(int layerIndex) const
{
    return const_cast<ImageCanvas *>(this)->imageForLayerAt(layerIndex);
}

int ImageCanvas::currentLayerIndex() const
{
    return -1;
}

void ImageCanvas::paintBackground(QPainter *const painter) const
{
    QBrush brush(mCheckerPixmap);
    // Invert painter transform for background pattern so unaffected by painter zoom and pan
    brush.setTransform(painter->transform().inverted());

    // Draw the checkered pixmap that acts as an indicator for transparency
    painter->fillRect(0, 0, currentProjectImage()->width(), currentProjectImage()->height(), brush);
}

void ImageCanvas::paintContent(QPainter *const painter)
{
    painter->drawImage(QPointF(0, 0), getComposedImage());
}

void ImageCanvas::paintContentWithPreview(QPainter *const painter)
{
    static const QSet<Tool> previewTools{PenTool, EraserTool};
    QUndoStack tempUndoStack;

    const bool showPreview = containsMouse() && (mProject->settings()->isBrushPreviewVisible() || isLineVisible()) && previewTools.contains(mTool);

    // Draw the brush/line preview to the image
    if (showPreview) {
        applyCurrentTool(&tempUndoStack);
    }

    paintContent(painter);

    // Undo drawing the brush/line preview
    if (showPreview) {
        tempUndoStack.undo();
    }
}

QImage ImageCanvas::contentImage()
    mCachedContentImage = getComposedImage();
    return mCachedContentImage;
QImage ImageCanvas::getComposedImage()
{
    return !shouldDrawSelectionPreviewImage() ? *currentProjectImage() : mSelectionPreviewImage;
}

void ImageCanvas::doSetSplitScreen(bool splitScreen)
{
    if (splitScreen == mSplitScreen)
        return;

    mSplitScreen = splitScreen;

    for (int i = 1; i < mPanes.size(); ++i) {
        mPanes[i]->setVisible(mSplitScreen);
    }

    updatePaneGeometry();
    updateRulerVisibility();

    requestContentPaint();

    emit splitScreenChanged();
}

void ImageCanvas::setDefaultPaneSizes()
{
    mSplitter.setPosition(0.5);
}

void ImageCanvas::updatePaneGeometry()
{
    const qreal split = mSplitScreen ? mSplitter.position() : 1.0;
    mFirstPane.setGeometry(QRectF(QPointF(0.0, 0.0), QSizeF(split * width(), height())));
    mSecondPane.setGeometry(QRectF(QPointF(split * width(), 0.0), QSizeF((1.0 - split) * width(), height())));

    resizeRulers();
    if (mGuidesVisible) mGuidesItem->update();
    mSelectionItem->update();

    requestContentPaint();
}

bool ImageCanvas::mouseOverSplitterHandle(const QPoint &mousePos)
{
    const QRect splitterRegion(paneWidth(0) - mSplitter.width() / 2, 0, mSplitter.width(), height());
    return splitterRegion.contains(mousePos);
}

void ImageCanvas::updateRulerVisibility()
{
    for (int i = 0; i < mPanes.size(); ++i) {
        mHorizontalRulers[i]->setVisible(mRulersVisible && mPanes[i]->visible());
        mVerticalRulers[i]->setVisible(mRulersVisible && mPanes[i]->visible());
    }
}

void ImageCanvas::resizeRulers()
{
    for (int i = 0; i < mPanes.size(); ++i) {
        mHorizontalRulers[i]->setPosition(mPanes[i]->geometry().topLeft());
        mHorizontalRulers[i]->setSize(QSizeF(mPanes[i]->geometry().width(), 20));
        mVerticalRulers[i]->setPosition(mPanes[i]->geometry().topLeft());
        mVerticalRulers[i]->setSize(QSizeF(20, mPanes[i]->geometry().height()));
    }
}

void ImageCanvas::updatePressedRuler()
{
    mPressedRuler = !guidesLocked() ? rulerAtCursorPos() : nullptr;
}

Ruler *ImageCanvas::rulerAtCursorPos()
{
    Ruler *ruler = nullptr;

    for (int i = 0; i < mPanes.size(); ++i) {
        if (mHorizontalRulers[i]->isVisible() && mHorizontalRulers[i]->contains(mapToItem(mHorizontalRulers[i], mCursorPos))) {
            ruler = mHorizontalRulers[i];
        }
        else if (mVerticalRulers[i]->isVisible() && mVerticalRulers[i]->contains(mapToItem(mVerticalRulers[i], mCursorPos))) {
            ruler = mVerticalRulers[i];
        }
    }

    return ruler;
}

void ImageCanvas::addNewGuide()
{
    mProject->beginMacro(QLatin1String("AddGuide"));
    mProject->addChange(new AddGuideCommand(mProject, Guide(
        mPressedRuler->orientation() == Qt::Horizontal ? cursorScenePixel().y() : cursorScenePixel().x(),
        mPressedRuler->orientation())));
    mProject->endMacro();

    // The update for these guide commands happens in onGuidesChanged.
}

void ImageCanvas::moveGuide()
{
    const Guide guide = mProject->guides().at(mPressedGuideIndex);

    mProject->beginMacro(QLatin1String("MoveGuide"));
    mProject->addChange(new MoveGuideCommand(mProject, guide,
        guide.orientation() == Qt::Horizontal ? cursorScenePixel().y() : cursorScenePixel().x()));
    mProject->endMacro();
}

void ImageCanvas::removeGuide()
{
    const Guide guide = mProject->guides().at(mPressedGuideIndex);

    mProject->beginMacro(QLatin1String("DeleteGuide"));
    mProject->addChange(new DeleteGuideCommand(mProject, guide));
    mProject->endMacro();
}

void ImageCanvas::updatePressedGuide()
{
    mPressedGuideIndex = guideIndexAtCursorPos();

    if (mPressedGuideIndex != -1)
        mGuidePositionBeforePress = mProject->guides().at(mPressedGuideIndex).position();
}

int ImageCanvas::guideIndexAtCursorPos()
{
    const QVector<Guide> guides = mProject->guides();
    for (int i = 0; i < guides.size(); ++i) {
        const Guide guide = guides.at(i);
        if (guide.orientation() == Qt::Horizontal) {
            if (cursorScenePixel().y() == guide.position()) {
                return i;
            }
        } else {
            if (cursorScenePixel().x() == guide.position()) {
                return i;
            }
        }
    }

    return -1;
}

void ImageCanvas::onGuidesChanged()
{
    mGuidesItem->update();
}

// TODO: make projectJson a reference to make this neater
void ImageCanvas::onReadyForWritingToJson(QJsonObject *projectJson)
{
    (*projectJson)["lastFillToolUsed"] = QMetaEnum::fromType<Tool>().valueToKey(mLastFillToolUsed);

    QJsonObject firstPaneJson;
    mFirstPane.write(firstPaneJson);
    (*projectJson)["firstPane"] = firstPaneJson;

    QJsonObject secondPaneJson;
    mSecondPane.write(secondPaneJson);
    (*projectJson)["secondPane"] = secondPaneJson;

    if (mSplitScreen)
        (*projectJson)["splitScreen"] = true;
    if (mSplitter.isEnabled())
        (*projectJson)["splitterLocked"] = true;
}

void ImageCanvas::onAboutToBeginMacro(const QString &macroText)
{
    // See Project::beginMacro() for the justification for this function's existence.
    if (mConfirmingSelectionModification)
        return;

    if (macroText.contains(QLatin1String("Selection"))) {
        // The macro involves a selection, so we shouldn't clear it.
        return;
    }

    clearOrConfirmSelection();
}

void ImageCanvas::recreateCheckerImage()
{
    mCheckerImage = QImage(32, 32, QImage::Format_ARGB32_Premultiplied);

    QPainter painter(&mCheckerImage);
    int i = 0;
    for (int y = 0; y < mCheckerImage.height(); y += 8) {
        for (int x = 0; x < mCheckerImage.width(); x += 8, ++i) {
            const int row = y / 8;
            if (i % 2 == 0)
                painter.fillRect(QRect(x, y, 8, 8), row % 2 == 0 ? mCheckerColour2 : mCheckerColour1);
            else
                painter.fillRect(QRect(x, y, 8, 8), row % 2 == 0 ? mCheckerColour1 : mCheckerColour2);
        }
    }

    mCheckerPixmap = QPixmap::fromImage(mCheckerImage);

    requestContentPaint();
}

bool ImageCanvas::isPanning() const
{
    // Pressing the mouse while holding down space (or using middle mouse button) should pan.
    return mSpacePressed || mMouseButtonPressed == Qt::MiddleButton;
}

bool ImageCanvas::supportsSelectionTool() const
{
    return true;
}

void ImageCanvas::beginSelectionMove()
{
    qCDebug(lcImageCanvasSelection) << "beginning selection move... mIsSelectionFromPaste =" << mIsSelectionFromPaste;

    setMovingSelection(true);
    mSelectionAreaBeforeLastMove = mSelectionArea;

    if (mSelectionAreaBeforeFirstModification.isEmpty()) {
        // When the selection is moved for the first time in its life,
        // copy the contents within it so that we can moved them around as a preview.
        qCDebug(lcImageCanvasSelection) << "copying currentProjectImage()" << *currentProjectImage() << "into mSelectionContents";
        mSelectionAreaBeforeFirstModification = mSelectionArea;
        mSelectionContents = currentProjectImage()->copy(mSelectionAreaBeforeFirstModification);
        // Technically we don't need to call this until the selection has actually moved,
        // but updateCursorPos() calls pixelColor() on the result of contentImage(), which will be an invalid
        // image until we've updated the selection preview image (since shouldDrawSelectionPreviewImage() will
        // return true due to mMovingSelection being true).
        updateSelectionPreviewImage(SelectionMove);
    }
}

void ImageCanvas::updateOrMoveSelectionArea()
{
    if (!mMovingSelection) {
        updateSelectionArea();
    } else {
        moveSelectionArea();
    }
}

void ImageCanvas::updateSelectionArea()
{
    if (!mPotentiallySelecting) {
        // updateSelectionArea() can be called by updateOrMoveSelectionArea() as a result
        // of moving after panning (all without releasing the mouse). In that case,
        // we can't be selecting, as we were just panning, so we return early.
        // Previously we would assert that mPotentiallySelecting was true, but that's too strict.
        return;
    }

    QRect newSelectionArea(QPoint(qMin(pressScenePixelCorner().x(), cursorScenePixelCorner().x()), qMin(pressScenePixelCorner().y(), cursorScenePixelCorner().y())),
                           QPoint(qMax(pressScenePixelCorner().x(), cursorScenePixelCorner().x()) - 1, qMax(pressScenePixelCorner().y(), cursorScenePixelCorner().y()) - 1));
    newSelectionArea = clampSelectionArea(newSelectionArea);
    setSelectionArea(newSelectionArea);
}

void ImageCanvas::updateSelectionPreviewImage(SelectionModification reason)
{
    qCDebug(lcImageCanvasSelectionPreviewImage) << "updating selection preview image due to" << reason;

    if (!mIsSelectionFromPaste) {
        // Only if the selection wasn't pasted should we erase the area left behind.
        mSelectionPreviewImage = Utils::erasePortionOfImage(*currentProjectImage(), mSelectionAreaBeforeFirstModification);
        qCDebug(lcImageCanvasSelectionPreviewImage) << "... selection is not from paste; erasing area left behind"
            << "- new selection preview image:" << mSelectionPreviewImage;
    } else {
        mSelectionPreviewImage = *currentProjectImage();
        qCDebug(lcImageCanvasSelectionPreviewImage) << "... selection is from a paste; not touching existing canvas content"
            << "- new selection preview image:" << mSelectionPreviewImage;
    }

    // Then, move the dragged contents to their new location.
    // Doing this last ensures that the drag contents are painted over the transparency,
    // and not the other way around.
    qCDebug(lcImageCanvasSelectionPreviewImage) << "painting selection contents" << mSelectionContents
       << "within selection area" << mSelectionArea << "over top of current project image" << mSelectionPreviewImage;
    mSelectionPreviewImage = Utils::paintImageOntoPortionOfImage(mSelectionPreviewImage, mSelectionArea, mSelectionContents);
}

void ImageCanvas::moveSelectionArea()
{
//    qCDebug(lcImageCanvasSelection) << "moving selection area... mIsSelectionFromPaste =" << mIsSelectionFromPaste;

    QRect newSelectionArea = mSelectionAreaBeforeLastMove;
    const QPoint distanceMoved(cursorScenePixel().x() - pressScenePixel().x(), cursorScenePixel().y() - pressScenePixel().y());
    newSelectionArea.translate(distanceMoved);
    setSelectionArea(boundSelectionArea(newSelectionArea));

    // TODO: move this to be second-last once all tests are passing
    updateSelectionPreviewImage(SelectionMove);

    setLastSelectionModification(SelectionMove);

    requestContentPaint();
}

void ImageCanvas::moveSelectionAreaBy(const QPoint &pixelDistance)
{
    qCDebug(lcImageCanvasSelection) << "moving selection area by" << pixelDistance;

    // Moving a selection with the directional keys creates a single move command instantly.
    beginSelectionMove();

    const QRect newSelectionArea = mSelectionArea.translated(pixelDistance.x(), pixelDistance.y());
    setSelectionArea(boundSelectionArea(newSelectionArea));

    // see TODO in the function above
    updateSelectionPreviewImage(SelectionMove);

    setLastSelectionModification(SelectionMove);

    setMovingSelection(false);
    mLastValidSelectionArea = mSelectionArea;

    // setSelectionArea() should do this anyway, but just in case..
    requestContentPaint();
}

void ImageCanvas::confirmSelectionModification()
{
    Q_ASSERT(mLastSelectionModification != NoSelectionModification);
    qCDebug(lcImageCanvasSelection) << "confirming selection modification" << mLastSelectionModification;

    const QImage sourceAreaImage = currentProjectImage()->copy(mSelectionAreaBeforeFirstModification);
    const QImage targetAreaImageBeforeModification = currentProjectImage()->copy(mLastValidSelectionArea);
    const QImage targetAreaImageAfterModification = mSelectionContents;

    // Calling beginMacro() causes Project::aboutToBeginMacro() to be
    // emitted, and we're connected to it, so we have to avoid recursing.
    mConfirmingSelectionModification = true;

    mProject->beginMacro(QLatin1String("ModifySelection"));
    mProject->addChange(new ModifyImageCanvasSelectionCommand(
        this, currentLayerIndex(), mLastSelectionModification,
        mSelectionAreaBeforeFirstModification, sourceAreaImage,
        mLastValidSelectionArea, targetAreaImageBeforeModification, targetAreaImageAfterModification,
        mIsSelectionFromPaste, (mIsSelectionFromPaste ? mSelectionContents : QImage())));
    mProject->endMacro();

    mConfirmingSelectionModification = false;

    clearSelection();
}

// Limits selectionArea to the canvas' bounds, shrinking it if necessary.
QRect ImageCanvas::clampSelectionArea(const QRect &selectionArea) const
{
    QRect newSelectionArea = selectionArea;

    if (selectionArea.x() < 0) {
        newSelectionArea.setLeft(0);
    }

    if (selectionArea.y() < 0) {
        newSelectionArea.setTop(0);
    }

    if (newSelectionArea.width() < 0 || newSelectionArea.height() < 0
            || newSelectionArea.x() >= mProject->widthInPixels()
            || newSelectionArea.y() >= mProject->heightInPixels()) {
        newSelectionArea.setSize(QSize(0, 0));
    }

    if (newSelectionArea.x() + newSelectionArea.width() > mProject->widthInPixels()) {
        newSelectionArea.setWidth(mProject->widthInPixels() - newSelectionArea.x());
    }

    if (newSelectionArea.y() + newSelectionArea.height() > mProject->heightInPixels()) {
        newSelectionArea.setHeight(mProject->heightInPixels() - newSelectionArea.y());
    }

    if (newSelectionArea.width() < 0 || newSelectionArea.height() < 0) {
        newSelectionArea.setSize(QSize(0, 0));
    }

    return newSelectionArea;
}

// Limits selectionArea to the canvas' bounds without shrinking it.
// This should be used when the selection area has already been created and is being dragged.
QRect ImageCanvas::boundSelectionArea(const QRect &selectionArea) const
{
    return Utils::ensureWithinArea(selectionArea, mProject->size());
}

void ImageCanvas::clearSelection()
{
    qCDebug(lcImageCanvasSelection) << "clearing selection";

    setSelectionArea(QRect());
    mPotentiallySelecting = false;
    setHasSelection(false);
    setMovingSelection(false);
    setSelectionFromPaste(false);
    mSelectionAreaBeforeFirstModification = QRect(0, 0, 0, 0);
    mSelectionAreaBeforeLastMove = QRect(0, 0, 0, 0);
    mLastValidSelectionArea = QRect(0, 0, 0, 0);
    mSelectionPreviewImage = QImage();
    mSelectionContents = QImage();
    setLastSelectionModification(NoSelectionModification);
    setHasModifiedSelection(false);
}

void ImageCanvas::clearOrConfirmSelection()
{
    if (mHasSelection) {
        if (mLastSelectionModification != NoSelectionModification && mLastSelectionModification != SelectionPaste)
            confirmSelectionModification();
        else
            clearSelection();
    }
}

void ImageCanvas::setHasSelection(bool hasSelection)
{
    if (hasSelection == mHasSelection)
        return;

    mHasSelection = hasSelection;
    updateWindowCursorShape();
    updateSelectionCursorGuideVisibility();
    emit hasSelectionChanged();
}

void ImageCanvas::setMovingSelection(bool movingSelection)
{
    mMovingSelection = movingSelection;
}

bool ImageCanvas::cursorOverSelection() const
{
    return mHasSelection ? mSelectionArea.contains(cursorScenePixel()) : false;
}

bool ImageCanvas::shouldDrawSelectionPreviewImage() const
{
    return mMovingSelection || mIsSelectionFromPaste
        || mLastSelectionModification != NoSelectionModification;
}

bool ImageCanvas::shouldDrawSelectionCursorGuide() const
{
    return mTool == SelectionTool && !mHasSelection && mContainsMouse;
}

void ImageCanvas::updateSelectionCursorGuideVisibility()
{
    qCDebug(lcImageCanvasSelectionCursorGuideVisibility)
        << "mTool == SelectionTool:" << (mTool == SelectionTool)
        << "!mHasSelection:" << !mHasSelection
        << "mContainsMouse:" << mContainsMouse;

    const bool wasVisible = mSelectionCursorGuide->isVisible();

    mSelectionCursorGuide->setVisible(shouldDrawSelectionCursorGuide());

    // It seems to be necessary to request an update after making the guide visible.
    if (mSelectionCursorGuide->isVisible() && !wasVisible)
        mSelectionCursorGuide->update();
}

void ImageCanvas::confirmPasteSelection()
{
    // This is what the PasteImageCanvasCommand would usually do in redo().
    // Since we do it here and bypass the undo stack, we need to inform the auto swatch model that
    // a change in the image contents occurred, which is why we emit pasteSelectionConfirmed.
    paintImageOntoPortionOfImage(currentLayerIndex(), mSelectionAreaBeforeFirstModification, mSelectionContents);
    emit pasteSelectionConfirmed();
    clearSelection();
}

void ImageCanvas::setSelectionFromPaste(bool isSelectionFromPaste)
{
    mIsSelectionFromPaste = isSelectionFromPaste;
}

void ImageCanvas::panWithSelectionIfAtEdge(ImageCanvas::SelectionPanReason reason)
{
    // Only pan if there's a selection being moved or created.
    // Also, don't pan from mouse events, as the timer takes care of that
    // (prevents jumpiness when moving the mouse)
    if ((!mHasSelection && !mMovingSelection) || (mSelectionEdgePanTimer.isActive() && reason == SelectionPanMouseMovementReason))
        return;

    bool panned = false;
    static const int boostFactor = 6;
    // Scale the velocity by the zoom level to ensure that it's not too slow when zoomed in.
    static const int maxVelocity = boostFactor * mCurrentPane->zoomLevel();
    // The amount by which we should pan based on the distance by which the
    // mouse went over the edge. This is limited to max velocity, but no scaling
    // has been applied yet.
    QPoint baseOffsetChange;

    // Check the left edge.
    if (mCursorPos.x() < 0) {
        baseOffsetChange.rx() += qMin(qAbs(mCursorPos.x()), maxVelocity);
        panned = true;
    } else {
        // Check the right edge.
        const int distancePastRight = mCursorPos.x() - width();
        if (distancePastRight > 0) {
            baseOffsetChange.rx() += qMax(-distancePastRight, -maxVelocity);
            panned = true;
        }
    }

    // Check the top edge.
    if (mCursorPos.y() < 0) {
        baseOffsetChange.ry() += qMin(qAbs(mCursorPos.y()), maxVelocity);
        panned = true;
    } else {
        // Check the bottom edge.
        const int distancePastBottom = mCursorPos.y() - height();
        if (distancePastBottom > 0) {
            baseOffsetChange.ry() += qMax(-distancePastBottom, -maxVelocity);
            panned = true;
        }
    }

    if (panned) {
        // Scale the velocity based on a certain curve, rather than just doing it linearly.
        // With the right curve, this should make it easier to precisely pan with a slower velocity
        // most of the time, but also allow fast panning if the mouse is far enough past the edge.
        const int xOffsetChangeSign = baseOffsetChange.x() > 0 ? 1 : -1;
        const int yOffsetChangeSign = baseOffsetChange.y() > 0 ? 1 : -1;

        // Use qAbs() to ensure that the progress values we pass are between 0.0 and 1.0.
        const QEasingCurve curve(QEasingCurve::InCirc);
        qreal scaledXOffsetChange = curve.valueForProgress(qAbs(baseOffsetChange.x()) / qreal(maxVelocity));
        qreal scaledYOffsetChange = curve.valueForProgress(qAbs(baseOffsetChange.y()) / qreal(maxVelocity));

        // Althought InCirc works well, there is still a certain range of values that it produces
        // that will result in no panning, even though the mouse is past the edge.
        // Work around this by increasing the lower bound. We do it here instead of at the end
        // so that we don't have to mess around with getting the right sign (positive vs negative).
        if (!qFuzzyIsNull(scaledXOffsetChange))
            scaledXOffsetChange = qMin(scaledXOffsetChange + 0.15, 1.0);
        if (!qFuzzyIsNull(scaledYOffsetChange))
            scaledYOffsetChange = qMin(scaledYOffsetChange + 0.15, 1.0);

        const QPoint finalOffsetChange(
             (scaledXOffsetChange * xOffsetChangeSign) * maxVelocity,
             (scaledYOffsetChange * yOffsetChangeSign) * maxVelocity);
        mCurrentPane->setIntegerOffset(mCurrentPane->integerOffset() + finalOffsetChange);

        // Ensure that the panning still occurs when the mouse is at the edge but isn't moving.
        if (!mSelectionEdgePanTimer.isActive()) {
            static const int pansPerSecond = 25;
            static const int panInterval = 1000 / pansPerSecond;
            mSelectionEdgePanTimer.start(panInterval, this);
        }

        // The pane offset changing causes the cursor scene position to change, which
        // in turn affects the selection area.
        updateCursorPos(mCursorPos);
        updateOrMoveSelectionArea();

        requestContentPaint();
    } else {
        // If the mouse isn't over the edge, stop the timer.
        mSelectionEdgePanTimer.stop();
    }
}

void ImageCanvas::setLastSelectionModification(ImageCanvas::SelectionModification selectionModification)
{
    qCDebug(lcImageCanvasSelection) << "setting mLastSelectionModification to" << selectionModification;
    mLastSelectionModification = selectionModification;
    if (mLastSelectionModification == SelectionMove
            || mLastSelectionModification == SelectionFlip
            || mLastSelectionModification == SelectionRotate
            || mLastSelectionModification == SelectionHsl) {
        setHasModifiedSelection(true);
    } else if (mLastSelectionModification == NoSelectionModification) {
        setHasModifiedSelection(false);
    }
    // If it's paste, it should stay false.
    // TODO: verify that pasting multiple times in succession works as expected
}

void ImageCanvas::setHasModifiedSelection(bool hasModifiedSelection)
{
    if (hasModifiedSelection == mHasModifiedSelection)
        return;

    mHasModifiedSelection = hasModifiedSelection;
    emit hasModifiedSelectionChanged();
}

void ImageCanvas::reset()
{
    mFirstPane.reset();
    mSecondPane.reset();
    setCurrentPane(nullptr);
    mSplitter.setPosition(0.5);
    mSplitter.setPressed(false);
    mSplitter.setHovered(false);

    mPressedRuler = nullptr;
    mGuidePositionBeforePress = 0;
    mPressedGuideIndex = -1;

    setCursorPos(QPoint());
    mCursorPaneX = 0;
    mCursorPaneY = 0;
    mCursorScenePos = QPointF();
    mContainsMouse = false;
    mMouseButtonPressed = Qt::NoButton;
    mLastMouseButtonPressed = Qt::NoButton;
    mPressPosition = QPoint(0, 0);
    mPressScenePosition = QPoint(0, 0);

    mIsTabletEvent = false;
    mTabletPressure = 0.0;
    setPenForegroundColour(Qt::black);
    setPenBackgroundColour(Qt::white);
    updateBrush();

    mTexturedFillParameters.reset();

    mCropArea = QRect();

    clearSelection();
    setAltPressed(false);
    mToolBeforeAltPressed = PenTool;
    mSpacePressed = false;
    mHasBlankCursor = false;

    // Things that we don't want to set, as they
    // don't really need to be reset each time:
    // - tool
    // - toolSize

    requestContentPaint();
}

void ImageCanvas::centreView()
{
    CanvasPane *pane = currentPane();
    if (!pane)
        return;

    const QPointF centre(qreal(mProject->heightInPixels()) / 2.0, qreal(mProject->heightInPixels()) / 2.0);
    pane->setOffset(-centre);
    requestContentPaint();
}

void ImageCanvas::zoomIn()
{
    CanvasPane *pane = currentPane();
    if (!pane)
        return;

    pane->setZoomLevel(pane->integerZoomLevel() + 1);
}

void ImageCanvas::zoomOut()
{
    CanvasPane *pane = currentPane();
    if (!pane)
        return;

    pane->setZoomLevel(pane->integerZoomLevel() - 1);
}

void ImageCanvas::flipSelection(Qt::Orientation orientation)
{
    if (!mHasSelection)
        return;

    // Just like mspaint, flipping a pasted selection has no effect on the undo stack -
    // undoing will simply remove the pasted selection and its contents.
    if (!mIsSelectionFromPaste) {
        // TODO: need to prevent the selection from being cleared here
        mProject->beginMacro(QLatin1String("FlipSelection"));
        mProject->addChange(new FlipImageCanvasSelectionCommand(this, mSelectionArea, orientation));
        mProject->endMacro();
    } else {
        mSelectionContents = mSelectionContents.mirrored(orientation == Qt::Horizontal, orientation == Qt::Vertical);
        updateSelectionPreviewImage(SelectionFlip);
        requestContentPaint();
    }
}

void ImageCanvas::rotateSelection(int angle)
{
    qCDebug(lcImageCanvasSelection) << "rotating selection area" << mSelectionArea << angle << "by degrees";

    if (!mHasSelection)
        return;

    bool isFirstModification = false;
    if (mSelectionAreaBeforeFirstModification.isNull()) {
        mSelectionAreaBeforeFirstModification = mSelectionArea;
        isFirstModification = true;
    }

    QRect rotatedArea;
    // Only use the project's image the first time; for every consecutive rotation,
    // do the rotation on a fully transparent image and then paint it onto that.
    // This avoids contents that are under the selection as its rotated being picked up
    // and becoming part of the selection (rotateSelectionTransparentBackground() tests this).
    if (isFirstModification) {
        const QImage image = *imageForLayerAt(currentLayerIndex());
        mSelectionContents = Utils::rotateAreaWithinImage(image, mSelectionArea, angle, rotatedArea);
    } else {
        QImage image(mProject->size(), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter painter(&image);
        painter.drawImage(mSelectionArea, mSelectionContents);
        mSelectionContents = Utils::rotateAreaWithinImage(image, mSelectionArea, angle, rotatedArea);
    }

    Q_ASSERT(mHasSelection);
    setSelectionArea(rotatedArea);
    mLastValidSelectionArea = mSelectionArea;

    setLastSelectionModification(SelectionRotate);

    updateSelectionPreviewImage(SelectionRotate);
    requestContentPaint();
}

// How we do HSL modifications:
//
// We apply the HSL modification directly on the selection contents,
// just as we do for e.g. rotation. This is easier than having a separate,
// intermediate preview pane.
//
// Why these begin/end functions exist:
//
// If we only had modifySelectionHsl(), each cumulative HSL modification
// would be applied to the result of the last. The visible result of this is
// that it's not possible to move the saturation slider back and forth
// without losing detail in the image, for example.
// By keeping track of when the HSL modifications begin and end, we can
// ensure that all modifications are done on the original image.
//
// Also, no other modifications to the canvas are possible while
// the Hue/Saturation dialog is open, so we don't have to worry about conflict there.
void ImageCanvas::beginModifyingSelectionHsl()
{
    if (!mHasSelection) {
        qWarning() << "Can't modify HSL without a selection";
        return;
    }

    if (!mSelectionContentsBeforeImageAdjustment.isNull()) {
        qWarning() << "Already modifying selection's HSL";
        return;
    }

    qCDebug(lcImageCanvasSelection) << "beginning modification of selection's HSL";

    if (mSelectionAreaBeforeFirstModification.isNull()) {
        mSelectionAreaBeforeFirstModification = mSelectionArea;
        mSelectionContents = currentProjectImage()->copy(mSelectionAreaBeforeFirstModification);
    }

    // Store the original selection contents before we start modifying it
    // so that each modification is done on a copy of the contents instead of the original contents.
    mSelectionContentsBeforeImageAdjustment = mSelectionContents;
    mLastSelectionModificationBeforeImageAdjustment = mLastSelectionModification;
    emit adjustingImageChanged();
}

void ImageCanvas::modifySelectionHsl(qreal hue, qreal saturation, qreal lightness)
{
    if (!isAdjustingImage()) {
        qWarning() << "Not adjusting an image; can't modify selection's HSL";
        return;
    }

    qCDebug(lcImageCanvasSelection).nospace() << "modifying HSL of selection"
        << mSelectionArea << " with h=" << hue << " s=" << saturation << " l=" << lightness;

    // Copy the original so we don't just modify the result of the last adjustment (if any).
    mSelectionContents = mSelectionContentsBeforeImageAdjustment;

    Utils::modifyHsl(mSelectionContents, hue, saturation, lightness);

    // Set this so that the check in shouldDrawSelectionPreviewImage() evaluates to true.
    setLastSelectionModification(SelectionHsl);

    updateSelectionPreviewImage(SelectionHsl);
    requestContentPaint();
}

void ImageCanvas::endModifyingSelectionHsl(AdjustmentAction adjustmentAction)
{
    qCDebug(lcImageCanvasSelection) << "ended modification of selection's HSL";

    if (adjustmentAction == RollbackAdjustment) {
        mSelectionContents = mSelectionContentsBeforeImageAdjustment;
        setLastSelectionModification(mLastSelectionModificationBeforeImageAdjustment);
        updateSelectionPreviewImage(SelectionHsl);
        requestContentPaint();
    } else {
        // Commit the adjustments. We don't need to request a repaint
        // since nothing has changed since the last one.
        setLastSelectionModification(SelectionHsl);
    }

    mSelectionContentsBeforeImageAdjustment = QImage();
    emit adjustingImageChanged();
}

void ImageCanvas::copySelection()
{
    if (!mHasSelection)
        return;

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(currentProjectImage()->copy(mSelectionArea));
}

void ImageCanvas::paste()
{
    qCDebug(lcImageCanvasSelection) << "pasting selection from clipboard";

    QRect pastedArea = mSelectionArea;
    const bool fromExternalSource = !mHasSelection;

    clearOrConfirmSelection();

    QClipboard *clipboard = QGuiApplication::clipboard();
    QImage clipboardImage = clipboard->image();
    if (clipboardImage.isNull()) {
        qCDebug(lcImageCanvasSelection) << "Clipboard content is not an image; can't paste";
        return;
    }

    setTool(SelectionTool);

    const QSize adjustedSize(qMin(clipboardImage.width(), mProject->widthInPixels()),
        qMin(clipboardImage.height(), mProject->heightInPixels()));
    if (fromExternalSource) {
        // If the paste was from an external source, or there was no
        // selection prior to pasting, we just paste it at 0, 0.
        pastedArea = QRect(0, 0, adjustedSize.width(), adjustedSize.height());
        // TODO: #16 - pastes too far to the top left for some reason
//        if (mCurrentPane->offset().x() < 0)
//            pastedArea.moveLeft(qAbs(mCurrentPane->offset().x()) / mCurrentPane->zoomLevel());
//        if (mCurrentPane->offset().y() < 0)
//            pastedArea.moveTop(qAbs(mCurrentPane->offset().y()) / mCurrentPane->zoomLevel());
    }
    // Crop the clipboard image if it's larger than the canvas,
    if (adjustedSize != clipboardImage.size())
        clipboardImage = clipboardImage.copy(pastedArea);

    mProject->beginMacro(QLatin1String("PasteCommand"));
    mProject->addChange(new PasteImageCanvasCommand(this, currentLayerIndex(), clipboardImage, pastedArea.topLeft()));
    mProject->endMacro();

    // Setting a selection area is only done when a paste is first created,
    // not when it's redone, so we do it here instead of in the command.
    setSelectionFromPaste(true);
    qCDebug(lcImageCanvasSelection) << "setting selection contents to clipboard image with area" << pastedArea;
    mSelectionContents = clipboardImage;

    setSelectionArea(pastedArea);

    // This part is also important, as it ensures that beginSelectionMove()
    // doesn't overwrite the paste contents.
    mSelectionAreaBeforeFirstModification = mSelectionArea;
    setLastSelectionModification(SelectionPaste);

    // moveSelectionArea() does this for us when we're moving, but for the initial
    // paste, we must do it ourselves.
    updateSelectionPreviewImage(SelectionPaste);

    requestContentPaint();
}

void ImageCanvas::deleteSelectionOrContents()
{
    mProject->beginMacro(QLatin1String("DeleteSelection"));
    const QRect deletionArea = mHasSelection ? mSelectionArea : mProject->bounds();
    mProject->addChange(new DeleteImageCanvasSelectionCommand(this, currentLayerIndex(), deletionArea));
    mProject->endMacro();
    clearSelection();
}

void ImageCanvas::selectAll()
{
    if (mTool == SelectionTool) {
        clearOrConfirmSelection();
    } else {
        setTool(SelectionTool);
    }

    setSelectionArea(QRect(0, 0, mProject->widthInPixels(), mProject->heightInPixels()));
}

void ImageCanvas::brushFromSelection()
{
    if (!mHasSelection)
        return;

    mBrush = Brush(currentProjectImage()->copy(mSelectionArea));
    setBrushType(Brush::ImageType);
    setUpperToolSize(qMax(mBrush.size.width(), mBrush.size.height()));
}

void ImageCanvas::cycleFillTools()
{
    setTool(mLastFillToolUsed == FillTool ? TexturedFillTool : FillTool);
}

QImage ImageCanvas::fillPixels() const
{
    const QPoint scenePos = cursorScenePixel();
    if (!isWithinImage(scenePos))
        return QImage();

    const QColor previousColour = currentProjectImage()->pixelColor(scenePos);
    // Don't do anything if the colours are the same.
    if (previousColour == penColour())
        return QImage();

    return imagePixelFloodFill2(currentProjectImage(), scenePos, previousColour, penColour());
}

QImage ImageCanvas::greedyFillPixels() const
{
    const QPoint scenePos = cursorScenePixel();
    if (!isWithinImage(scenePos))
        return QImage();

    const QColor previousColour = currentProjectImage()->pixelColor(scenePos);
    if (previousColour == penColour())
        return QImage();

    return imageGreedyPixelFill2(currentProjectImage(), scenePos, previousColour, penColour());
}

QImage ImageCanvas::texturedFillPixels() const
{
    const QPoint scenePos = cursorScenePixel();
    if (!isWithinImage(scenePos))
        return QImage();

    const QColor previousColour = currentProjectImage()->pixelColor(scenePos);
    if (previousColour == penColour())
        return QImage();

    return texturedFill(currentProjectImage(), scenePos, previousColour, penColour(), mTexturedFillParameters);
}

QImage ImageCanvas::greedyTexturedFillPixels() const
{
    const QPoint scenePos = cursorScenePixel();
    if (!isWithinImage(scenePos))
        return QImage();

    const QColor previousColour = currentProjectImage()->pixelColor(scenePos);
    if (previousColour == penColour())
        return QImage();

    return greedyTexturedFill(currentProjectImage(), scenePos, previousColour, penColour(), mTexturedFillParameters);
}

QPainter::CompositionMode ImageCanvas::qPainterBlendMode() const
{
    static const QMap<ToolBlendMode, QPainter::CompositionMode> mapping{
        { BlendToolBlendMode, QPainter::CompositionMode_SourceOver },
        { ReplaceToolBlendMode, QPainter::CompositionMode_Source },
    };
    return mapping[mToolBlendMode];
}

void ImageCanvas::applyCurrentTool(QUndoStack *const alternateStack)
{
    if (areToolsForbidden())
        return;

    // For tool preview use alternate undo stack if given
    QUndoStack *const stack = alternateStack ? alternateStack : mProject->undoStack();
    const QUndoCommand *const continueCommand = (mToolContinue && stack->index() > 0) ? stack->command(stack->index() - 1) : nullptr;
    QUndoCommand *command = nullptr;

    const int brushSize = qMax(brush().size.width(), brush().size.height());
    const qreal scaleMax = qreal(mUpperToolSize) / qreal(brushSize);
    const qreal scaleMin = mToolSizeUsePressure ? qreal(mLowerToolSize) / qreal(brushSize) : scaleMax;

    switch (mTool) {
    case PenTool: {
        command = new ApplyPixelLineCommand(this, mProject->currentLayerIndex(), mNewStroke, scaleMin, scaleMax, brush(), penColour(), qPainterBlendMode(), continueCommand);
        command->setText(QLatin1String("PixelLineTool"));
        break;
    }
    case EyeDropperTool: {
        const QColor pickedColour = pickColour(mNewStroke.last().pos);
        if (pickedColour.isValid())
            setPenColour(pickedColour);
        break;
    }
    case EraserTool: {
        command = new ApplyPixelLineCommand(this, mProject->currentLayerIndex(), mNewStroke, scaleMin, scaleMax, brush(), penColour(), QPainter::CompositionMode_Clear, continueCommand);
        command->setText(QLatin1String("PixelEraserTool"));
        break;
    }
    case FillTool: {
        if (!mShiftPressed) {
            const QImage filledImage = fillPixels();
            if (filledImage.isNull())
                return;

            command = new ApplyPixelFillCommand(this, mProject->currentLayerIndex(), *currentProjectImage(), filledImage);
            command->setText(QLatin1String("PixelFillTool"));
        } else {
            const QImage filledImage = greedyFillPixels();
            if (filledImage.isNull())
                return;

            command = new ApplyGreedyPixelFillCommand(this, mProject->currentLayerIndex(), *currentProjectImage(), filledImage);
            command->setText(QLatin1String("GreedyPixelFillTool"));
        }
        break;
    }
    case TexturedFillTool: {
        if (!mShiftPressed) {
            const QImage filledImage = texturedFillPixels();
            if (filledImage.isNull())
                return;

            command = new ApplyPixelFillCommand(this, mProject->currentLayerIndex(), *currentProjectImage(), filledImage);
            command->setText(QLatin1String("PixelTexturedFillTool"));
        } else {
            const QImage filledImage = greedyFillPixels();
            if (filledImage.isNull())
                return;

            command = new ApplyGreedyPixelFillCommand(this, mProject->currentLayerIndex(), *currentProjectImage(), filledImage);
            command->setText(QLatin1String("GreedyPixelTexturedFillTool"));
        }
        break;
    }
    default:
        break;
    }

    if (command) {
//        mProject->addChange(command);
        stack->push(command);
    }
}

// This function actually operates on the image.
void ImageCanvas::applyPixelPenTool(int layerIndex, const QPoint &scenePos, const QColor &colour)
{
    imageForLayerAt(layerIndex)->setPixelColor(scenePos, colour);
    requestContentPaint();
}

void ImageCanvas::paintImageOntoPortionOfImage(int layerIndex, const QRect &portion, const QImage &replacementImage)
{
    QImage *image = imageForLayerAt(layerIndex);
    *image = Utils::paintImageOntoPortionOfImage(*image, portion, replacementImage);
    requestContentPaint();
}

void ImageCanvas::replacePortionOfImage(int layerIndex, const QRect &portion, const QImage &replacementImage)
{
    QImage *image = imageForLayerAt(layerIndex);
    *image = Utils::replacePortionOfImage(*image, portion, replacementImage);
    requestContentPaint();
}

void ImageCanvas::erasePortionOfImage(int layerIndex, const QRect &portion)
{
    QImage *image = imageForLayerAt(layerIndex);
    *image = Utils::erasePortionOfImage(*image, portion);
    requestContentPaint();
}

void ImageCanvas::replaceImage(int layerIndex, const QImage &replacementImage)
{
    // TODO: could ImageCanvas just be a LayeredImageCanvas with one layer?
    QImage *image = imageForLayerAt(layerIndex);
    *image = replacementImage;
    requestContentPaint();
}

void ImageCanvas::doFlipSelection(int layerIndex, const QRect &area, Qt::Orientation orientation)
{
    const QImage flippedImagePortion = currentProjectImage()->copy(area)
        .mirrored(orientation == Qt::Horizontal, orientation == Qt::Vertical);
    erasePortionOfImage(layerIndex, area);
    paintImageOntoPortionOfImage(layerIndex, area, flippedImagePortion);
}

QRect ImageCanvas::doRotateSelection(int layerIndex, const QRect &area, int angle)
{
    QImage *image = imageForLayerAt(layerIndex);
    QRect rotatedArea;
    *image = Utils::rotateAreaWithinImage(*image, area, angle, rotatedArea);
    // Only update the selection area when the commands are being created for the first time,
    // not when they're being undone and redone.
    if (mHasSelection)
        setSelectionArea(rotatedArea);
    requestContentPaint();
    return area.united(rotatedArea);
}

void ImageCanvas::updateBrush()
{
    // Create new brush if not image brush
    if (mBrushType != Brush::ImageType) {
        mBrush = Brush(mBrushType, {mUpperToolSize, mUpperToolSize});
    }
    emit brushRectChanged();
}

const Brush &ImageCanvas::brush()
{
    return mBrush;
}

qreal ImageCanvas::pressure() const
{
    // Use tablet pressure if tablet button pressed, otherwise must be mouse so use full pressure
    return mIsTabletEvent ? mTabletPressure : 1.0;
}

void ImageCanvas::updateCursorPos(const QPoint &eventPos)
{
    CanvasPane *const pane = hoveredPane(eventPos);

    setCursorPos(eventPos);

    // Don't change current panes if panning, as the mouse position should
    // be allowed to go outside of the original pane.
    // If we're creating or moving a selection, don't let the current pane be changed.
    if (!mSpacePressed && !(mHasSelection && mMouseButtonPressed == Qt::LeftButton) && mMouseButtonPressed != Qt::NoButton) {
        if (pane) setCurrentPane(pane);
    }

    if (!mProject->hasLoaded() || !pane) {
        setCursorScenePos(QPointF());
        // We could do this once at the beginning of the function, but we
        // try to avoid unnecessary property changes.
        setCursorPixelColour(QColor(Qt::black));
        return;
    }

    const QPoint oldCursorSceneInteger = cursorScenePixel();

    // We need the position as floating point numbers so that pen sizes > 1 work properly.
    setCursorScenePos(pane->transform().inverted().map(QPointF(eventPos)));

    setCursorPixelColour(pickColour(cursorScenePixel()));

    if (cursorScenePixel() != oldCursorSceneInteger && mSelectionCursorGuide->isVisible())
        mSelectionCursorGuide->update();
}

void ImageCanvas::onLoadedChanged()
{
    updateWindowCursorShape();
}

void ImageCanvas::requestContentPaint()
{
    // It's nice to be able to debug where a paint request comes from;
    // that's the only reason that these functions are slots and the signal isn't
    // just emitted immediately instead.
    emit contentPaintRequested(-1);
}

void ImageCanvas::requestPaneContentPaint(int paneIndex)
{
    emit contentPaintRequested(paneIndex);
}

void ImageCanvas::updateWindowCursorShape()
{
    if (!mProject)
        return;

    const bool overRuler = (rulerAtCursorPos() != nullptr);
    bool overGuide = false;
    if (guidesVisible() && !guidesLocked() && !overRuler) {
        overGuide = guideIndexAtCursorPos() != -1;
    }

    // Hide the window's cursor when we're in the spotlight; otherwise, use the non-custom arrow cursor.
    const bool nothingOverUs = mProject->hasLoaded() && hasActiveFocus() /*&& !mModalPopupsOpen*/ && mContainsMouse;
    const bool splitterHovered = mSplitter.isEnabled() && mSplitter.isHovered();
    const bool overSelection = cursorOverSelection();
    const bool toolsForbidden = areToolsForbidden();
    setHasBlankCursor(nothingOverUs && !isPanning() && !splitterHovered && !overSelection && !overRuler && !overGuide && !toolsForbidden);

    Qt::CursorShape cursorShape = Qt::BlankCursor;
    if (!mHasBlankCursor) {
        if (isPanning()) {
            // If panning while space is pressed, the left mouse button is used, otherwise it's the middle mouse button.
            cursorShape = (mMouseButtonPressed == Qt::LeftButton || mMouseButtonPressed == Qt::MiddleButton) ? Qt::ClosedHandCursor : Qt::OpenHandCursor;
        } else if (overGuide) {
            cursorShape = mPressedGuideIndex != -1 ? Qt::ClosedHandCursor : Qt::OpenHandCursor;
        } else if (overSelection) {
            cursorShape = Qt::SizeAllCursor;
        } else if (splitterHovered) {
            cursorShape = Qt::SplitHCursor;
        } else if (toolsForbidden && nothingOverUs) {
            cursorShape = Qt::ForbiddenCursor;
        } else {
            cursorShape = Qt::ArrowCursor;
        }
    }

    if (lcImageCanvasCursorShape().isDebugEnabled()) {
        qCDebug(lcImageCanvasCursorShape) << "Updating window cursor shape for" << objectName() << "..."
            << "\n... mProject->hasLoaded()" << mProject->hasLoaded()
            << "\n........ hasActiveFocus()" << hasActiveFocus()
            << "\n.......... mContainsMouse" << mContainsMouse
            << "\n............. isPanning()" << isPanning()
            << "\n... mSplitter.isHovered()" << mSplitter.isHovered()
            << "\n..... areToolsForbidden()" << toolsForbidden
            << "\n......... mHasBlankCursor" << mHasBlankCursor
            << "\n............ cursor shape" << Utils::enumToString(cursorShape);
    }

    if (window())
        window()->setCursor(QCursor(cursorShape));
}

void ImageCanvas::onZoomLevelChanged()
{
    for (int i = 0; i < mPanes.size(); ++i) {
        mHorizontalRulers[i]->setZoomLevel(mPanes[i]->zoomLevel());
        mVerticalRulers[i]->setZoomLevel(mPanes[i]->zoomLevel());
    }

    requestContentPaint();

    if (mGuidesVisible)
        mGuidesItem->update();

    mSelectionItem->update();
}

void ImageCanvas::onPaneOffsetChanged()
{
    for (int i = 0; i < mPanes.size(); ++i) {
        const QPointF offset = mPanes[i]->transform().map(-mPanes[i]->geometry().topLeft());
        mHorizontalRulers[i]->setFrom(qFloor(offset.x()));
        mVerticalRulers[i]->setFrom(qFloor(offset.y()));
    }

    if (mGuidesVisible)
        mGuidesItem->update();

    mSelectionItem->update();
}

void ImageCanvas::onPaneSizeChanged()
{
    resizeRulers();

    if (mGuidesVisible)
        mGuidesItem->update();

    mSelectionItem->update();
}

void ImageCanvas::error(const QString &message)
{
//    qWarning() << Q_FUNC_INFO << message;
    emit errorOccurred(message);
}

Qt::MouseButton ImageCanvas::pressedMouseButton() const
{
    // For some tools, like the line tool, the mouse button won't be pressed at times,
    // so we take the last mouse button that was pressed.
    return mMouseButtonPressed == Qt::NoButton ? mLastMouseButtonPressed : mMouseButtonPressed;
}

QColor ImageCanvas::penColour() const
{
    return pressedMouseButton() == Qt::LeftButton ? mPenForegroundColour : mPenBackgroundColour;
}

void ImageCanvas::setPenColour(const QColor &colour)
{
    if (pressedMouseButton() == Qt::LeftButton)
        setPenForegroundColour(colour);
    else
        setPenBackgroundColour(colour);
}

void ImageCanvas::setHasBlankCursor(bool hasCustomCursor)
{
    if (hasCustomCursor == mHasBlankCursor)
        return;

    mHasBlankCursor = hasCustomCursor;
    emit hasBlankCursorChanged();
}

void ImageCanvas::setCurrentPane(CanvasPane *pane)
{
    if (pane == mCurrentPane)
        return;

    mCurrentPane = pane;
    mCurrentPaneIndex = (pane == &mSecondPane ? 1 : 0);
    emit currentPaneChanged();
}

CanvasPane *ImageCanvas::hoveredPane(const QPoint &pos)
{
    for (int i = 0; i < mPanes.size(); ++i) {
        if (mPanes[i]->geometry().contains(pos)) return mPanes[i];
    }
    return nullptr;
}

void ImageCanvas::restoreToolBeforeAltPressed()
{
    setAltPressed(false);
    setTool(mToolBeforeAltPressed);
}

bool ImageCanvas::areToolsForbidden() const
{
    return false;
}

bool ImageCanvas::eventFilter(QObject *watched, QEvent *event)
{
    // When cursor on canvas call tablet event handler with duplicate event
    static const QSet<QEvent::Type> tabletEvents {
        QEvent::TabletMove, QEvent::TabletPress, QEvent::TabletRelease,
        QEvent::TabletEnterProximity, QEvent::TabletLeaveProximity,
        QEvent::TabletTrackingChange
    };
    if (mContainsMouse && tabletEvents.contains(event->type())) {
        const QTabletEvent *const other = static_cast<QTabletEvent *>(event);
        QTabletEvent own(other->type(), mapFromGlobal(other->globalPosF()), other->globalPosF(),
                                    other->device(), other->pointerType(), other->pressure(),
                                    other->xTilt(), other->yTilt(), other->tangentialPressure(),
                                    other->rotation(), other->z(), other->modifiers(), other->uniqueId(),
                                    other->button(), other->buttons());
        tabletEvent(&own);
    }

    return QQuickItem::eventFilter(watched, event);
}

bool ImageCanvas::event(QEvent *event)
{
    // This allows us to handle the two-finger pinch zoom gesture on macOS trackpads, for example.
    if (mGesturesEnabled && event->type() == QEvent::NativeGesture) {
        QNativeGestureEvent *gestureEvent = static_cast<QNativeGestureEvent*>(event);
        if (gestureEvent->gestureType() == Qt::ZoomNativeGesture
                || gestureEvent->gestureType() == Qt::BeginNativeGesture
                || gestureEvent->gestureType() == Qt::EndNativeGesture) {
            if (!qFuzzyIsNull(gestureEvent->value())) {
                // Zooming is a bit slow without this.
                static const qreal minZoomFactor = 3.0;
                static const qreal maxZoomFactor = 10.0;

                // TODO
                // Apply an easing curve to make zooming faster the more zoomed-in we are.
                const qreal percentageOfMaxZoomLevel = mCurrentPane->zoomLevel() / mCurrentPane->maxZoomLevel();
                const QEasingCurve zoomCurve(QEasingCurve::OutQuart);
                const qreal scaledZoomFactor = qMax(minZoomFactor, zoomCurve.valueForProgress(percentageOfMaxZoomLevel) * maxZoomFactor);

                const qreal zoomAmount = gestureEvent->value() * scaledZoomFactor;
                const qreal newZoom = mCurrentPane->zoomLevel() + zoomAmount;
                applyZoom(newZoom, gestureEvent->pos());
            }
            return true;
        }
    }

    return QQuickItem::event(event);
}

void ImageCanvas::applyZoom(qreal zoom, const QPoint &origin)
{
    // Get point before transform
    const QPointF pointBefore = mCurrentPane->transform().inverted().map(QPointF(origin));
    // Apply transform
    mCurrentPane->setZoomLevel(zoom);
    // Get point after transform
    const QPointF pointAfter = mCurrentPane->transform().inverted().map(QPointF(origin));
    // Offset by difference between points
    const QPointF delta = pointAfter - pointBefore;
    mCurrentPane->setOffset(mCurrentPane->offset() + delta);
}

void ImageCanvas::wheelEvent(QWheelEvent *event)
{
    if (!mProject->hasLoaded() || !mScrollZoom) {
        event->ignore();
        return;
    }

    const QPoint pixelDelta = event->pixelDelta();
    const QPoint angleDelta = event->angleDelta();

    if (!mGesturesEnabled) {
        // Wheel events zoom.
        qreal newZoomLevel = 0;
        if (!pixelDelta.isNull()) {
            const qreal zoomAmount = pixelDelta.y() * 0.01;
            newZoomLevel = mCurrentPane->zoomLevel() + zoomAmount;
        } else if (!angleDelta.isNull()) {
            const qreal zoomAmount = 1.0;
            newZoomLevel = mCurrentPane->zoomLevel() + (angleDelta.y() > 0 ? zoomAmount : -zoomAmount);
        }

        if (!qFuzzyIsNull(newZoomLevel))
            applyZoom(newZoomLevel, event->pos());
    } else {
        // Wheel events pan.
        if (pixelDelta.isNull())
            return;

        const QPointF panDistance(pixelDelta.x() * 0.1, pixelDelta.y() * 0.1);
        mCurrentPane->setOffset(mCurrentPane->offset() + panDistance);
        requestPaneContentPaint(mCurrentPaneIndex);
    }
}

void ImageCanvas::mousePressEvent(QMouseEvent *event)
{
    QQuickItem::mousePressEvent(event);

    // Is it possible to get a press without a hover enter? If so, we need this line.
    updateCursorPos(event->pos());

    if (!mProject->hasLoaded()) {
        return;
    }

    event->accept();

    mMouseButtonPressed = event->button();
    mLastMouseButtonPressed = mMouseButtonPressed;
    mPressPosition = event->pos();
    mPressScenePosition = mCursorScenePos;
    setContainsMouse(true);

    if (!isPanning()) {
        if (mSplitter.isEnabled() && mouseOverSplitterHandle(event->pos())) {
            mSplitter.setPressed(true);
            return;
        }

        if (rulersVisible() && guidesVisible()) {
            updatePressedRuler();
            if (mPressedRuler)
                return;

            if (!guidesLocked()) {
                updatePressedGuide();
                if (mPressedGuideIndex != -1) {
                    updateWindowCursorShape();
                    return;
                }
            }
        }

        mNewStroke = {StrokePoint{mCursorScenePos, pressure()}};
        if (mShiftPressed && !mOldStroke.isEmpty()) {
            mToolContinue = true;
            mNewStroke.prepend(mOldStroke.last());
        }
        else {
            mToolContinue = false;
        }

        if (mTool != SelectionTool) {
            applyCurrentTool();
            return;
        }

        if (!cursorOverSelection()) {
            mPotentiallySelecting = true;
            updateSelectionArea();
        } else {
            // The user has just clicked the selection. We don't actually
            // move anything on press events; we wait until mouseMoveEvent()
            // and then start moving the selection and its contents.
            beginSelectionMove();
        }
    } else {
        updateWindowCursorShape();
    }

    oldMousePos = event->pos();
}

void ImageCanvas::mouseMoveEvent(QMouseEvent *event)
{
    QQuickItem::mouseMoveEvent(event);

    updateCursorPos(event->pos());

    if (!mProject->hasLoaded())
        return;

    setContainsMouse(true);

    if (mMouseButtonPressed) {
        if (mSplitter.isEnabled() && mSplitter.isPressed()) {
            mSplitter.setPosition(mCursorPos.x() / width());
        } else if (mPressedRuler) {
            requestContentPaint();
            // Ensure that the guide being created is drawn.
            mGuidesItem->update();
        } else if (mPressedGuideIndex != -1) {
            mGuidesItem->update();
        } else {
            if (!isPanning()) {
                if (mTool != SelectionTool) {
                    mToolContinue = true;
                    mNewStroke = {StrokePoint{mCursorScenePos, pressure()}};
                    if (!mOldStroke.isEmpty()) {
                        mNewStroke.prepend(mOldStroke.last());
                    }
                    mPressScenePosition = mCursorScenePos;
                    applyCurrentTool();
                } else {
                    panWithSelectionIfAtEdge(SelectionPanMouseMovementReason);

                    updateOrMoveSelectionArea();
                }
            } else {
                // Panning
                const QPointF delta = mCurrentPane->transform().inverted().map(QPointF(event->pos())) - mCurrentPane->transform().inverted().map(QPointF(oldMousePos));
                mCurrentPane->setOffset(mCurrentPane->offset() + delta);
                requestPaneContentPaint(mCurrentPaneIndex);
            }
        }
    }

    oldMousePos = event->pos();
}

void ImageCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    QQuickItem::mouseReleaseEvent(event);

    updateCursorPos(event->pos());

    if (!mProject->hasLoaded())
        return;

    mMouseButtonPressed = Qt::NoButton;

    // Make sure we do this after the mouse button has been cleared
    // (as setSelectionArea() relies on this to accurately set mHasSelection),
    // but before the press position has been cleared (as
    // updateSelectionArea() needs that information).
    if (mTool == SelectionTool && !mPressedRuler) {
        if (!mMovingSelection) {
            if (mPotentiallySelecting) {
                // The mouse press that caused mPotentiallySelecting to be set to
                // true has now been accompanied by a release. If there was mouse movement
                // in between these two events, then we now have a selection.
                // Temporary note: this check used to be "if (!mHasMovedSelection) {"
                if (!mHasModifiedSelection) {
                    // We haven't moved the selection, meaning that there have been no
                    // changes to the canvas during this "event cycle".
                    if (mHasSelection) {
                        // Finish off the selection by including the last release event.
                        updateSelectionArea();
                        if (!mSelectionArea.size().isEmpty()) {
                            // As this selection is valid, store it. If this branch (or the one above it)
                            // doesn't get run, then the user was simply clicking outside of the selection
                            // in order to clear it.
                            mLastValidSelectionArea = mSelectionArea;
                        }
                    } else {
                        // There is no selection.
                        if (mIsSelectionFromPaste) {
                            // Pasting an image creates a selection, and clicking outside of that selection
                            // without moving it should apply the paste.
                            confirmPasteSelection();
                        } else {
                            // Since we hadn't done anything to the selection that we might have had
                            // before (that this event cycle is interrupting), releasing the mouse
                            // should cancel it.
                            clearSelection();
                        }
                    }
                } else {
                    // We have moved the selection since creating it, but we're not
                    // currently moving it, which means that the user just clicked outside of it,
                    // which means we clear it.
                    confirmSelectionModification();
                }
            }
        } else {
            // mspaint and photoshop both exhibit the same behaviour here:
            // moving a selection several times and then undoing once will undo all movement.
            // It's for this reason that we don't create a move command here.
            // They handle rotation differently though: mspaint will undo all rotation since
            // the selection was created, whereas photoshop will only undo one rotation at a time.
            moveSelectionArea();
            setMovingSelection(false);
            if (!mSelectionArea.size().isEmpty())
                mLastValidSelectionArea = mSelectionArea;
        }
        mPotentiallySelecting = false;

        mSelectionEdgePanTimer.stop();
    }

    if (mProject->isComposingMacro()) {
        mProject->endMacro();
    }

    Ruler *hoveredRuler = rulerAtCursorPos();
    if (mPressedRuler && !hoveredRuler) {
        // A ruler was pressed but isn't hovered; create a new guide.
        addNewGuide();
    } else if (mPressedGuideIndex != -1) {
        if (hoveredRuler) {
            if (hoveredRuler->orientation() == mProject->guides().at(mPressedGuideIndex).orientation()) {
                // A ruler wasn't pressed but a guide is, and now a ruler is hovered;
                // the user has dragged a guide onto a ruler with the correct orientation, so remove it.
                removeGuide();
            }
        } else {
            moveGuide();
        }

        mPressedGuideIndex = -1;
        updateWindowCursorShape();
    }

    mPressPosition = QPoint(0, 0);
    mPressScenePosition = QPoint(0, 0);
    updateWindowCursorShape();
    mSplitter.setPressed(false);
    mPressedRuler = nullptr;
    mGuidePositionBeforePress = 0;
}

void ImageCanvas::hoverEnterEvent(QHoverEvent *event)
{
    QQuickItem::hoverEnterEvent(event);

    updateCursorPos(event->pos());

    setContainsMouse(true);

    if (!mProject->hasLoaded())
        return;
}

void ImageCanvas::hoverMoveEvent(QHoverEvent *event)
{
    QQuickItem::hoverMoveEvent(event);

    updateCursorPos(event->pos());

    setContainsMouse(true);

    if (!mProject->hasLoaded())
        return;

    mSplitter.setHovered(mouseOverSplitterHandle(event->pos()));

    updateWindowCursorShape();

    mNewStroke = {{mCursorScenePos, 1.0}};
    if (isLineVisible() && !mOldStroke.isEmpty()) mNewStroke.prepend(mOldStroke.last());
    if (isLineVisible() || mProject->settings()->isBrushPreviewVisible()) {
        requestContentPaint();
    }
}

void ImageCanvas::hoverLeaveEvent(QHoverEvent *event)
{
    QQuickItem::hoverLeaveEvent(event);

    setContainsMouse(false);

    if (!mProject->hasLoaded())
        return;
}

void ImageCanvas::keyPressEvent(QKeyEvent *event)
{
    QQuickItem::keyPressEvent(event);

    if (!mProject->hasLoaded())
        return;

    if (mHasSelection && !mMovingSelection && (event->key() >= Qt::Key_Left && event->key() <= Qt::Key_Down)) {
        switch (event->key()) {
        case Qt::Key_Left:
            moveSelectionAreaBy(QPoint(-1, 0));
            return;
        case Qt::Key_Right:
            moveSelectionAreaBy(QPoint(1, 0));
            return;
        case Qt::Key_Up:
            moveSelectionAreaBy(QPoint(0, -1));
            return;
        case Qt::Key_Down:
            moveSelectionAreaBy(QPoint(0, 1));
            return;
        }
    }

    if (event->isAutoRepeat())
        return;

    if (event->key() >= Qt::Key_1 && event->key() <= Qt::Key_5) {
        const Tool activatedTool = static_cast<ImageCanvas::Tool>(PenTool + event->key() - Qt::Key_1);
        if (activatedTool == FillTool && (mTool == FillTool || mTool == TexturedFillTool)) {
            // If the tool was already one of the fill tools, select the next fill tool.
            cycleFillTools();
        } else {
            if (activatedTool != SelectionTool || (activatedTool == SelectionTool && supportsSelectionTool())) {
                // Don't set the selection tool if the canvas doesn't support it.
                setTool(activatedTool);
            }
        }
    } else if (event->key() == Qt::Key_Space) {
        mSpacePressed = true;
        updateWindowCursorShape();
    } else if (event->key() == Qt::Key_Escape && mHasSelection) {
        if (mLastSelectionModification != NoSelectionModification && mLastSelectionModification != SelectionPaste) {
            // We've modified the selection since creating it, so, like mspaint, escape confirms it.
            confirmSelectionModification();
        } else if (mIsSelectionFromPaste) {
            // Pressing escape to clear a pasted selection should apply that selection, like mspaint.
            confirmPasteSelection();
        } else {
            clearSelection();
        }
    } else if (event->modifiers().testFlag(Qt::AltModifier)) {
        setAltPressed(true);
        mToolBeforeAltPressed = mTool;
        setTool(EyeDropperTool);
    } else if (event->modifiers().testFlag(Qt::ShiftModifier)) {
        setShiftPressed(true);
    }
}

void ImageCanvas::keyReleaseEvent(QKeyEvent *event)
{
    QQuickItem::keyReleaseEvent(event);

    if (!mProject->hasLoaded())
        return;

    if (event->isAutoRepeat())
        return;

    if (event->key() == Qt::Key_Space) {
        mSpacePressed = false;
        updateWindowCursorShape();
    } else if (mAltPressed && (event->key() == Qt::Key_Alt || event->modifiers().testFlag(Qt::AltModifier))) {
        restoreToolBeforeAltPressed();
    } else if (event->key() == Qt::Key_Shift || event->modifiers().testFlag(Qt::ShiftModifier)) {
        setShiftPressed(false);
    }
}

void ImageCanvas::focusInEvent(QFocusEvent *event)
{
    QQuickItem::focusInEvent(event);

    updateWindowCursorShape();
}

void ImageCanvas::focusOutEvent(QFocusEvent *event)
{
    QQuickItem::focusOutEvent(event);

    // The alt-to-eyedrop feature is meant to be temporary,
    // so it should restore the previous tool if focus is taken away.
    if (mAltPressed) {
        restoreToolBeforeAltPressed();
    }

    // When e.g. a dialog opens, we shouldn't keep drawing the line preview.
    setShiftPressed(false);

    updateWindowCursorShape();
}

void ImageCanvas::timerEvent(QTimerEvent *event)
{
    if (event->timerId() != mSelectionEdgePanTimer.timerId()) {
        QQuickItem::timerEvent(event);
        return;
    }

    panWithSelectionIfAtEdge(SelectionPanTimerReason);
}

void ImageCanvas::tabletEvent(QTabletEvent *event)
{
    mIsTabletEvent = (event->buttons() != Qt::NoButton);
    mTabletPressure = event->pressure();
    event->accept();
}

void ImageCanvas::updateWindow(QQuickWindow *const window)
{
    if (mWindow) mWindow->removeEventFilter(this);
    mWindow = window;
    if (mWindow) mWindow->installEventFilter(this);
}

void ImageCanvas::undo()
{
    if (mHasSelection && !mIsSelectionFromPaste) {
        if (mLastSelectionModification != NoSelectionModification) {
            qCDebug(lcImageCanvasSelection) << "Undo activated while a selection that has previously been modified is active;"
                << "confirming selection to create undo command, and then instantly undoing it";

            // Create a move command so that the undo can be redone...
            confirmSelectionModification();
            // ... and then immediately undo it. This is weird, but it has
            // to be done this way, because we want to behave like mspsaint, where pressing Ctrl+Z
            // with a modified selection will undo *all* modifications done to the selection since it was created.
            // Since we have special undo behaviour, we can't use the undo framework for all of it, and so
            // we store the temporary state in mSelectionContents (which is displayed via mSelectionPreviewImage).
            // See the undo shortcut in Shortcuts.qml for more info.
            mProject->undoStack()->undo();
//            requestContentPaint();
        } else {
            // Nothing was ever modified, and this isn't a paste, so we can simply clear the selection.
            qCDebug(lcImageCanvasSelection) << "Overriding undo shortcut to cancel selection that hadn't been modified";
            clearSelection();
        }
    } else {
        mProject->undoStack()->undo();
    }
}
