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
        border.color: control.activeFocus ? control.palette.highlight : control.palette.dark
    }
    contentItem: Text {
        text: control.textFromValue(control.value, control.locale)
        font: control.font
        color: control.palette.text
        horizontalAlignment: Qt.AlignRight
        verticalAlignment: Text.AlignVCenter
    }
    upIndicator: Rectangle {
        x: control.width - width - 2; y: 2
        implicitWidth: 18; implicitHeight: (control.height - 4) / 2
        radius: 2
        color: control.palette.button
        border.color: control.palette.dark
        Text { anchors.centerIn: parent; text: "▴"; color: control.palette.buttonText }
    }
    downIndicator: Rectangle {
        x: control.width - width - 2; y: control.height - height - 2
        implicitWidth: 18; implicitHeight: (control.height - 4) / 2
        radius: 2
        color: control.palette.button
        border.color: control.palette.dark
        Text { anchors.centerIn: parent; text: "▾"; color: control.palette.buttonText }
    }
}
