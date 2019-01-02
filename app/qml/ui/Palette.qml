import QtQuick 2.0
import QtQml.Models 2.11
import QtGraphicalEffects 1.12
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import App 1.0

import "." as Ui

Page {
    id: root

    property real swatchScale: 2
    property int columns: 8
    property size swatchSize: Qt.size(32, 32)
    property bool fixedColumns: true
    property bool showIndices: false
    property alias currentIndex: gridView.currentIndex

    property Component delegate: null
    property AbstractItemModel model: null
//    readonly property ItemSelectionModel selection: ItemSelectionModel {
//        onCurrentChanged: {
//            console.log("onCurrentChanged", currentIndex, currentIndex.row)
//        }
//        onSelectionChanged: {
//            console.log("onSelectionChanged", selection)
//        }
//    }

//    onModelChanged: {
//        console.log(model)
//        selection.clear()
//        selection.model = Qt.binding(function () { return model })
//    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        GridView {
            id: gridView

            property alias showIndices: root.showIndices
            property real padding: 1
            cellWidth: root.fixedColumns ? /*Math.floor*/(width / root.columns) : root.swatchSize.width * root.swatchScale + 2 * padding
            property real swatchScale: (cellWidth - 2 * padding) / root.swatchSize.width
            cellHeight: /*Math.floor*/(root.swatchSize.height * swatchScale) + 2 * padding

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.minimumWidth: root.swatchSize.width
            Layout.minimumHeight: root.swatchSize.height

            model: root.model
            delegate: root.delegate

            boundsBehavior: GridView.StopAtBounds
            focus: true
            clip: true

            onCurrentIndexChanged: {
//                selection.select(model.index(currentIndex, 0), ItemSelectionModel.SelectCurrent)
//                selection.setCurrentIndex(model.index(currentIndex, 0), ItemSelectionModel.SelectCurrent)
//                console.log(selection.selectedIndexes)
//                console.log(selection.hasSelection)
            }

            highlightMoveDuration: 0
            highlight: Component {
                Item {
                    z: 2

                    Item {
                        x: -gridView.padding
                        y: -gridView.padding
                        width: gridView.cellWidth + 2 * gridView.padding
                        height: gridView.cellHeight + 2 * gridView.padding

                        Rectangle {
                            id: rect
                            anchors.fill: parent
                            color: "transparent"
                            border {
                                width: 2 * gridView.padding
                                color: Ui.CanvasColours.focusColour
                            }
                            radius: 2 * gridView.padding
                        }

                        DropShadow {
                            anchors.fill: parent
                            horizontalOffset: 1
                            verticalOffset: 1
                            radius: 2
                            samples: 9
                            color: Ui.CanvasColours.panelColour
                            source: rect
                        }
                    }
                }
            }

            ScrollBar.vertical: scrollBar
        }

        ScrollBar {
            id: scrollBar
            policy: ScrollBar.AlwaysOn
            Layout.fillHeight: true
        }
    }

    footer: ToolBar {
        Layout.fillWidth: true

        RowLayout {
            ToolButton {
                id: newToolButton
                text: "\uf067"
                font.family: "FontAwesome"
            }

            ToolButton {
                id: duplicateToolButton
                text: "\uf24d"
                font.family: "FontAwesome"
            }

            ToolSeparator {}

            ToolButton {
                id: deleteToolButton
                text: "\uf1f8"
                font.family: "FontAwesome"
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}
