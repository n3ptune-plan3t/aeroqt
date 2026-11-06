import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.TextArea {
    id: control
    implicitWidth: 200
    implicitHeight: 80
    leftPadding: 8; rightPadding: 8; topPadding: 6; bottomPadding: 6
    color: control.palette.text
    selectionColor: control.palette.highlight
    selectedTextColor: control.palette.highlightedText
    background: Rectangle {
        radius: 5
        color: control.palette.base
        border.color: control.activeFocus ? control.palette.highlight : control.palette.dark
        border.width: control.activeFocus ? 2 : 1
    }
}
