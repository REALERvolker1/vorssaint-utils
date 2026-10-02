pragma ComponentBehavior: Bound

import Quickshell.Io
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property string backendCommand
    property string defaultScope: "all"
    property int resultLimit: 100
    property bool compactRows: false

    property var units: []
    property string errorText: ""
    property string actionStatus: ""

    function refresh() {
        const scope = scopeBox.currentValue || "all"
        listProcess.exec([
            backendCommand,
            "services",
            "list",
            scope,
            searchField.text,
            String(resultLimit)
        ])
    }

    function serviceAction(scope, action, unit) {
        actionStatus = action + " " + unit + "…"
        actionProcess.exec([
            backendCommand,
            "services",
            "action",
            scope,
            action,
            unit
        ])
    }

    Process {
        id: listProcess

        stdout: StdioCollector {
            onStreamFinished: {
                try {
                    const result = JSON.parse(text)
                    root.units = result.units || []
                    root.errorText = result.error || ""
                } catch (e) {
                    root.units = []
                    root.errorText = "Could not parse systemd service list."
                }
            }
        }
    }

    Process {
        id: actionProcess

        stdout: StdioCollector {
            onStreamFinished: {
                try {
                    const result = JSON.parse(text)
                    root.actionStatus = result.ok
                        ? "Service action complete."
                        : (result.stderr || "Service action failed.")
                } catch (e) {
                    root.actionStatus = "Service action returned invalid output."
                }
                root.refresh()
            }
        }
    }

    Timer {
        id: searchDebounce
        interval: 120
        repeat: false
        onTriggered: root.refresh()
    }

    Component.onCompleted: {
        const wanted = root.defaultScope
        if (wanted === "user") scopeBox.currentIndex = 1
        else if (wanted === "system") scopeBox.currentIndex = 2
        else scopeBox.currentIndex = 0
        root.refresh()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        RowLayout {
            Layout.fillWidth: true

            TextField {
                id: searchField
                Layout.fillWidth: true
                placeholderText: "Fuzzy-search services"
                onTextChanged: searchDebounce.restart()
            }

            ComboBox {
                id: scopeBox
                textRole: "text"
                valueRole: "value"
                model: [
                    { text: "All", value: "all" },
                    { text: "User", value: "user" },
                    { text: "System", value: "system" }
                ]
                onActivated: root.refresh()
            }

            Button {
                text: "Refresh"
                onClicked: root.refresh()
            }
        }

        Label {
            Layout.fillWidth: true
            visible: root.actionStatus.length > 0
            text: root.actionStatus
            color: "#aeb4c0"
            elide: Text.ElideRight
        }

        Label {
            Layout.fillWidth: true
            visible: root.errorText.length > 0
            text: root.errorText
            color: "#ff8b8b"
            wrapMode: Text.Wrap
        }

        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            spacing: 5
            model: root.units

            delegate: Rectangle {
                id: serviceRow

                required property var modelData

                width: ListView.view.width
                height: root.compactRows ? 52 : 68
                radius: 9
                color: "#1a1d24"

                RowLayout {
                    anchors.fill: parent
                    anchors.margins: 9

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 1

                        Label {
                            Layout.fillWidth: true
                            text: serviceRow.modelData.name
                            color: "#f1f3f7"
                            font.bold: true
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            visible: !root.compactRows
                            text: serviceRow.modelData.description || "No description"
                            color: "#858b98"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }

                        Label {
                            Layout.fillWidth: true
                            text: serviceRow.modelData.scope
                                  + " · "
                                  + (serviceRow.modelData.activeState || "inactive")
                                  + " · "
                                  + (serviceRow.modelData.fileState || "unknown")
                            color: serviceRow.modelData.activeState === "active"
                                   ? "#8bd5a5" : "#aeb4c0"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                        }
                    }

                    Button {
                        text: serviceRow.modelData.activeState === "active"
                              ? "Restart" : "Start"
                        onClicked: root.serviceAction(
                            serviceRow.modelData.scope,
                            serviceRow.modelData.activeState === "active"
                                ? "restart" : "start",
                            serviceRow.modelData.name)
                    }

                    Button {
                        text: "Stop"
                        enabled: serviceRow.modelData.activeState === "active"
                        onClicked: root.serviceAction(
                            serviceRow.modelData.scope,
                            "stop",
                            serviceRow.modelData.name)
                    }

                    Button {
                        text: serviceRow.modelData.fileState === "enabled"
                              ? "Disable" : "Enable"
                        onClicked: root.serviceAction(
                            serviceRow.modelData.scope,
                            serviceRow.modelData.fileState === "enabled"
                                ? "disable" : "enable",
                            serviceRow.modelData.name)
                    }
                }
            }
        }

        Label {
            Layout.fillWidth: true
            text: "System-scope mutations authenticate through polkit/pkexec; user services run without elevation."
            color: "#686e79"
            font.pixelSize: 10
            wrapMode: Text.Wrap
        }
    }
}
