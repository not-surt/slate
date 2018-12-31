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

#ifndef IMAGECANVAS_H
#define IMAGECANVAS_H

#include <QBasicTimer>
#include <QObject>
#include <QLoggingCategory>
#include <QPixmap>
#include <QQuickItem>
#include <QStack>
#include <QTimerEvent>
#include <QUndoStack>
#include <QWheelEvent>
#include <QPainter>
#include <QtMath>

#include "canvaspane.h"
#include "ruler.h"
#include "selectionitem.h"
#include "slate-global.h"
#include "splitter.h"
#include "texturedfillparameters.h"
#include "brush.h"
#include "stroke.h"

Q_DECLARE_LOGGING_CATEGORY(lcImageCanvas)
Q_DECLARE_LOGGING_CATEGORY(lcImageCanvasLifecycle)

class Guide;
class GuidesItem;
class ImageProject;
class Project;
class SelectionCursorGuide;

class SLATE_EXPORT ImageCanvas : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(Project *project READ project WRITE setProject NOTIFY projectChanged)
    Q_PROPERTY(QColor backgroundColour READ backgroundColour WRITE setBackgroundColour NOTIFY backgroundColourChanged)
    Q_PROPERTY(bool gridVisible READ gridVisible WRITE setGridVisible NOTIFY gridVisibleChanged)
    Q_PROPERTY(QColor gridColour READ gridColour WRITE setGridColour NOTIFY gridColourChanged)
    Q_PROPERTY(bool rulersVisible READ rulersVisible WRITE setRulersVisible NOTIFY rulersVisibleChanged)
    Q_PROPERTY(bool guidesVisible READ guidesVisible WRITE setGuidesVisible NOTIFY guidesVisibleChanged)
    Q_PROPERTY(bool guidesLocked READ guidesLocked WRITE setGuidesLocked NOTIFY guidesLockedChanged)
    Q_PROPERTY(QColor splitColour READ splitColour WRITE setSplitColour NOTIFY splitColourChanged)
    Q_PROPERTY(QColor checkerColour1 READ checkerColour1 WRITE setCheckerColour1 NOTIFY checkerColour1Changed)
    Q_PROPERTY(QColor checkerColour2 READ checkerColour2 WRITE setCheckerColour2 NOTIFY checkerColour2Changed)
    Q_PROPERTY(bool splitScreen READ isSplitScreen WRITE setSplitScreen NOTIFY splitScreenChanged)
    Q_PROPERTY(bool scrollZoom READ scrollZoom WRITE setScrollZoom NOTIFY scrollZoomChanged)
    Q_PROPERTY(bool gesturesEnabled READ areGesturesEnabled WRITE setGesturesEnabled NOTIFY gesturesEnabledChanged)
    Q_PROPERTY(Splitter *splitter READ splitter CONSTANT)
    Q_PROPERTY(CanvasPane *firstPane READ firstPane CONSTANT)
    Q_PROPERTY(CanvasPane *secondPane READ secondPane CONSTANT)
    Q_PROPERTY(CanvasPane *currentPane READ currentPane NOTIFY currentPaneChanged)
    Q_PROPERTY(QColor rulerForegroundColour READ rulerForegroundColour WRITE setRulerForegroundColour)
    Q_PROPERTY(QColor rulerBackgroundColour READ rulerBackgroundColour WRITE setRulerBackgroundColour)
    Q_PROPERTY(QPoint cursorPos READ cursorPos NOTIFY cursorPosChanged)
    Q_PROPERTY(QPointF cursorScenePos READ cursorScenePos NOTIFY cursorScenePosChanged)
    Q_PROPERTY(QPoint cursorScenePixelCoord READ cursorScenePixelCoord NOTIFY cursorScenePosChanged)
    Q_PROPERTY(QPoint cursorScenePixelCorner READ cursorScenePixelCorner NOTIFY cursorScenePosChanged)
    Q_PROPERTY(QColor cursorPixelColour READ cursorPixelColour NOTIFY cursorPixelColourChanged)
    Q_PROPERTY(QColor invertedCursorPixelColour READ invertedCursorPixelColour NOTIFY cursorPixelColourChanged)
    Q_PROPERTY(bool containsMouse READ containsMouse NOTIFY containsMouseChanged)
    Q_PROPERTY(Tool tool READ tool WRITE setTool NOTIFY toolChanged)
    Q_PROPERTY(Tool lastFillToolUsed READ lastFillToolUsed NOTIFY lastFillToolUsedChanged)
    Q_PROPERTY(int lowerToolSize READ lowerToolSize WRITE setLowerToolSize NOTIFY lowerToolSizeChanged)
    Q_PROPERTY(int upperToolSize READ upperToolSize WRITE setUpperToolSize NOTIFY upperToolSizeChanged)
    Q_PROPERTY(int maxToolSize READ maxToolSize CONSTANT)
    Q_PROPERTY(QRectF brushRect READ brushRect NOTIFY brushRectChanged)
    Q_PROPERTY(bool toolSizeUsePressure READ toolSizeUsePressure WRITE setToolSizeUsePressure NOTIFY toolSizeUsePressureChanged)
    Q_PROPERTY(qreal lowerToolOpacity READ lowerToolOpacity WRITE setLowerToolOpacity NOTIFY lowerToolOpacityChanged)
    Q_PROPERTY(qreal upperToolOpacity READ upperToolOpacity WRITE setUpperToolOpacity NOTIFY upperToolOpacityChanged)
    Q_PROPERTY(bool toolOpacityUsePressure READ toolOpacityUsePressure WRITE setToolOpacityUsePressure NOTIFY toolOpacityUsePressureChanged)
    Q_PROPERTY(Brush::Type brushType READ brushType WRITE setBrushType NOTIFY brushTypeChanged)
    Q_PROPERTY(ToolBlendMode toolBlendMode READ toolBlendMode WRITE setToolBlendMode NOTIFY toolBlendModeChanged)
    Q_PROPERTY(QColor penForegroundColour READ penForegroundColour WRITE setPenForegroundColour NOTIFY penForegroundColourChanged)
    Q_PROPERTY(QColor penBackgroundColour READ penBackgroundColour WRITE setPenBackgroundColour NOTIFY penBackgroundColourChanged)
    Q_PROPERTY(TexturedFillParameters *texturedFillParameters READ texturedFillParameters CONSTANT FINAL)
    Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY hasSelectionChanged)
    Q_PROPERTY(bool hasModifiedSelection READ hasModifiedSelection NOTIFY hasModifiedSelectionChanged)
    Q_PROPERTY(QRect selectionArea READ selectionArea NOTIFY selectionAreaChanged)
    Q_PROPERTY(bool adjustingImage READ isAdjustingImage NOTIFY adjustingImageChanged)
    Q_PROPERTY(bool hasBlankCursor READ hasBlankCursor NOTIFY hasBlankCursorChanged)
    Q_PROPERTY(bool altPressed READ isAltPressed NOTIFY altPressedChanged)
    Q_PROPERTY(bool lineVisible READ isLineVisible NOTIFY lineVisibleChanged)
    Q_PROPERTY(int lineLength READ lineLength NOTIFY lineLengthChanged)
    Q_PROPERTY(qreal lineAngle READ lineAngle NOTIFY lineLengthChanged)
    Q_PROPERTY(int currentColourIndex READ currentColourIndex WRITE setCurrentColourIndex NOTIFY currentColourIndexChanged)
    Q_PROPERTY(int currentTileIndex READ currentTileIndex WRITE setCurrentTileIndex NOTIFY currentTileIndexChanged)

