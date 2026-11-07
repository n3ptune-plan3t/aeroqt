import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.TabButton {
    id: control
    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             contentItem.implicitHeight + topPadding + bottomPadding)
    leftPadding: 12; rightPadding: 12; topPadding: 5; bottomPadding: 5

    background: Rectangle {
        implicitHeight: 30
        radius: 5
        // selected tab = the lit glass one; the others sit slightly darker and lift on hover
        color: control.checked ? control.palette.button
             : control.hovered ? Qt.darker(control.palette.button, 1.02)
             : Qt.darker(control.palette.button, 1.07)
        opacity: control.enabled ? 1 : 0.6
        border.color: control.visualFocus ? control.palette.highlight : control.palette.dark
        border.width: control.visualFocus ? 2 : 1
        Rectangle { // gloss: white top half
            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
            height: parent.height * 0.48
            radius: 5
            color: Qt.rgba(1, 1, 1, control.checked ? 0.28 : 0.12)
        }
        Rectangle { // accent glow along the bottom of the selected tab
            visible: control.checked
            anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
            height: 2
            color: control.palette.highlight
        }
    }
    contentItem: Text {
        text: control.text
        font: control.font
        color: control.palette.buttonText
        opacity: control.enabled ? 1 : 0.5
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
}
