import QtQuick 2.11

import App 1.0

// For access to CanvasColours Singleton
import "."

ImageCanvas {
    id: imageCanvas
    objectName: "imageCanvas"
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
    scrollZoom: settings.scrollZoom
    gesturesEnabled: settings.gesturesEnabled
    anchors.fill: parent

    readonly property bool useCrosshairCursor: imageCanvas.tool === TileCanvas.SelectionTool
        || (imageCanvas.upperToolSize < 4 && imageCanvas.currentPane.zoomLevel <= 3)
    readonly property bool useIconCursor: imageCanvas.tool === TileCanvas.EyeDropperTool

    onErrorOccurred: errorPopup.showError(errorMessage)

    CanvasPaneRepeater {
        canvas: imageCanvas
    }

    SplitterBar {
        canvas: imageCanvas
    }
}