public:
    // The order of these is important, as the number keys can activate the tools.
    enum Tool {
        PenTool,
        EyeDropperTool,
        EraserTool,
        FillTool,
        SelectionTool,
        TexturedFillTool,
        CropTool
    };
    Q_ENUM(Tool)

    enum ToolBlendMode {
        BlendToolBlendMode,
        ReplaceToolBlendMode,
    };
    Q_ENUM(ToolBlendMode)

    ImageCanvas();
    virtual ~ImageCanvas() override;

    virtual bool eventFilter(QObject *watched, QEvent *event) override;

    Project *project() const;
    void setProject(Project *project);

    QPoint cursorPos() const;
    void setCursorPos(const QPoint point);

    QPointF pressScenePos() const;
    QPoint pressScenePixelCoord() const;
    QPoint pressScenePixelCorner() const;

    QPointF cursorScenePos() const;
    void setCursorScenePos(const QPointF point);
    QPoint cursorScenePixelCoord() const;
    QPoint cursorScenePixelCorner() const;

    QColor cursorPixelColour() const;
    QColor invertedCursorPixelColour() const;
    static QColor invertedColour(const QColor &colour);

    bool containsMouse() const;
    void setContainsMouse(bool containsMouse);

    bool gridVisible() const;
    void setGridVisible(bool gridVisible);

    QColor gridColour() const;
    void setGridColour(const QColor &gridColour);

    bool rulersVisible() const;
    void setRulersVisible(bool rulersVisible);

    bool guidesVisible() const;
    void setGuidesVisible(bool guidesVisible);

    bool guidesLocked() const;
    void setGuidesLocked(bool guidesLocked);

    QColor splitColour() const;
    void setSplitColour(const QColor &splitColour);

    QColor checkerColour1() const;
    void setCheckerColour1(const QColor &colour);

    QColor checkerColour2() const;
    void setCheckerColour2(const QColor &colour);

    QColor backgroundColour() const;
    void setBackgroundColour(const QColor &backgroundColour);

    bool isSplitScreen() const;
    void setSplitScreen(bool splitScreen);

    bool scrollZoom() const;
    void setScrollZoom(bool scrollZoom);

    bool areGesturesEnabled() const;
    void setGesturesEnabled(bool gesturesEnabled);

    Ruler *pressedRuler() const;
    int pressedGuideIndex() const;

    Splitter *splitter();

    CanvasPane *firstPane();
    const CanvasPane *firstPane() const;
    CanvasPane *secondPane();
    const CanvasPane *secondPane() const;
    CanvasPane *currentPane();
    const CanvasPane *paneAt(int index) const;
    Q_INVOKABLE CanvasPane *paneAt(int index);
    const QVector<CanvasPane *> &panes() const;
    int paneWidth(int index) const;

    QColor rulerForegroundColour() const;
    void setRulerForegroundColour(const QColor &foregroundColour) const;

    QColor rulerBackgroundColour() const;
    void setRulerBackgroundColour(const QColor &backgroundColour) const;

    QColor mapBackgroundColour() const;

    Tool tool() const;
    void setTool(const Tool &tool);

    Brush::Type brushType() const;
    void setBrushType(const Brush::Type &brushType);

    ToolBlendMode toolBlendMode() const;
    void setToolBlendMode(const ToolBlendMode &toolBlendMode);

    Tool lastFillToolUsed() const;

    int lowerToolSize() const;
    void setLowerToolSize(int lowerToolSize);
    int upperToolSize() const;
    void setUpperToolSize(int upperToolSize);
    int maxToolSize() const;
    bool toolSizeUsePressure() const;
    void setToolSizeUsePressure(bool toolSizeUsePressure);

    QRectF brushRect();

    qreal lowerToolOpacity() const;
    void setLowerToolOpacity(qreal lowerToolOpacity);
    qreal upperToolOpacity() const;
    void setUpperToolOpacity(qreal upperToolOpacity);
    bool toolOpacityUsePressure() const;
    void setToolOpacityUsePressure(bool toolOpacityUsePressure);

    QColor penForegroundColour() const;
    void setPenForegroundColour(const QColor &penForegroundColour);

    QColor penBackgroundColour() const;
    void setPenBackgroundColour(const QColor &penBackgroundColour);

    TexturedFillParameters *texturedFillParameters();

    bool hasSelection() const;
    bool hasModifiedSelection() const;

    QRect selectionArea() const;
    void setSelectionArea(const QRect &selectionArea);

    bool isAdjustingImage() const;

    bool hasBlankCursor() const;

    bool isAltPressed() const;

    bool isLineVisible() const;
    int lineLength() const;
    qreal lineAngle() const;

    struct SubImage {
        bool operator==(const SubImage &other) const {
            return imageIndex == other.imageIndex && bounds == other.bounds && origin == other.origin;
        }

        // Index of master image
        int imageIndex;
        // Image-space bounds
        QRect bounds;
        // Local-space origin
        QPoint origin;
    };

    virtual SubImage getSubImage(const int index) const;

    struct SubImageInstance {
        bool operator==(const SubImageInstance &other) const {
            return index == other.index && position == other.position;
        }

        // Index of master subimage
        int index;
        // Scene-space position
        QPoint position;
        // Other transform?
    };

    virtual QList<SubImageInstance> subImageInstancesInBounds(const QRect &bounds) const;
    QColor pickColour(const QPointF point) const;

    int currentColourIndex() const;
    int currentTileIndex() const;

    // Essentially currentProjectImage() for regular image canvas, but may return a
    // preview image if there is a selection active. For layered image canvases, this
    // should return all layers flattened into one image, or the same flattened image
    // as part of a selection preview image.
    //
    // This function calls getContentImage() and caches the result so that we have
    // cheap lookup of pixel data, which is useful for e.g. mCursorPixelColour.
    //
    // Public for auto test access.
    QImage contentImage();

    Q_INVOKABLE void undo();

    // The image that is currently being drawn on. For regular image canvases, this is
    // the project's image. For layered image canvases, this is the image belonging to
    // the current layer.
    virtual const QImage *currentProjectImage() const;
    virtual QImage *currentProjectImage();

    virtual const QImage *imageForLayerAt(const int layerIndex) const;
    virtual QImage *imageForLayerAt(const int layerIndex);
    virtual int currentLayerIndex() const;

    virtual void paintBackground(QPainter *const painter, const QRect &rect) const;
    virtual void paintContent(QPainter *const painter, const QRect &rect);
    virtual void paintContentWithPreview(QPainter *const painter, const QRect &rect);
    virtual void paintOverlay(QPainter *const painter, const QRect &rect) const {}

    enum SelectionModification {
        NoSelectionModification,
        SelectionPaste,
        SelectionMove,
        SelectionFlip,
        SelectionRotate,
        SelectionHsl
    };
    // For nice printing.
    Q_ENUM(SelectionModification)

    enum AdjustmentAction {
        RollbackAdjustment,
        CommitAdjustment
    };
    Q_ENUM(AdjustmentAction)

