import QtQuick
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.SpinBox {
    id: control
    implicitWidth: 110
    implicitHeight: 30
    leftPadding: 8 + (mirrored ? 24 : 0); rightPadding: 8 + (mirrored ? 0 : 24); topPadding: 4; bottomPadding: 4
    editable: true
    background: Rectangle {
        radius: 5
        color: control.palette.base
        opacity: control.enabled ? 1 : 0.6
        border.color: control.visualFocus ? control.palette.highlight
                        : control.palette.dark
        border.width: control.visualFocus ? 2 : 1
    }
    // T.SpinBox drives an editable box through a TextInput contentItem.
    contentItem: TextInput {
        text: control.displayText
        font: control.font
        color: control.palette.text
        selectionColor: control.palette.highlight
        selectedTextColor: control.palette.highlightedText
        opacity: control.enabled ? 1 : 0.5
        horizontalAlignment: Qt.AlignRight
        verticalAlignment: Text.AlignVCenter
        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: control.inputMethodHints
        selectByMouse: true
    }
    up.indicator: Rectangle {
        x: control.mirrored ? 2 : control.width - width - 2; y: 2
        implicitWidth: 18; implicitHeight: (control.height - 4) / 2
        radius: 2
        color: control.up.pressed ? Qt.darker(control.palette.button, 1.12)
             : control.up.hovered ? Qt.lighter(control.palette.button, 1.08)
             : control.palette.button
        opacity: control.enabled && control.value < control.to ? 1 : 0.5
        border.color: control.palette.dark
        Text { anchors.centerIn: parent; text: "\u25B4"; color: control.palette.buttonText }
    }
    down.indicator: Rectangle {
        x: control.mirrored ? 2 : control.width - width - 2; y: control.height - height - 2
        implicitWidth: 18; implicitHeight: (control.height - 4) / 2
        radius: 2
        color: control.down.pressed ? Qt.darker(control.palette.button, 1.12)
             : control.down.hovered ? Qt.lighter(control.palette.button, 1.08)
             : control.palette.button
        opacity: control.enabled && control.value > control.from ? 1 : 0.5
        border.color: control.palette.dark
        Text { anchors.centerIn: parent; text: "\u25BE"; color: control.palette.buttonText }
    }
}
