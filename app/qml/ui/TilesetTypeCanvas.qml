import QtQuick 2.11

import App 1.0

// For access to CanvasColours Singleton
import "."

TileCanvas {
    id: tileCanvas
    objectName: "tileCanvas"
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
    scrollZoom: settings.scrollZoom
    gesturesEnabled: settings.gesturesEnabled

    readonly property bool useCrosshairCursor: tileCanvas.mode === TileCanvas.TileMode
        || tileCanvas.tool === TileCanvas.SelectionTool || (tileCanvas.upperToolSize < 4 && tileCanvas.currentPane.zoomLevel <= 3)
    readonly property bool useIconCursor: tileCanvas.tool === TileCanvas.EyeDropperTool

    onErrorOccurred: errorPopup.showError(errorMessage)

    CanvasPaneRepeater {
        canvas: tileCanvas
    }

    SplitterBar {
        canvas: tileCanvas
    }
}
