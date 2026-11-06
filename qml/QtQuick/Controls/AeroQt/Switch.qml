import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.Switch {
    id: control
    implicitWidth: Math.max(background ? background.implicitWidth : 0, contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0, contentItem.implicitHeight + topPadding + bottomPadding)
    leftPadding: indicator.width + spacing
    spacing: 6
    indicator: Rectangle {
        implicitWidth: 40; implicitHeight: 20
        radius: 10
        x: control.leftPadding - width - control.spacing
        y: (control.height - height) / 2
        color: control.checked ? (control.hovered ? Qt.lighter(control.palette.highlight, 1.1)
                                     : control.palette.highlight)
                : control.palette.button
        opacity: control.enabled ? 1 : 0.6
        border.color: control.visualFocus ? control.palette.highlight
                        : control.palette.dark
        border.width: control.visualFocus ? 2 : 1
        Rectangle {
            width: 16; height: 16; radius: 8
            x: control.checked ? parent.width - width - 2 : 2
            y: 2
            color: control.checked ? control.palette.highlightedText : control.palette.button
            border.color: control.palette.dark
            Behavior on x { NumberAnimation { duration: 120 } }
        }
    }
    contentItem: Text {
        text: control.text
        font: control.font
        color: control.palette.text
        opacity: control.enabled ? 1 : 0.5
        verticalAlignment: Text.AlignVCenter
    }
}
