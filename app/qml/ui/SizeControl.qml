import QtQuick 2.6
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11

import App 1.0

Control {
    id: root

    property size value
    property size from: Qt.size(1, 1)
    property size to: Qt.size(100, 100)
    property size stepSize: Qt.size(1, 1)

    padding: 0

    contentItem:  GridLayout {
        id: content
        columns: 3

        property size value
        property real aspectRatio
        property alias aspectRatioLocked: lockAspectRatioButton.checked

        onValueChanged: {
            if (widthSpinBox.value !== value.width) {
                widthSpinBox.value = value.width
            }
            if (heightSpinBox.value !== value.height) {
                heightSpinBox.value = value.height
            }
        }

        function updateAspectRatio() {
            if (aspectRatioLocked) aspectRatio = value.width / value.height
        }

        onAspectRatioLockedChanged: updateAspectRatio()
        Component.onCompleted: updateAspectRatio()

        Item {
            id: lockAspectRatioItem
            Layout.rowSpan: 2
            Layout.fillHeight: true
            implicitWidth: lockAspectRatioButton.implicitWidth

            Rectangle {
                width: 1
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: lockAspectRatioItem.height / 4
                anchors.bottom: parent.verticalCenter
                anchors.bottomMargin: (lockAspectRatioButton.height - 16) / 2 + 4

                Rectangle {
                    width: lockAspectRatioButton.width / 2 - 6
                    height: 1
                    anchors.left: parent.left
                }
            }

            Rectangle {
                width: 1
                anchors.top: parent.verticalCenter
                anchors.topMargin: (lockAspectRatioButton.height - 16) / 2 + 4
                anchors.bottom: parent.bottom
                anchors.bottomMargin: lockAspectRatioItem.height / 4
                anchors.horizontalCenter: parent.horizontalCenter

                Rectangle {
                    width: lockAspectRatioButton.width / 2 - 6
                    height: 1
                    anchors.left: parent.left
                    anchors.bottom: parent.bottom
                }
            }

            IconToolButton {
                id: lockAspectRatioButton
                objectName: "preserveAspectRatioButton"
                text: "\uf0c1"
                checked: true
                topPadding: 0
                bottomPadding: 0
                anchors.verticalCenter: parent.verticalCenter
                focusPolicy: Qt.StrongFocus

                ToolTip.text: qsTr("Preserve aspect ratio")
                ToolTip.visible: hovered
            }
        }

        Label {
            text: qsTr("Width")
        }

        SpinBox {
            id: widthSpinBox
            from: root.from.width
            to: root.to.width
            stepSize: root.stepSize.width
            editable: true
            focus: true
            Layout.fillWidth: true

            onValueChanged: {
                if (content.value.width !== value) {
                    if (content.aspectRatioLocked) {
                        heightSpinBox.value = Math.round(value / content.aspectRatio)
                    }
                    content.value = Qt.size(value, heightSpinBox.value)
                }
            }
        }

        Label {
            text: qsTr("Height")
        }

        SpinBox {
            id: heightSpinBox
            from: root.from.height
            to: root.to.height
            stepSize: root.stepSize.height
            editable: true
            Layout.fillWidth: true

            onValueChanged: {
                if (content.value.height !== value) {
                    if (content.aspectRatioLocked) {
                        widthSpinBox.value = Math.round(value * content.aspectRatio)
                    }
                    content.value = Qt.size(widthSpinBox.value, value)
                }
            }
        }
    }

    Binding {
        target: content
        property: "value"
        value: root.value
    }

    Binding {
        target: root
        property: "value"
        value: content.value
    }
}
