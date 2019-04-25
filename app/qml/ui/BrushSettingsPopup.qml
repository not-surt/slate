import QtQuick 2.6
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

import App 1.0

import "." as Ui

Popup {
    id: root
    focus: true

    property ImageCanvas canvas
    property EditingContext context: canvas ? canvas.editingContext : null

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
//                onTriggered: if (canvas) context.brush.type = action.brushType
//                checkedAction: canvas ? actions[context.brush.type] : squareBrushType

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
            currentIndex: canvas ? context.brush.type : 0

            Binding {
                target: context.brush
                property: "type"
                value: typeComboBox.currentIndex // why parent.currentIndex not work?
                when: canvas
            }
        }

        Ui.IconToolButton {
            id: singleColourButton
            text: "\uf1fc"
            hoverEnabled: true

            checked: canvas ? context.brush.singleColour : false
            checkable: true
            enabled: canvas && context.brush.type === Brush.ImageType

            ToolTip.text: qsTr("Replace brush colours with current colour")
            ToolTip.visible: hovered

            Binding {
                target: context.brush
                property: "singleColour"
                value: singleColourButton.checked
                when: canvas
            }
        }

        SizeControl {
            id: sizeControl
            Layout.columnSpan: 3
            Layout.fillWidth: true
            value: canvas ? context.brush.size : Qt.size()

            Binding {
                target: context.brush
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
            value: canvas ? context.brush.angle : 0
            textFromValue: function(value, locale) { return value + "\u00b0" }
            valueFromText: function(text, locale) { return parseInt(text, 10) }

            Layout.columnSpan: 2

            Binding {
                target: context.brush
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
            value: canvas ? context.brush.opacity : 1
            enabled: canvas && (context.brush.type != Brush.ImageType || (context.brush.type == Brush.ImageType && singleColourButton.checked))

            Binding {
                target: context.brush
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
            value: canvas ? context.brush.hardness : 1
            enabled: canvas && (context.brush.type != Brush.ImageType || (context.brush.type == Brush.ImageType && singleColourButton.checked))

            Binding {
                target: context.brush
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
