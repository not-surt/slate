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
        id: content
        columns: 5

        property var scalingModeStrings: [
            qsTr("None"),
            qsTr("Tablet Pressure"),
            qsTr("Stroke Tappering")
        ]
        property var rotationModeStrings: [
            qsTr("None"),
            qsTr("Tablet Rotation"),
            qsTr("Tablet Tilt"),
            qsTr("Stroke Angle")
        ]
        property real pressureMin: 0.0
        property real pressureMax: 1.0
        property real rotationMin: -360.0
        property real rotationMax: 360.0

        Label {
            text: qsTr("Scale")
        }

        ComboBox {
            id: scaleComboBox
            model: content.scalingModeStrings
            currentIndex: canvas ? context.brushScalingMode : 0

            Binding {
                target: context
                property: "brushScalingMode"
                value: scaleComboBox.currentIndex // why parent.currentIndex not work?
                when: canvas
            }
        }

        Slider {
            id: brushScalingMinSlider
            from: content.pressureMin
            to: 1.0
            Layout.fillWidth: true
            value: canvas ? context.brushScalingMin : 1

            ToolTip.text: qsTr("Brush minimum scale")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "brushScalingMin"
                value: parent.value
                when: canvas
            }
        }

        Slider {
            id: brushScalingMaxSlider
            from: content.pressureMin
            to: content.pressureMax
            Layout.fillWidth: true
            value: canvas ? context.brushScalingMax : 1

            ToolTip.text: qsTr("Brush maximum scale")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "brushScalingMax"
                value: parent.value
                when: canvas
            }
        }

        Label {
            text: brushScalingMinSlider.value.toFixed(3) + " to " + brushScalingMaxSlider.value.toFixed(3)
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Rotation")
        }

        ComboBox {
            id: rotationComboBox
            model: content.rotationModeStrings
            currentIndex: canvas ? context.brushRotationMode : 0

            Binding {
                target: context
                property: "brushRotationMode"
                value: rotationComboBox.currentIndex // why parent.currentIndex not work?
                when: canvas
            }
        }

        Slider {
            id: brushRotationMinSlider
            from: content.rotationMin
            to: content.rotationMax
            Layout.fillWidth: true
            value: canvas ? context.brushRotationMin : 1

            ToolTip.text: qsTr("Brush minimum rotation")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "brushRotationMin"
                value: parent.value
                when: canvas
            }
        }

        Slider {
            id: brushRotationMaxSlider
            from: content.rotationMin
            to: content.rotationMax
            Layout.fillWidth: true
            value: canvas ? context.brushRotationMax : 1

            ToolTip.text: qsTr("Brush maximum rotation")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "brushRotationMax"
                value: parent.value
                when: canvas
            }
        }

        Label {
            text: brushRotationMinSlider.value.toFixed(0) + "\u00b0" + " to " + brushRotationMaxSlider.value.toFixed(0) + "\u00b0"
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        MenuSeparator {
            Layout.columnSpan: 5
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Hardness")
        }

        ComboBox {
            id: hardnessComboBox
            model: content.scalingModeStrings
            currentIndex: canvas ? context.brushHardnessMode : 0

            Binding {
                target: context
                property: "brushHardnessMode"
                value: hardnessComboBox.currentIndex // why parent.currentIndex not work?
                when: canvas
            }
        }

        Slider {
            id: brushHardnessMinSlider
            from: content.pressureMin
            to: content.pressureMax
            Layout.fillWidth: true
            value: canvas ? context.brushHardnessMin : 1

            ToolTip.text: qsTr("Brush minimum hardness")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "brushHardnessMin"
                value: parent.value
                when: canvas
            }
        }

        Slider {
            id: brushHardnessMaxSlider
            from: content.pressureMin
            to: content.pressureMax
            Layout.fillWidth: true
            value: canvas ? context.brushHardnessMax : 1

            ToolTip.text: qsTr("Brush maximum hardness")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "brushHardnessMax"
                value: parent.value
                when: canvas
            }
        }

        Label {
            text: brushHardnessMinSlider.value.toFixed(3) + " to " + brushHardnessMaxSlider.value.toFixed(3)
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Opacity")
        }

        ComboBox {
            id: opacityComboBox
            model: content.scalingModeStrings
            currentIndex: canvas ? context.brushOpacityMode : 0

            Binding {
                target: context
                property: "brushOpacityMode"
                value: opacityComboBox.currentIndex // why parent.currentIndex not work?
                when: canvas
            }
        }

        Slider {
            id: brushOpacityMinSlider
            from: content.pressureMin
            to: content.pressureMax
            Layout.fillWidth: true
            value: canvas ? context.brushOpacityMin : 1

            ToolTip.text: qsTr("Brush minimum opacity")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "brushOpacityMin"
                value: parent.value
                when: canvas
            }
        }

        Slider {
            id: brushOpacityMaxSlider
            from: content.pressureMin
            to: content.pressureMax
            Layout.fillWidth: true
            value: canvas ? context.brushOpacityMax : 1

            ToolTip.text: qsTr("Brush maximum opacity")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "brushOpacityMax"
                value: parent.value
                when: canvas
            }
        }

        Label {
            text: brushOpacityMinSlider.value.toFixed(3) + " to " + brushOpacityMaxSlider.value.toFixed(3)
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        MenuSeparator {
            Layout.columnSpan: 5
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Proportional Spacing")
        }

        ComboBox {
            id: proportionalSpacingComboBox
            model: content.scalingModeStrings
            currentIndex: canvas ? context.dynamicsProportionalSpacingMode : 0

            Binding {
                target: context
                property: "dynamicsProportionalSpacingMode"
                value: proportionalSpacingComboBox.currentIndex // why parent.currentIndex not work?
                when: canvas
            }
        }

        Slider {
            id: proportionalSpacingMinSlider
            from: content.pressureMin
            to: content.pressureMax
            Layout.fillWidth: true
            value: canvas ? context.dynamicsProportionalSpacingMin : 1

            ToolTip.text: qsTr("Brush minimum proportional spacing")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "dynamicsProportionalSpacingMin"
                value: parent.value
                when: canvas
            }
        }

        Slider {
            id: proportionalSpacingMaxSlider
            from: content.pressureMin
            to: content.pressureMax
            Layout.fillWidth: true
            value: canvas ? context.dynamicsProportionalSpacingMax : 1

            ToolTip.text: qsTr("Brush maximum proportional spacing")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "dynamicsProportionalSpacingMax"
                value: parent.value
                when: canvas
            }
        }

        Label {
            text: proportionalSpacingMinSlider.value.toFixed(3) + " to " + proportionalSpacingMaxSlider.value.toFixed(3)
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Absolute Spacing")
        }

        ComboBox {
            id: absoluteSpacingComboBox
            model: content.scalingModeStrings
            currentIndex: canvas ? context.dynamicsAbsoluteSpacingMode : 0

            Binding {
                target: context
                property: "dynamicsAbsoluteSpacingMode"
                value: absoluteSpacingComboBox.currentIndex // why parent.currentIndex not work?
                when: canvas
            }
        }

        Slider {
            id: absoluteSpacingMinSlider
            from: content.pressureMin
            to: content.pressureMax
            Layout.fillWidth: true
            value: canvas ? context.dynamicsAbsoluteSpacingMin : 1

            ToolTip.text: qsTr("Brush minimum absolute spacing")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "dynamicsAbsoluteSpacingMin"
                value: parent.value
                when: canvas
            }
        }

        Slider {
            id: absoluteSpacingMaxSlider
            from: content.pressureMin
            to: content.pressureMax
            Layout.fillWidth: true
            value: canvas ? context.dynamicsAbsoluteSpacingMax : 1

            ToolTip.text: qsTr("Brush maximum absolute spacing")
            ToolTip.visible: hovered

            Binding {
                target: context
                property: "dynamicsAbsoluteSpacingMax"
                value: parent.value
                when: canvas
            }
        }

        Label {
            text: absoluteSpacingMinSlider.value.toFixed(3) + " to " + absoluteSpacingMaxSlider.value.toFixed(3)
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }
    }
}
