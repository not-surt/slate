import QtQuick 2.6
import QtQuick.Controls 2.4
import QtQuick.Window 2.2

import App 1.0

import "." as Ui

ToolBar {
    id: root
    objectName: "toolBar"

    property Project project
    property ImageCanvas canvas
    property EditingContext context: canvas ? canvas.editingContext : null
    property Popup canvasSizePopup
    property Popup imageSizePopup

    property alias toolButtonGroup: toolButtonGroup

    readonly property int projectType: project ? project.type : 0
    readonly property bool isTilesetProject: projectType === Project.TilesetType
    readonly property bool isImageProject: projectType === Project.ImageType || projectType === Project.LayeredImageType

    Connections {
        target: canvas
        onToolChanged: {
            switch (canvas.tool) {
            case TileCanvas.PenTool:
                toolButtonGroup.checkedButton = penToolButton;
                break;
            case TileCanvas.EyeDropperTool:
                toolButtonGroup.checkedButton = eyeDropperToolButton;
                break;
            case TileCanvas.EraserTool:
                toolButtonGroup.checkedButton = eraserToolButton;
                break;
            case TileCanvas.FillTool:
            case TileCanvas.TexturedFillTool:
                toolButtonGroup.checkedButton = fillToolButton;
                break;
            case TileCanvas.SelectionTool:
                toolButtonGroup.checkedButton = selectionToolButton;
                break;
            case TileCanvas.CropTool:
                toolButtonGroup.checkedButton = cropToolButton;
                break;
            }
        }
    }

    Row {
        id: toolbarRow
        enabled: canvas
        anchors.fill: parent
        // Make sure that we don't end up on a sub-pixel position.
        anchors.leftMargin: Math.round(toolSeparator.implicitWidth / 2)

        ToolButton {
            id: canvasSizeButton
            objectName: "canvasSizeButton"
            enabled: project && project.loaded
            hoverEnabled: true
            focusPolicy: Qt.NoFocus

            icon.source: "qrc:/images/change-canvas-size.png"

            ToolTip.text: qsTr("Change the size of the canvas")
            ToolTip.visible: hovered && !canvasSizePopup.visible

            onClicked: canvasSizePopup.open()
        }

        ToolButton {
            id: imageSizeButton
            objectName: "imageSizeButton"
            enabled: project && project.loaded && !isTilesetProject
            hoverEnabled: true
            focusPolicy: Qt.NoFocus

            icon.source: "qrc:/images/change-image-size.png"

            ToolTip.text: qsTr("Change the size of the image")
            ToolTip.visible: hovered && !imageSizePopup.visible

            onClicked: imageSizePopup.open()
        }

        ToolSeparator {}

        Row {
            spacing: 5

            Ui.IconToolButton {
                objectName: "undoButton"
                text: "\uf0e2"
                enabled: project && canvas && (project.undoStack.canUndo || canvas.hasModifiedSelection)
                hoverEnabled: true

                ToolTip.text: qsTr("Undo the last canvas operation")
                ToolTip.visible: hovered

                onClicked: canvas.undo()
            }

            Ui.IconToolButton {
                objectName: "redoButton"
                text: "\uf01e"
                enabled: project && project.undoStack.canRedo
                hoverEnabled: true

                ToolTip.text: qsTr("Redo the last undone canvas operation")
                ToolTip.visible: hovered

                onClicked: canvas.redo()
            }

            ToolSeparator {}
        }

        Ui.IconToolButton {
            id: modeToolButton
            objectName: "modeToolButton"
            text: "\uf044"
            checked: canvas && canvas.mode === TileCanvas.TileMode
            checkable: true
            hoverEnabled: true
            enabled: canvas && projectType === Project.TilesetType
            visible: enabled

            ToolTip.text: qsTr("Operate on either pixels or whole tiles")
            ToolTip.visible: hovered

            onClicked: canvas.mode = checked ? TileCanvas.TileMode : TileCanvas.PixelMode
        }

        ToolSeparator {
            visible: modeToolButton.visible
        }

        ButtonGroup {
            id: toolButtonGroup
            objectName: "toolBarButtonGroup"
            buttons: toolLayout.children
        }

        Row {
            id: toolLayout
            spacing: 5

            Ui.IconToolButton {
                id: penToolButton
                objectName: "penToolButton"
                text: "\uf040"
                checked: true
                hoverEnabled: true

                ToolTip.text: qsTr("Draw pixels%1 on the canvas").arg(isTilesetProject ? qsTr(" or tiles") : "")
                ToolTip.visible: hovered

                onClicked: canvas.tool = ImageCanvas.PenTool
            }

            Ui.IconToolButton {
                id: eyeDropperToolButton
                objectName: "eyeDropperToolButton"
                text: "\uf1fb"
                checkable: true
                hoverEnabled: true

                ToolTip.text: qsTr("Select colours%1 from the canvas").arg(isTilesetProject ? qsTr(" or tiles") : "")
                ToolTip.visible: hovered

                onClicked: canvas.tool = ImageCanvas.EyeDropperTool
            }

            Ui.IconToolButton {
                id: eraserToolButton
                objectName: "eraserToolButton"
                text: "\uf12d"
                checkable: true
                hoverEnabled: true

                ToolTip.text: qsTr("Erase pixels%1 from the canvas").arg(isTilesetProject ? qsTr(" or tiles") : "")
                ToolTip.visible: hovered

                onClicked: canvas.tool = ImageCanvas.EraserTool
            }

            ToolButton {
                id: fillToolButton
                objectName: "fillToolButton"
                checkable: true
                hoverEnabled: true
                focusPolicy: Qt.NoFocus

                readonly property bool regularFill: canvas && canvas.lastFillToolUsed === ImageCanvas.FillTool
                readonly property string imageProjectToolTipText:
                    qsTr("Fill a contiguous area with %1pixels.\nHold Shift to fill all pixels matching the target colour.")
                        .arg(!regularFill ? "semi-randomised" : "")

                icon.source: fillToolGroup.checkedAction.icon.source

                ToolTip.text: isTilesetProject ? qsTr("Fill a contiguous area with pixels or tiles") : imageProjectToolTipText
                ToolTip.visible: hovered

                onClicked: canvas.tool = canvas.lastFillToolUsed
                onPressAndHold: if (!isTilesetProject) fillMenu.open()
                // TODO: respond to right clicks when https://bugreports.qt.io/browse/QTBUG-67331 is implemented

                ToolButtonMenuIndicator {
                    visible: !isTilesetProject
                }

                ActionGroup {
                    id: fillToolGroup
                    exclusive: true

                    Action {
                        id: fillTool
                        text: qsTr("Fill Tool")
                        icon.source: "qrc:/images/fill.png"
                        checkable: true
                        checked: canvas && canvas.lastFillToolUsed === ImageCanvas.FillTool
                        onTriggered: canvas.tool = ImageCanvas.FillTool
                    }

                    Action {
                        id: texturedFillTool
                        text: qsTr("Textured Fill Tool")
                        icon.source: "qrc:/images/textured-fill.png"
                        checkable: true
                        checked: canvas && canvas.lastFillToolUsed === ImageCanvas.TexturedFillTool
                        onTriggered: canvas.tool = ImageCanvas.TexturedFillTool
                    }
                }

                Menu {
                    id: fillMenu
                    y: parent.height

                    MenuItem { action: fillTool }
                    MenuItem { action: texturedFillTool }
                }
            }

            ToolButton {
                id: selectionToolButton
                objectName: "selectionToolButton"
                checkable: true
                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                visible: projectType === Project.ImageType || projectType === Project.LayeredImageType

                icon.source: "qrc:/images/selection.png"

                ToolTip.text: qsTr("Select pixels within an area and move them")
                ToolTip.visible: hovered

                onClicked: canvas.tool = ImageCanvas.SelectionTool
            }

            Ui.IconToolButton {
                id: cropToolButton
                objectName: "cropToolButton"
                text: "\uf125"
                checkable: true
                hoverEnabled: true
                visible: false // TODO: implement crop

                ToolTip.text: qsTr("Crop the canvas")
                ToolTip.visible: hovered

                onClicked: canvas.tool = ImageCanvas.CropTool
            }

            ToolSeparator {}
        }

        Ui.IconToolButton {
            id: brushSettingsButton
            objectName: "brushSettingsButton"
            text: "\uf1fc"
            hoverEnabled: true
            focusPolicy: Qt.NoFocus
            checkable: true
            checked: brushSettingsPopup.visible

            ToolTip.text: qsTr("Change the brush settings")
            ToolTip.visible: hovered && !brushSettingsPopup.visible

            ToolButtonMenuIndicator {}

            BrushSettingsPopup {
                id: brushSettingsPopup
                x: parent.width / 2 - width / 2
                y: parent.height
                canvas: root.canvas
                visible: brushSettingsButton.checked
            }
        }

        Ui.IconToolButton {
            id: strokeSettingsButton
            objectName: "strokeSettingsButton"
            text: "\uf5ae"
            hoverEnabled: true
            focusPolicy: Qt.NoFocus
            checkable: true
            checked: strokeSettingsPopup.visible

            ToolTip.text: qsTr("Change the stroke settings")
            ToolTip.visible: hovered && !strokeSettingsPopup.visible

            ToolButtonMenuIndicator {}

            StrokeSettingsPopup {
                id: strokeSettingsPopup
                x: parent.width / 2 - width / 2
                y: parent.height
                canvas: root.canvas
                visible: strokeSettingsButton.checked
            }
        }

        Ui.IconToolButton {
            id: dynamicsSettingsButton
            objectName: "dynamicsSettingsButton"
            text: "\uf1de"
            hoverEnabled: true
            focusPolicy: Qt.NoFocus
            checkable: true
            checked: dynamicsSettingsPopup.visible

            ToolTip.text: qsTr("Change the brush dynamics settings")
            ToolTip.visible: hovered && !dynamicsSettingsButton.visible

            ToolButtonMenuIndicator {}

            DynamicsSettingsPopup {
                id: dynamicsSettingsPopup
                x: parent.width / 2 - width / 2
                y: parent.height
                canvas: root.canvas
                visible: dynamicsSettingsButton.checked
            }
        }

        ToolButton {
            id: blendModeButton
            objectName: "toolBlendModeButton"
            hoverEnabled: true
            focusPolicy: Qt.NoFocus
            checkable: true
            checked: blendModeMenu.visible

            icon.source: blendModeGroup.checkedAction.icon.source

            ToolTip.text: qsTr("Choose blending mode")
            ToolTip.visible: hovered

            ToolButtonMenuIndicator {}

            ActionGroup {
                id: blendModeGroup
                exclusive: true
                checkedAction: canvas ? actions[context.blendMode] : replaceBlendModeAction

                Action {
                    id: blendBlendModeAction
                    text: qsTr("Blend")
                    icon.source: "qrc:/images/blend-tool-blend-mode.png"
                    checkable: true
                    property int blendMode: EditingContext.BlendMode.Blend
                }

                Action {
                    id: replaceBlendModeAction
                    text: qsTr("Replace")
                    icon.source: "qrc:/images/replace-tool-blend-mode.png"
                    checkable: true
                    property int blendMode: EditingContext.BlendMode.Replace
                }

                Action {
                    id: replaceAlphaBlendModeAction
                    text: qsTr("Replace Colour")
                    icon.source: "qrc:/images/replace-tool-blend-mode.png"
                    checkable: true
                    property int blendMode: EditingContext.BlendMode.ReplaceColour
                }

                Action {
                    id: replaceColourBlendModeAction
                    text: qsTr("Replace Alpha")
                    icon.source: "qrc:/images/replace-tool-blend-mode.png"
                    checkable: true
                    property int blendMode: EditingContext.BlendMode.ReplaceAlpha
                }

                Action {
                    id: addBlendModeAction
                    text: qsTr("Add")
                    icon.source: "qrc:/images/replace-tool-blend-mode.png"
                    checkable: true
                    property int blendMode: EditingContext.BlendMode.Add
                }

                Action {
                    id: subtractBlendModeAction
                    text: qsTr("Subtract")
                    icon.source: "qrc:/images/replace-tool-blend-mode.png"
                    checkable: true
                    property int blendMode: EditingContext.BlendMode.Subtract
                }

                Action {
                    id: multiplyBlendModeAction
                    text: qsTr("Multiply")
                    icon.source: "qrc:/images/replace-tool-blend-mode.png"
                    checkable: true
                    property int blendMode: EditingContext.BlendMode.Multiply
                }

                Action {
                    id: eraseBlendModeAction
                    text: qsTr("Erase")
                    icon.source: "qrc:/images/replace-tool-blend-mode.png"
                    checkable: true
                    property int blendMode: EditingContext.BlendMode.Erase
                }
            }

            Binding {
                target: context
                property: "blendMode"
                value: blendModeGroup.checkedAction.blendMode
                when: canvas && blendModeGroup.checkedAction
            }

            Menu {
                id: blendModeMenu
                y: parent.height
                visible: blendModeButton.checked

                MenuItem { action: blendBlendModeAction }
                MenuItem { action: replaceBlendModeAction }
                MenuItem { action: replaceColourBlendModeAction }
                MenuItem { action: replaceAlphaBlendModeAction }
                MenuItem { action: addBlendModeAction }
                MenuItem { action: subtractBlendModeAction }
                MenuItem { action: multiplyBlendModeAction }
                MenuItem { action: eraseBlendModeAction }
            }
        }

        ToolSeparator {
            id: toolSeparator
        }

        Row {
            id: transformLayout
            spacing: 5
            visible: projectType === Project.ImageType || projectType === Project.LayeredImageType

            ToolButton {
                id: rotate90CcwToolButton
                objectName: "rotate90CcwToolButton"
                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                enabled: isImageProject && canvas && canvas.hasSelection

                icon.source: "qrc:/images/rotate-90-ccw.png"

                ToolTip.text: qsTr("Rotate the selection by 90 degrees counter-clockwise")
                ToolTip.visible: hovered

                onClicked: canvas.rotateSelection(-90)
            }

            ToolButton {
                id: rotate90CwToolButton
                objectName: "rotate90CwToolButton"
                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                enabled: isImageProject && canvas && canvas.hasSelection

                icon.source: "qrc:/images/rotate-90-cw.png"

                ToolTip.text: qsTr("Rotate the selection by 90 degrees clockwise")
                ToolTip.visible: hovered

                onClicked: canvas.rotateSelection(90)
            }

            ToolButton {
                id: flipHorizontallyToolButton
                objectName: "flipHorizontallyToolButton"
                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                enabled: isImageProject && canvas && canvas.hasSelection

                icon.source: "qrc:/images/flip-horizontally.png"

                ToolTip.text: qsTr("Flip the selection horizontally")
                ToolTip.visible: hovered

                onClicked: canvas.flipSelection(Qt.Horizontal)
            }

            ToolButton {
                id: flipVerticallyToolButton
                objectName: "flipVerticallyToolButton"
                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                enabled: isImageProject && canvas && canvas.hasSelection

                icon.source: "qrc:/images/flip-vertically.png"

                ToolTip.text: qsTr("Flip the selection vertically")
                ToolTip.visible: hovered

                onClicked: canvas.flipSelection(Qt.Vertical)
            }

            ToolSeparator {}
        }

        Row {
            id: viewLayout
            spacing: 5

            ToolButton {
                objectName: "showRulersToolButton"
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                checkable: true
                checked: settings.rulersVisible

                icon.source: "qrc:/images/show-rulers.png"

                ToolTip.text: qsTr("Show rulers")
                ToolTip.visible: hovered

                onClicked: settings.rulersVisible = checked
            }

            ToolButton {
                objectName: "showGuidesToolButton"
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                checkable: true
                checked: settings.guidesVisible

                icon.source: "qrc:/images/show-guides.png"

                ToolTip.text: qsTr("Show guides")
                ToolTip.visible: hovered

                onClicked: settings.guidesVisible = checked
            }

            ToolButton {
                objectName: "lockGuidesToolButton"
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                checkable: true
                checked: settings.guidesLocked

                icon.source: "qrc:/images/lock-guides.png"

                ToolTip.text: qsTr("Lock guides")
                ToolTip.visible: hovered

                onClicked: settings.guidesLocked = checked
            }
        }

        ToolSeparator {}

        Row {
            id: viewSplitscreenLayout
            spacing: 5

            ToolButton {
                objectName: "splitScreenToolButton"
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                checkable: true
                checked: canvas && canvas.splitScreen

                icon.source: "qrc:/images/splitscreen.png"

                ToolTip.text: qsTr("Split Screen")
                ToolTip.visible: hovered

                onClicked: canvas.splitScreen = checked
            }

            ToolButton {
                objectName: "lockSplitterToolButton"
                focusPolicy: Qt.NoFocus
                hoverEnabled: true
                checkable: true
                checked: canvas && !canvas.splitter.enabled
                enabled: canvas && canvas.splitScreen

                icon.source: "qrc:/images/lock-splitter.png"

                ToolTip.text: qsTr("Lock Splitter")
                ToolTip.visible: hovered

                onClicked: canvas.splitter.enabled = !checked
            }
        }

        ToolSeparator {}

        Row {
            Ui.IconToolButton {
                id: fullScreenButton
                text: "\uF108"
                enabled: true
                hoverEnabled: true
                focusPolicy: Qt.NoFocus
                checkable: true
                checked: window.visibility === Window.FullScreen

                ToolTip.text: qsTr("Toggle fullscreen window")
                ToolTip.visible: hovered

                onClicked: toggleFullScreen()
            }
        }
    }
}