signals:
    void projectChanged();
    void zoomLevelChanged();
    void cursorPosChanged();
    void cursorScenePosChanged();
    void cursorPixelColourChanged();
    void containsMouseChanged();
    void backgroundColourChanged();
    void gridVisibleChanged();
    void gridColourChanged();
    void checkerColour1Changed();
    void checkerColour2Changed();
    void rulersVisibleChanged();
    void guidesVisibleChanged();
    void guidesLockedChanged();
    void splitColourChanged();
    void splitScreenChanged();
    void scrollZoomChanged();
    void gesturesEnabledChanged();
    void currentPaneChanged();
    void toolChanged();
    void brushTypeChanged();
    void toolBlendModeChanged();
    void lastFillToolUsedChanged();
    void lowerToolSizeChanged();
    void upperToolSizeChanged();
    void brushRectChanged();
    void toolSizeUsePressureChanged();
    void lowerToolOpacityChanged();
    void upperToolOpacityChanged();
    void toolOpacityUsePressureChanged();
    void penForegroundColourChanged();
    void penBackgroundColourChanged();
    void hasBlankCursorChanged();
    void hasSelectionChanged();
    void hasModifiedSelectionChanged();
    void selectionAreaChanged();
    void adjustingImageChanged();
    void altPressedChanged();
    void lineVisibleChanged();
    void lineLengthChanged();
    void pasteSelectionConfirmed();
    void currentColourIndexChanged(const int currentColourIndex);
    void currentTileIndexChanged(const int currentTileIndex);

    // Used to signal CanvasPaneItem classes that they should redraw,
    // instead of them having to connect to lots of specific signals.
    // paneIndex is the index of the pane that should be redrawn,
    // or -1 for all panes.
    void contentPaintRequested(int paneIndex);

    void errorOccurred(const QString &errorMessage);

