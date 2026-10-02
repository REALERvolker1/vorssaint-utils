pragma ComponentBehavior: Bound

import Quickshell.Io
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property string backendCommand
    property int refreshMs: 2000
    property var metrics: ({})
    property string errorText: ""

    function fmtBytes(value) {
        if (!value || value <= 0) return "0 B"
        const units = ["B", "KiB", "MiB", "GiB", "TiB"]
        let i = 0
        let n = value
        while (n >= 1024 && i < units.length - 1) {
            n /= 1024
            ++i
        }
        return n.toFixed(i >= 3 ? 1 : 0) + " " + units[i]
    }

    function refresh() {
        if (!metricsProcess.running)
            metricsProcess.exec([backendCommand, "metrics"])
    }

    Process {
        id: metricsProcess
        stdout: StdioCollector {
            onStreamFinished: {
                try {
                    root.metrics = JSON.parse(text)
                    root.errorText = ""
                } catch (e) {
                    root.errorText = "Could not parse system metrics"
                }
            }
        }
    }

    Timer {
        interval: Math.max(root.refreshMs, 500)
        repeat: true
        running: true
        triggeredOnStart: true
        onTriggered: root.refresh()
    }

    Flickable {
        anchors.fill: parent
        contentHeight: content.implicitHeight
        clip: true

        ColumnLayout {
            id: content
            width: parent.width
            spacing: 12

            Label {
                text: (root.metrics.hostname || "Linux") + "  ·  " + (root.metrics.kernel || "")
                color: "#c6cad2"
                Layout.fillWidth: true
                elide: Text.ElideRight
            }

            GridLayout {
                columns: 2
                Layout.fillWidth: true
                rowSpacing: 10
                columnSpacing: 10

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    radius: 10
                    color: "#1a1d24"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 6
                        Label { text: "CPU"; color: "#858b98" }
                        Label {
                            text: Number(root.metrics.cpuPercent || 0).toFixed(1) + "%"
                            color: "#f1f3f7"
                            font.pixelSize: 28
                            font.bold: true
                        }
                        Label {
                            text: "Load " + Number(root.metrics.load?.one || 0).toFixed(2)
                            color: "#aeb4c0"
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    radius: 10
                    color: "#1a1d24"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 6
                        Label { text: "Memory"; color: "#858b98" }
                        Label {
                            text: Number(root.metrics.memory?.usedPercent || 0).toFixed(1) + "%"
                            color: "#f1f3f7"
                            font.pixelSize: 28
                            font.bold: true
                        }
                        Label {
                            text: root.fmtBytes(root.metrics.memory?.usedBytes || 0)
                                  + " / " + root.fmtBytes(root.metrics.memory?.totalBytes || 0)
                            color: "#aeb4c0"
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    radius: 10
                    color: "#1a1d24"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 6
                        Label { text: "Network totals"; color: "#858b98" }
                        Label {
                            text: "↓ " + root.fmtBytes(root.metrics.network?.rxBytes || 0)
                            color: "#f1f3f7"
                            font.pixelSize: 18
                        }
                        Label {
                            text: "↑ " + root.fmtBytes(root.metrics.network?.txBytes || 0)
                            color: "#aeb4c0"
                            font.pixelSize: 18
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 110
                    radius: 10
                    color: "#1a1d24"

                    Column {
                        anchors.fill: parent
                        anchors.margins: 14
                        spacing: 6
                        Label { text: "Root filesystem"; color: "#858b98" }
                        Label {
                            readonly property double total: root.metrics.disk?.totalBytes || 0
                            readonly property double avail: root.metrics.disk?.availableBytes || 0
                            text: root.fmtBytes(total - avail) + " used"
                            color: "#f1f3f7"
                            font.pixelSize: 18
                        }
                        Label {
                            text: root.fmtBytes(root.metrics.disk?.availableBytes || 0) + " free"
                            color: "#aeb4c0"
                        }
                    }
                }
            }

            Label {
                visible: root.errorText.length > 0
                text: root.errorText
                color: "#ff8b8b"
            }
        }
    }
}
