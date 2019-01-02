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
    scrollZoom: settings.scrollZoom
    gesturesEnabled: settings.gesturesEnabled

    readonly property bool useCrosshairCursor: layeredCanvas.tool === ImageCanvas.SelectionTool
        || (layeredCanvas.upperToolSize < 4 && layeredCanvas.currentPane.zoomLevel <= 3)
    readonly property bool useIconCursor: layeredCanvas.tool === ImageCanvas.EyeDropperTool

    onErrorOccurred: errorPopup.showError(errorMessage)

    CanvasPaneRepeater {
        canvas: layeredCanvas
    }

    SplitterBar {
        canvas: layeredCanvas
    }
}
