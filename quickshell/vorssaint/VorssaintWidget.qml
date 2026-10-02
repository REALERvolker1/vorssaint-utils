pragma ComponentBehavior: Bound

import Quickshell
import QtQuick
import QtQuick.Controls

Item {
    id: root

    required property var screen

    implicitWidth: launcher.implicitWidth
    implicitHeight: launcher.implicitHeight

    Button {
        id: launcher
        text: "V"
        focusPolicy: Qt.NoFocus
        onClicked: popup.visible = !popup.visible
    }

    PopupWindow {
        id: popup

        anchor.item: launcher
        anchor.edges: Edges.Bottom | Edges.Right
        anchor.gravity: Edges.Bottom | Edges.Left
        anchor.adjustment: PopupAdjustment.All
        grabFocus: true

        implicitWidth: panel.preferredWidth
        implicitHeight: panel.preferredHeight
        visible: false
        color: "transparent"

        VorssaintPanel {
            id: panel
            anchors.fill: parent
            screen: root.screen
            onRequestClose: popup.visible = false
        }
    }
}
