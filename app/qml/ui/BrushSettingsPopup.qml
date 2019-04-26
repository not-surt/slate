import QtQuick 2.6
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

import App 1.0

import "." as Ui

Popup {
    id: root
    focus: true

    property ImageCanvas canvas
    property EditingContextManager context: canvas ? canvas.editingContextManager : null

    contentItem: GridLayout {
        columns: 3

        Label {
            text: qsTr("Type")
        }

//        ToolButton {
//            id: typeButton
//            hoverEnabled: true
//            Layout.fillWidth: true
//            flat: true
//            checkable: true
//            checked: typeMenu.visible
//            display: AbstractButton.TextBesideIcon
//            text: typeGroup.checkedAction.text
//            icon.source: typeGroup.checkedAction.icon.source

//            ToolTip.text: qsTr("Choose brush shape")
//            ToolTip.visible: hovered && !typeMenu.visible

//            ToolButtonMenuIndicator {}

//            ActionGroup {
//                id: typeGroup
//                exclusive: true
//                onTriggered: if (canvas) context.brushManager.type = action.brushType
//                checkedAction: canvas ? actions[context.brushManager.type] : squareBrushType

//                Action {
//                    id: squareBrushType
//                    text: qsTr("Square")
//                    icon.source: "qrc:/images/square-tool-shape.png"
//                    checkable: true
//                    property int brushType: Brush.SquareType
//                }

//                Action {
//                    id: circleBrushType
//                    text: qsTr("Circle")
//                    icon.source: "qrc:/images/circle-tool-shape.png"
//                    checkable: true
//                    property int brushType: Brush.CircleType
//                }

//                Action {
//                    id: imageBrushType
//                    text: qsTr("Image")
//                    icon.source: "qrc:/images/image-tool-shape.png"
//                    checkable: true
//                    property int brushType: Brush.ImageType
//                }
//            }

//            Menu {
//                id: typeMenu
//                y: parent.height
//                visible: parent.checked

//                MenuItem { action: squareBrushType}
//                MenuItem { action: circleBrushType }
//                MenuItem { action: imageBrushType }
//            }
//        }
        ComboBox {
            id: typeComboBox
            Layout.fillWidth: true
            model: [
                qsTr("Square"),
                qsTr("Circle"),
                qsTr("Image")
            ]
            currentIndex: canvas ? context.brushManager.type : 0

            Binding {
                target: context.brushManager
                property: "type"
                value: typeComboBox.currentIndex // why parent.currentIndex not work?
                when: canvas
            }
        }

        Ui.IconToolButton {
            id: singleColourButton
            text: "\uf1fc"
            hoverEnabled: true

            checked: canvas ? context.brushManager.singleColour : false
            checkable: true
            enabled: canvas && context.brushManager.type === Brush.ImageType

            ToolTip.text: qsTr("Replace brush colours with current colour")
            ToolTip.visible: hovered

            Binding {
                target: context.brushManager
                property: "singleColour"
                value: singleColourButton.checked
                when: canvas
            }
        }

        SizeControl {
            id: sizeControl
            Layout.columnSpan: 3
            Layout.fillWidth: true
            value: canvas ? context.brushManager.size : Qt.size()

            Binding {
                target: context.brushManager
                property: "size"
                when: canvas
                value: sizeControl.value
            }
        }

        Label {
            text: qsTr("Angle")
        }

        SpinBox {
            id: angleSpinBox
            editable: true
            from: -360
            to: 360
            stepSize: 1
            Layout.fillWidth: true
            value: canvas ? context.brushManager.angle : 0
            textFromValue: function(value, locale) { return value + "\u00b0" }
            valueFromText: function(text, locale) { return parseInt(text, 10) }

            Layout.columnSpan: 2

            Binding {
                target: context.brushManager
                property: "angle"
                value: angleSpinBox.value
                when: canvas
            }
        }

        Label {
            text: qsTr("Opacity")
        }

        Slider {
            id: opacitySlider
            from: 0.0
            to: 1.0
            Layout.fillWidth: true
            value: canvas ? context.brushManager.opacity : 1
            enabled: canvas && (context.brushManager.type != Brush.ImageType || (context.brushManager.type == Brush.ImageType && singleColourButton.checked))

            Binding {
                target: context.brushManager
                property: "opacity"
                value: opacitySlider.value
                when: canvas
            }
        }

        Label {
            text: opacitySlider.value.toFixed(3)
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Hardness")
        }

        Slider {
            id: hardnessSlider
            from: 0.0
            to: 1.0
            Layout.fillWidth: true
            value: canvas ? context.brushManager.hardness : 1
            enabled: canvas && (context.brushManager.type != Brush.ImageType || (context.brushManager.type == Brush.ImageType && singleColourButton.checked))

            Binding {
                target: context.brushManager
                property: "hardness"
                value: hardnessSlider.value
                when: canvas
            }
        }

        Label {
            text: hardnessSlider.value.toFixed(3)
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
    }
}