public slots:
    void centreView();
    void zoomIn();
    void zoomOut();

    void flipSelection(Qt::Orientation orientation);
    void rotateSelection(int angle);
    void beginModifyingSelectionHsl();
    void modifySelectionHsl(qreal hue, qreal saturation, qreal lightness);
    void endModifyingSelectionHsl(AdjustmentAction adjustmentAction);
    void copySelection();
    void paste();
    void deleteSelectionOrContents();
    void selectAll();
    void brushFromSelection();
    void setCurrentColourIndex(const int currentColourIndex);
    void setCurrentTileIndex(const int currentTileIndex);

    void cycleFillTools();

protected slots:
    virtual void reset();
    virtual void onLoadedChanged();

    // Requests both panes to be repainted. Most operations
    // (like the user drawing pixels) require both panes to be redrawn,
    // but stuff like panning does not, and hence it should use
    // requestPaneContentPaint() and pass a specific index.
    void requestContentPaint();
    void requestPaneContentPaint(int paneIndex);
    void updateWindowCursorShape();
    void onZoomLevelChanged();
    void onPaneOffsetChanged();
    void onPaneSizeChanged();
    void onSplitterPositionChanged();
    void onGuidesChanged();
    void onReadyForWritingToJson(QJsonObject* projectJson);
    void onAboutToBeginMacro(const QString &macroText);
    void recreateCheckerImage();

