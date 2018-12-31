import QtQuick 2.11
import QtGraphicalEffects 1.12
import App 1.0

import "." as Ui

Item  {
    property GridView view: parent.parent

    width: view.cellWidth
    height: view.cellHeight

    Text {
        id: indexText
        z: 3
        visible: view.showIndices
        anchors {
            top: parent.top
            left: parent.left
            margins: view.padding * 2
        }
        color: Ui.CanvasColours.focusColour
        font.pixelSize: parent.height / 4
//        fontSizeMode: Text.Fit
//        topPadding: 0
//        lineHeightMode: Text.FixedHeight
//        lineHeight: font.pixelSize
//        verticalAlignment: Text.AlignTop
        text: index
    }

    DropShadow {
        z: 2
        visible: view.showIndices
        anchors.fill: indexText
        horizontalOffset: 1
        verticalOffset: 1
        radius: 2
        samples: 9
        color: Ui.CanvasColours.panelColour
        source: indexText
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            view.focus = true
            view.currentIndex = index
        }
    }
}
