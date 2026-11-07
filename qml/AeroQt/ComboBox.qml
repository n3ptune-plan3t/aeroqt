import QtQuick
import QtQuick.Window
import QtQuick.Templates as T
import QtQuick.Controls.impl

T.ComboBox {
    id: control
    implicitWidth: 160
    implicitHeight: 30
    leftPadding: 8; rightPadding: 28; topPadding: 4; bottomPadding: 4
    background: Rectangle {
        radius: 6
        color: control.pressed || control.down ? Qt.lighter(control.palette.button, 1.06)
                : control.hovered ? Qt.lighter(control.palette.button, 1.04)
                : control.palette.button
        opacity: control.enabled ? 1 : 0.6
        border.color: control.visualFocus ? control.palette.highlight
                        : control.palette.dark
        border.width: control.visualFocus ? 2 : 1
        Rectangle {
            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
            height: parent.height * 0.48; radius: 6
            color: Qt.rgba(1, 1, 1, 0.28)
            visible: control.enabled && !control.pressed
        }
    }
    // Non-editable boxes show displayText (editText is empty for them); editable ones need a
    // TextField as contentItem, which T.ComboBox drives.
    contentItem: T.TextField {
        text: control.editable ? control.editText : control.displayText
        enabled: control.editable
        autoScroll: control.editable
        readOnly: control.down
        inputMethodHints: control.inputMethodHints
        validator: control.validator
        selectByMouse: control.selectTextByMouse
        font: control.font
        color: control.palette.buttonText
        selectionColor: control.palette.highlight
        selectedTextColor: control.palette.highlightedText
        opacity: control.enabled ? 1 : 0.5
        verticalAlignment: Text.AlignVCenter
    }
    delegate: T.ItemDelegate {
        width: ListView.view ? ListView.view.width : control.width
        implicitHeight: 28
        leftPadding: 8; rightPadding: 8
        highlighted: control.highlightedIndex === index
        hoverEnabled: control.hoverEnabled
        text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole]
                                                                : model[control.textRole])
                               : modelData
        contentItem: Text {
            text: parent.text
            font: control.font
            color: parent.highlighted ? control.palette.highlightedText : control.palette.text
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            radius: 3
            color: parent.highlighted ? control.palette.highlight : "transparent"
        }
    }
    indicator: Text {
        x: control.width - width - 8
        y: (control.height - height) / 2
        text: "▾"
        color: control.palette.buttonText
        opacity: control.enabled ? 1 : 0.5
        rotation: control.popup.visible ? 180 : 0
        transformOrigin: Item.Center
        Behavior on rotation { NumberAnimation { duration: 120 } }
    }
    popup: T.Popup {
        y: control.height + 2
        width: control.width
        height: Math.min(contentItem.implicitHeight + verticalPadding * 2,
                         control.Window.height - topMargin - bottomMargin)
        topMargin: 6; bottomMargin: 6
        padding: 2
        contentItem: ListView {
            implicitHeight: contentHeight
            model: control.delegateModel
            currentIndex: control.highlightedIndex
            highlightMoveDuration: 0
            clip: true
            T.ScrollBar.vertical: ScrollBar { }
        }
        background: Rectangle {
            radius: 4
            color: control.palette.base
            border.color: control.palette.dark
        }
    }
}