protected:
    void componentComplete() override;
    virtual void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;

    void resizeChildren();

    friend class ApplyGreedyPixelFillCommand;
    friend class ApplyPixelFillCommand;
    friend class ApplyPixelLineCommand;
    friend class ModifyImageCanvasSelectionCommand;
    friend class DeleteImageCanvasSelectionCommand;
    friend class FlipImageCanvasSelectionCommand;
    friend class PasteImageCanvasCommand;

    struct PixelCandidateData
    {
        QVector<QPoint> scenePositions;
        QVector<QColor> previousColours;
    };
    QImage fillPixels() const;
    QImage greedyFillPixels() const;
    QImage texturedFillPixels() const;
    QImage greedyTexturedFillPixels() const;

    QPainter::CompositionMode qPainterBlendMode() const;
    virtual void applyCurrentTool(QUndoStack *const alternateStack = nullptr);
    virtual void applyPixelPenTool(int layerIndex, const QPoint &scenePos, const QColor &colour);
    void paintImageOntoPortionOfImage(int layerIndex, const QRect &portion, const QImage &replacementImage);
    void replacePortionOfImage(int layerIndex, const QRect &portion, const QImage &replacementImage);
    void erasePortionOfImage(int layerIndex, const QRect &portion);
    virtual void replaceImage(int layerIndex, const QImage &replacementImage);
    void doFlipSelection(int layerIndex, const QRect &area, Qt::Orientation orientation);
    QRect doRotateSelection(int layerIndex, const QRect &area, int angle);

    void updateBrush();
    const Brush &brush();
    qreal pressure() const;

    virtual void updateCursorPos(const QPoint &eventPos);
    void error(const QString &message);

    Qt::MouseButton pressedMouseButton() const;
    QColor penColour() const;
    void setPenColour(const QColor &colour);
    void setHasBlankCursor(bool hasBlankCursor);
    void restoreToolBeforeAltPressed();
    virtual bool areToolsForbidden() const;
    void setCursorPixelColour(const QColor &cursorPixelColour);
    bool isWithinImage(const QPoint &scenePos) const;
    QPoint clampToImageBounds(const QPoint &scenePos, bool inclusive = true) const;
    void setLastFillToolUsed(Tool lastFillToolUsed);

    void setCurrentPane(CanvasPane *pane);
    CanvasPane *hoveredPane(const QPoint &pos);
    virtual QImage getComposedImage();
    enum ResetPaneSizePolicy {
        DontResetPaneSizes,
        ResetPaneSizes
    };
    void doSetSplitScreen(bool splitScreen);
    void setDefaultPaneSizes();
    void updatePaneGeometry();
    bool mouseOverSplitterHandle(const QPoint &mousePos);

    void updateRulerVisibility();
    void resizeRulers();
    void updatePressedRuler();
    Ruler *rulerAtCursorPos();

    void addNewGuide();
    void moveGuide();
    void removeGuide();
    void updatePressedGuide();
    int guideIndexAtCursorPos();

    bool isPanning() const;

    enum SelectionPanReason {
        SelectionPanMouseMovementReason,
        SelectionPanTimerReason
    };

    virtual bool supportsSelectionTool() const;
    void beginSelectionMove();
    void updateOrMoveSelectionArea();
    void updateSelectionArea();
    void updateSelectionPreviewImage(SelectionModification reason = NoSelectionModification);
    void moveSelectionArea();
    void moveSelectionAreaBy(const QPoint &pixelDistance);
    void confirmSelectionModification();
    QRect clampSelectionArea(const QRect &selectionArea) const;
    QRect boundSelectionArea(const QRect &selectionArea) const;
    void clearSelection();
    void clearOrConfirmSelection();
    void setHasSelection(bool hasSelection);
    void setMovingSelection(bool movingSelection);
    bool cursorOverSelection() const;
    bool shouldDrawSelectionPreviewImage() const;
    bool shouldDrawSelectionCursorGuide() const;
    void updateSelectionCursorGuideVisibility();
    void confirmPasteSelection();
    void setSelectionFromPaste(bool isSelectionFromPaste);
    void panWithSelectionIfAtEdge(SelectionPanReason reason);
    void setLastSelectionModification(SelectionModification selectionModification);
    void setHasModifiedSelection(bool hasModifiedSelection);

    void setAltPressed(bool altPressed);

    void setShiftPressed(bool shiftPressed);

    void applyZoom(qreal zoom, const QPoint &origin);

    virtual void connectSignals();
    virtual void disconnectSignals();
    virtual void toolChange();

    bool event(QEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void hoverEnterEvent(QHoverEvent *event) override;
    void hoverMoveEvent(QHoverEvent *event) override;
    void hoverLeaveEvent(QHoverEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void timerEvent(QTimerEvent *event) override;
    void tabletEvent(QTabletEvent *event);

    void updateWindow(QQuickWindow *const window);

    Project *mProject;

private:
    ImageProject *mImageProject;

protected:
    friend class CanvasPaneItem;

    // The background colour of the entire pane.
    QColor mBackgroundColour;
    bool mRulersVisible;
    bool mGridVisible;
    QColor mGridColour;
    QColor mSplitColour;
    QPixmap mCheckerPixmap;
    QImage mCheckerImage;
    QColor mCheckerColour1;
    QColor mCheckerColour2;

    bool mSplitScreen;
    Splitter mSplitter;
    CanvasPane mFirstPane;
    CanvasPane mSecondPane;
    QVector<CanvasPane *> mPanes;
    QVector<Ruler *> mHorizontalRulers;
    QVector<Ruler *> mVerticalRulers;
    CanvasPane *mCurrentPane;
    int mCurrentPaneIndex;
    Ruler *mFirstHorizontalRuler;
    Ruler *mFirstVerticalRuler;
    Ruler *mSecondHorizontalRuler;
    Ruler *mSecondVerticalRuler;
    Ruler *mPressedRuler;
    bool mGuidesVisible;
    bool mGuidesLocked;
    int mGuidePositionBeforePress;
    int mPressedGuideIndex;
    GuidesItem *mGuidesItem;
    SelectionItem *mSelectionItem;

    // The position of the cursor in view coordinates.
    QPoint mCursorPos;
    // The position of the cursor in view coordinates relative to a pane.
    int mCursorPaneX;
    int mCursorPaneY;
    // The position of the cursor in scene coordinates.
    QPointF mCursorScenePos;
    QColor mCursorPixelColour;
    bool mContainsMouse;
    QPoint oldMousePos;
    // The mouse button that is currently pressed.
    Qt::MouseButton mMouseButtonPressed;
    // The mouse button that was last pressed (could still be currently pressed).
    Qt::MouseButton mLastMouseButtonPressed;
    // The position at which the mouse is currently pressed.
    QPoint mPressPosition;
    QPointF mPressScenePos;
    bool mToolContinue;
    Stroke mOldStroke, mNewStroke;
    // The scene position at which the mouse was pressed before the most-recent press.
    bool mScrollZoom;
    bool mGesturesEnabled;

    qreal mTabletPressure;
    bool mIsTabletEvent;
    Tool mTool;
    Brush::Type mBrushType;
    ToolBlendMode mToolBlendMode;
    Tool mLastFillToolUsed;
    int mLowerToolSize, mUpperToolSize;
    int mMaxToolSize;
    bool mToolSizeUsePressure;
    qreal mLowerToolOpacity, mUpperToolOpacity;
    bool mToolOpacityUsePressure;
    QColor mPenForegroundColour;
    QColor mPenBackgroundColour;
    Brush mBrush;
    int mCurrentColourIndex;
    int mCurrentTileIndex;

    TexturedFillParameters mTexturedFillParameters;

    bool mPotentiallySelecting;
    bool mHasSelection;
    bool mMovingSelection;
    bool mIsSelectionFromPaste;
    bool mConfirmingSelectionModification;
    // The current selection area. This is set as soon as we receive a mouse press event
    // outside of any existing selection, which means that it starts off with an "empty" size.
    QRect mSelectionArea;
    QRect mSelectionAreaBeforeFirstModification;
    // The selection area before the most recent move.
    QRect mSelectionAreaBeforeLastMove;
    // The last selection area with a non-empty size. This is set after a mouse release event.
    QRect mLastValidSelectionArea;
    // The image contents of the selection.
    QImage mSelectionContents;
    // The entire image as it would look if the selection (that is currently being dragged)
    // was dropped where it is now.
    QImage mSelectionPreviewImage;
    // See the definition of beginModifyingSelectionHsl() for info.
    QImage mSelectionContentsBeforeImageAdjustment;
    SelectionModification mLastSelectionModificationBeforeImageAdjustment;
    QBasicTimer mSelectionEdgePanTimer;
    SelectionCursorGuide *mSelectionCursorGuide;
    // The type of the last modification that was done to the selection.
    SelectionModification mLastSelectionModification;
    // True if the selection was moved, flipped, rotated, etc.
    bool mHasModifiedSelection;

    QRect mCropArea;

    bool mAltPressed;
    bool mShiftPressed;
    Tool mToolBeforeAltPressed;
    bool mSpacePressed;
    bool mHasBlankCursor;    

    QQuickWindow *mWindow;
};

inline uint qHash(const ImageCanvas::SubImageInstance &key, const uint seed = 0) {
    return qHashBits(&key, sizeof(ImageCanvas::SubImageInstance), seed);
}

inline QDebug operator<<(QDebug debug, const ImageCanvas::SubImage &subImage)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "SubImage(" << subImage.imageIndex << ", " << subImage.bounds << ", " << subImage.origin << ')';

    return debug;
}

inline QDebug operator<<(QDebug debug, const ImageCanvas::SubImageInstance &subImageInstance)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "SubImageInstance(" << subImageInstance.index << ", " << subImageInstance.position << ')';

    return debug;
}

#endif // IMAGECANVAS_H
