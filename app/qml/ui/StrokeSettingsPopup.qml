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
            text: qsTr("Stroke Type")
        }

        ComboBox {
            id: modeComboBox
            Layout.columnSpan: 2
            Layout.fillWidth: true
            model: [
                "Spaced",
                "Counted",
                "Timed"
            ]
            currentIndex: canvas ? context.strokeMode : 0

            Binding {
                target: context
                property: "brushScalingMode"
                value: modeComboBox.currentIndex
                when: canvas
            }
        }

        Label {
            text: qsTr("Proportional Spacing")
        }

        Slider {
            id: proportionalSpacingSlider
            from: 0.0
            to: 1.0
            Layout.fillWidth: true
            value: canvas ? context.strokeProportionalSpacing : 0.0

            Binding {
                target: context
                property: "strokeProportionalSpacing"
                value: proportionalSpacingSlider.value
                when: canvas
            }
        }

        Label {
            text: proportionalSpacingSlider.value.toFixed(3)
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Absolute Spacing")
        }

        Slider {
            id: absoluteSpacingSlider
            from: 0.0
            to: 1.0
            Layout.fillWidth: true
            value: canvas ? context.strokeAbsoluteSpacing : 0.0

            Binding {
                target: context
                property: "strokeAbsoluteSpacing"
                value: absoluteSpacingSlider.value
                when: canvas
            }
        }

        Label {
            text: absoluteSpacingSlider.value.toFixed(3)
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
        }

        Label {
            text: qsTr("Brush Count")
        }

        SpinBox {
            id: countSpinBox
            from: 1
            to: 256
            stepSize: 1
            Layout.fillWidth: true
            Layout.columnSpan: 2
//            value: canvas ? canvas.toolSpacing : 1

//            Binding {
//                target: canvas
//                property: "toolSpacing"
//                value: absoluteSpacingSlider.value
//                when: canvas
//            }
        }

        Label {
            text: qsTr("Brush Rate")
        }

        SpinBox {
            id: timingSlider
            from: 1
            to: 256
            stepSize: 1
            Layout.fillWidth: true
            Layout.columnSpan: 2
//            value: canvas ? canvas.toolSpacing : 1

//            Binding {
//                target: canvas
//                property: "toolSpacing"
//                value: absoluteSpacingSlider.value
//                when: canvas
//            }
        }
    }
}
