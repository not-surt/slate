import QtQuick 2.0
import App 1.0

PaletteDelegate {
    ImagePainter {
        x: parent.view.padding
        y: parent.view.padding
        width: parent.width - 2 * parent.view.padding
        height: parent.height - 2 * parent.view.padding
        image: tileImage
    }
}
