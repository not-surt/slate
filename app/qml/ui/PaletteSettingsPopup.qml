import QtQuick 2.11
import QtQuick.Controls 2.4
import App 1.0

Menu {
    id: root

    property alias swatchScale: swatchScaleSpinBox.value
    property alias columns: columnsSpinBox.value
    property alias fixedColumns: fixedColumnsMenuItem.checked
    property alias showIndices: showIndicesMenuItem.checked

    MenuItem {
        id: fixedColumnsMenuItem
        text: "Fixed Columns"
        checkable: true
        checked: fixedColumns
    }

    SpinBox {
        id: columnsSpinBox
        enabled: fixedColumnsMenuItem.checked
        value: columns
        from: 1
        to: 64
    }

    MenuSeparator {}

    MenuItem {
        id: fixedScaleMenuItem
        text: "Fixed Scale"
        checkable: true
        checked: !fixedColumns
    }

    SpinBox {
        id: swatchScaleSpinBox
        enabled: fixedScaleMenuItem.checked
        value: swatchScale
        from: 1
        to: 16
    }

    MenuSeparator {}

    MenuItem {
        id: showIndicesMenuItem
        text: "Show Indices"
        checkable: true
        checked: showIndices
    }
}
