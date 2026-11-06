import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.SpinBox {
    id: control
    implicitWidth: 110
    implicitHeight: 30
    leftPadding: 8; rightPadding: 24; topPadding: 4; bottomPadding: 4
    editable: true
    background: Rectangle {
        radius: 5
        color: control.palette.base
        opacity: control.enabled ? 1 : 0.6
        border.color: control.visualFocus ? control.palette.highlight
                        : control.palette.dark
        border.width: control.visualFocus ? 2 : 1
    }
    contentItem: Text {
        text: control.textFromValue(control.value, control.locale)
        font: control.font
        color: control.palette.text
        opacity: control.enabled ? 1 : 0.5
        horizontalAlignment: Qt.AlignRight
        verticalAlignment: Text.AlignVCenter
    }
    upIndicator: Rectangle {
        x: control.width - width - 2; y: 2
        implicitWidth: 18; implicitHeight: (control.height - 4) / 2
        radius: 2
        color: control.hovered ? Qt.lighter(control.palette.button, 1.08) : control.palette.button
        opacity: control.enabled ? 1 : 0.6
        border.color: control.palette.dark
        Text { anchors.centerIn: parent; text: "▴"; color: control.palette.buttonText }
    }
    downIndicator: Rectangle {
        x: control.width - width - 2; y: control.height - height - 2
        implicitWidth: 18; implicitHeight: (control.height - 4) / 2
        radius: 2
        color: control.hovered ? Qt.lighter(control.palette.button, 1.08) : control.palette.button
        opacity: control.enabled ? 1 : 0.6
        border.color: control.palette.dark
        Text { anchors.centerIn: parent; text: "▾"; color: control.palette.buttonText }
    }
}
