pragma ComponentBehavior: Bound

import Quickshell
import Quickshell.Hyprland
import Quickshell.Io
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property var screen
    required property string backendCommand
    property string statusText: ""

    function validWorkspace(text) {
        return /^[1-9][0-9]{0,3}$/.test(text)
    }

    function openWorkspace() {
        const value = workspaceField.text.trim()
        if (!validWorkspace(value)) {
            statusText = "Workspace must be an integer from 1 to 9999."
            return
        }

        workspaceProcess.exec([
            backendCommand,
            "hypr",
            "open-workspace",
            screen.name,
            value
        ])
    }

    function moveActive() {
        const value = workspaceField.text.trim()
        if (!validWorkspace(value)) {
            statusText = "Workspace must be an integer from 1 to 9999."
            return
        }

        workspaceProcess.exec([
            backendCommand,
            "hypr",
            "move-active",
            value
        ])
    }

    Process {
        id: workspaceProcess

        stdout: StdioCollector {
            onStreamFinished: {
                try {
                    const result = JSON.parse(text)
                    root.statusText = result.ok
                        ? "Workspace action complete."
                        : (result.stderr || "Workspace action failed.")
                } catch (e) {
                    root.statusText = "Workspace action returned invalid output."
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 10

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: workspaceControls.implicitHeight + 24
            radius: 10
            color: "#1a1d24"

            ColumnLayout {
                id: workspaceControls
                anchors.fill: parent
                anchors.margins: 12

                Label {
                    text: "Workspace on " + root.screen.name
                    textFormat: Text.PlainText
                    color: "#f1f3f7"
                    font.bold: true
                }

                RowLayout {
                    Layout.fillWidth: true

                    TextField {
                        id: workspaceField
                        Layout.fillWidth: true
                        placeholderText: "Workspace number"
                        inputMethodHints: Qt.ImhDigitsOnly
                        validator: IntValidator { bottom: 1; top: 9999 }
                        onAccepted: root.openWorkspace()
                    }

                    Button {
                        text: "Open here"
                        enabled: root.validWorkspace(workspaceField.text.trim())
                        onClicked: root.openWorkspace()
                    }

                    Button {
                        text: "Move active window"
                        enabled: root.validWorkspace(workspaceField.text.trim())
                        onClicked: root.moveActive()
                    }
                }

                Label {
                    text: root.statusText
                    textFormat: Text.PlainText
                    visible: text.length > 0
                    color: "#aeb4c0"
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Hyprland windows"
                color: "#f1f3f7"
                font.bold: true
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "Refresh"
                onClicked: Hyprland.refreshToplevels()
            }
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 6
            model: Hyprland.toplevels

            delegate: Rectangle {
                id: row
                required property var modelData

                width: ListView.view.width
                height: 62
                radius: 9
                color: modelData.activated ? "#252a35" : "#1a1d24"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 10

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 2

                        Label {
                            Layout.fillWidth: true
                            text: row.modelData.title || "(untitled)"
                            textFormat: Text.PlainText
                            color: "#f1f3f7"
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            text: row.modelData.appId || "unknown application"
                            textFormat: Text.PlainText
                            color: "#858b98"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }
                    }

                    Button {
                        text: "Focus"
                        onClicked: row.modelData.activate()
                    }

                    Button {
                        text: "Close"
                        onClicked: row.modelData.close()
                    }
                }
            }
        }
    }
}
