import QtQuick 2.6
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

import App 1.0

import "." as Ui

Popup {
    objectName: "brushSettingsPopup"
    focus: true

    property ImageCanvas canvas

    contentItem: ColumnLayout {

        GridLayout {
            columns: 3

            Label {
                text: qsTr("Type")
            }

            ComboBox {
                id: brushTypeComboBox
                Layout.fillWidth: true
                currentIndex: canvas.brushType

                model: ["Square", "Circle", "Image"]

                Binding {
                    target: canvas
                    property: "brushType"
                    value: brushTypeComboBox.currentIndex
                    when: canvas
                }
            }

            Ui.IconToolButton {
                id: toolSingleColourButton
                objectName: "toolSingleColourButton"
                text: "\uf1fc"
                hoverEnabled: true
                focusPolicy: Qt.NoFocus

                checked: canvas.toolSingleColour
                checkable: true
                enabled: canvas.brushType == Brush.ImageType

                ToolTip.text: qsTr("Draw image brush with current colour")
                ToolTip.visible: hovered

                Binding {
                    target: canvas
                    property: "toolSingleColour"
                    value: toolSingleColourButton.checked
                    when: canvas
                }
            }

            MenuSeparator {
                Layout.columnSpan: 3
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Spacing")
            }

            ColumnLayout {
                Layout.fillWidth: true
                Layout.columnSpan: 2

                Slider {
                    id: toolSpacingSlider
                    objectName: "toolSpacingSlider"
                    from: 0.0
                    to: 1.0
                    Layout.fillWidth: true
                    value: canvas ? canvas.toolSpacing : 1

                    ToolTip.text: qsTr("Brush spacing")
                    ToolTip.visible: hovered

                    Binding {
                        target: canvas
                        property: "toolSpacing"
                        value: toolSpacingSlider.value
                        when: canvas
                    }
                }

                Label {
                    text: toolSpacingSlider.value.toFixed(4)
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                }
            }

            MenuSeparator {
                Layout.columnSpan: 3
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Size")
            }

            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: canvas && canvas.toolSizeUsePressure

                    Slider {
                        id: lowerToolSizeSlider
                        objectName: "toolSizeSlider"
                        from: 0
                        to: canvas ? canvas.maxToolSize : 1
                        stepSize: 1
                        Layout.fillWidth: true
                        value: canvas ? canvas.lowerToolSize : 1

                        Binding {
                            target: canvas
                            property: "lowerToolSize"
                            value: lowerToolSizeSlider.value
                            when: canvas
                        }
                    }

                    SpinBox {
                        id: lowerToolSizeSpinBox
                        editable: true
                        from: lowerToolSizeSlider.from
                        to: lowerToolSizeSlider.to
                        stepSize: lowerToolSizeSlider.stepSize
                        Layout.fillWidth: true
                        value: canvas ? canvas.lowerToolSize : 1

                        Binding {
                            target: canvas
                            property: "lowerToolSize"
                            value: lowerToolSizeSpinBox.value
                            when: canvas
                        }

                        ToolTip.text: qsTr("Lower brush size")
                        ToolTip.visible: hovered
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    Slider {
                        id: upperToolSizeSlider
                        objectName: "upperToolSizeSlider"
                        from: 1
                        to: canvas ? canvas.maxToolSize : 1
                        stepSize: 1
                        Layout.fillWidth: true
                        value: canvas ? canvas.upperToolSize : 1

                        Binding {
                            target: canvas
                            property: "upperToolSize"
                            value: upperToolSizeSlider.value
                            when: canvas
                        }
                    }

                    SpinBox {
                        id: upperToolSizeSpinBox
                        editable: true
                        from: upperToolSizeSlider.from
                        to: upperToolSizeSlider.to
                        stepSize: upperToolSizeSlider.stepSize
                        Layout.fillWidth: true
                        value: canvas ? canvas.upperToolSize : 1

                        Binding {
                            target: canvas
                            property: "upperToolSize"
                            value: upperToolSizeSpinBox.value
                            when: canvas
                        }

                        ToolTip.text: canvas && canvas.toolSizeUsePressure ? qsTr("Upper brush size") : qsTr("Brush size")
                        ToolTip.visible: hovered
                    }
                }
            }

            Ui.IconToolButton {
                id: sizeUsePressureButton
                objectName: "sizeUsePressureButton"
                text: "\uf040"
                hoverEnabled: true
                focusPolicy: Qt.NoFocus

                checked: canvas && canvas.toolSizeUsePressure == true
                checkable: true

                ToolTip.text: qsTr("Enable tablet stylus pressure sensitivity for brush size")
                ToolTip.visible: hovered

                Binding {
                    target: canvas
                    property: "toolSizeUsePressure"
                    value: sizeUsePressureButton.checked
                    when: canvas
                }

            }

            MenuSeparator {
                Layout.columnSpan: 3
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Opacity")
            }

            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: canvas && canvas.toolOpacityUsePressure

                    Slider {
                        id: lowerToolOpacitySlider
                        objectName: "toolOpacitySlider"
                        from: 0.0
                        to: 1.0
                        Layout.fillWidth: true
                        value: canvas ? canvas.lowerToolOpacity : 1

                        ToolTip.text: qsTr("Lower brush opacity")
                        ToolTip.visible: hovered

                        Binding {
                            target: canvas
                            property: "lowerToolOpacity"
                            value: lowerToolOpacitySlider.value
                            when: canvas
                        }
                    }

                    Label {
                        text: lowerToolOpacitySlider.value.toFixed(4)
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    Slider {
                        id: upperToolOpacitySlider
                        objectName: "upperToolOpacitySlider"
                        from: 0.0
                        to: 1.0
                        Layout.fillWidth: true
                        value: canvas ? canvas.upperToolOpacity : 1

                        ToolTip.text: canvas && canvas.toolOpacityUsePressure ? qsTr("Upper brush opacity") : qsTr("Brush opacity")
                        ToolTip.visible: hovered

                        Binding {
                            target: canvas
                            property: "upperToolOpacity"
                            value: upperToolOpacitySlider.value
                            when: canvas
                        }
                    }

                    Label {
                        text: upperToolOpacitySlider.value.toFixed(4)
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                }
            }

            Ui.IconToolButton {
                id: opacityUsePressureButton
                objectName: "opacityUsePressureButton"
                text: "\uf040"
                hoverEnabled: true
                focusPolicy: Qt.NoFocus

                checked: canvas && canvas.toolOpacityUsePressure == true
                checkable: true

                ToolTip.text: qsTr("Enable tablet stylus pressure sensitivity for brush opacity")
                ToolTip.visible: hovered

                Binding {
                    target: canvas
                    property: "toolOpacityUsePressure"
                    value: opacityUsePressureButton.checked
                    when: canvas
                }
            }

            MenuSeparator {
                Layout.columnSpan: 3
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Hardness")
            }

            RowLayout {
                Layout.fillWidth: true

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: canvas && canvas.toolHardnessUsePressure

                    Slider {
                        id: lowerToolHardnessSlider
                        objectName: "toolHardnessSlider"
                        from: 0.0
                        to: 1.0
                        Layout.fillWidth: true
                        value: canvas ? canvas.lowerToolHardness : 1

                        ToolTip.text: qsTr("Lower brush hardness")
                        ToolTip.visible: hovered

                        Binding {
                            target: canvas
                            property: "lowerToolHardness"
                            value: lowerToolHardnessSlider.value
                            when: canvas
                        }
                    }

                    Label {
                        text: lowerToolHardnessSlider.value.toFixed(4)
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true

                    Slider {
                        id: upperToolHardnessSlider
                        objectName: "upperToolHardnessSlider"
                        from: 0.0
                        to: 1.0
                        Layout.fillWidth: true
                        value: canvas ? canvas.upperToolHardness : 1

                        ToolTip.text: canvas && canvas.toolHardnessUsePressure ? qsTr("Upper brush hardness") : qsTr("Brush hardness")
                        ToolTip.visible: hovered

                        Binding {
                            target: canvas
                            property: "upperToolHardness"
                            value: upperToolHardnessSlider.value
                            when: canvas
                        }
                    }

                    Label {
                        text: upperToolHardnessSlider.value.toFixed(4)
                        horizontalAlignment: Text.AlignHCenter
                        Layout.fillWidth: true
                    }
                }
            }

            Ui.IconToolButton {
                id: hardnessUsePressureButton
                objectName: "hardnessUsePressureButton"
                text: "\uf040"
                hoverEnabled: true
                focusPolicy: Qt.NoFocus

                checked: canvas && canvas.toolHardnessUsePressure == true
                checkable: true

                ToolTip.text: qsTr("Enable tablet stylus pressure sensitivity for brush hardness")
                ToolTip.visible: hovered

                Binding {
                    target: canvas
                    property: "toolHardnessUsePressure"
                    value: hardnessUsePressureButton.checked
                    when: canvas
                }
            }
        }
    }
}
