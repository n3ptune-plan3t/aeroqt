import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.TextField {
    id: control
    implicitWidth: 160
    implicitHeight: 30
    leftPadding: 8; rightPadding: 8; topPadding: 4; bottomPadding: 4
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
