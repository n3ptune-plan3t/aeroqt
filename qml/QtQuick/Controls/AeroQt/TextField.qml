import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.TextField {
    id: control
    implicitWidth: 160
    implicitHeight: 30
    leftPadding: 8; rightPadding: 8; topPadding: 4; bottomPadding: 4
    color: control.palette.text
    placeholderTextColor: Qt.alpha(control.palette.text, 0.55)
    selectionColor: control.palette.highlight
    selectedTextColor: control.palette.highlightedText
    background: Rectangle {
        radius: 5
        color: control.palette.base
        opacity: control.enabled ? 1 : 0.6
        border.color: control.visualFocus ? control.palette.highlight : control.palette.dark
        border.width: control.visualFocus ? 2 : 1
    }
}
