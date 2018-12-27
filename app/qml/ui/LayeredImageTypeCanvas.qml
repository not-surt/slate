import QtQuick 2.12

import App 1.0

// For access to CanvasColours Singleton
import "."

LayeredImageCanvas {
    id: layeredCanvas
    objectName: "layeredImageCanvas"
    focus: true
    project: args.project
    backgroundColour: CanvasColours.backgroundColour
    checkerColour1: settings.checkerColour1
    checkerColour2: settings.checkerColour2
    gridVisible: settings.gridVisible
    gridColour: "#55000000"
    rulersVisible: settings.rulersVisible
    guidesVisible: settings.guidesVisible
    guidesLocked: settings.guidesLocked
    splitColour: CanvasColours.splitColour
    splitter.width: 32
    rulerForegroundColour: CanvasColours.rulerForegroundColour
    rulerBackgroundColour: CanvasColours.rulerBackgroundColour
    scrollZoom: settings.scrollZoom
    gesturesEnabled: settings.gesturesEnabled
    anchors.fill: parent

    readonly property int currentPaneZoomLevel: layeredCanvas.currentPane ? layeredCanvas.currentPane.integerZoomLevel : 1
    readonly property point currentPaneOffset: layeredCanvas.currentPane ? layeredCanvas.currentPane.integerOffset : Qt.point(0, 0)
    readonly property bool useCrosshairCursor: layeredCanvas.tool === ImageCanvas.SelectionTool
        || (layeredCanvas.upperToolSize < 4 && layeredCanvas.currentPaneZoomLevel <= 3)
    readonly property bool useIconCursor: layeredCanvas.tool === ImageCanvas.EyeDropperTool

    onErrorOccurred: errorPopup.showError(errorMessage)

    CanvasPaneRepeater {
        canvas: layeredCanvas
    }

    SplitterBar {
        canvas: layeredCanvas
    }

//    PinchHandler {
//        id: handler
//        target: layeredCanvas
//        onScaleChanged: {
//            print("scale", scale, "position", centroid.position, centroid.pressPosition, centroid.sceneGrabPosition, centroid.scenePosition, centroid.scenePressPosition)
////            layeredCanvas.currentPane.zoomLevel = scale
//            layeredCanvas.applyZoom(scale, centroid.position)
//        }
////        centroid.onPositionChanged: print("centroid", centroid.position)
////        onRotationChanged: print(rotation)
////        onTranslationChanged: print(translation)
//    }
}
