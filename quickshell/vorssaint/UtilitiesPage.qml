pragma ComponentBehavior: Bound

import Quickshell
import Quickshell.Io
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property string backendCommand
    property string cleanResult: ""
    property string statusText: ""

    Process {
        id: urlProcess

        stdout: StdioCollector {
            onStreamFinished: {
                try {
                    const result = JSON.parse(text)
                    root.cleanResult = result.url || ""
                    root.statusText = root.cleanResult.length > 0
                        ? "Tracking parameters removed where recognized."
                        : ""
                } catch (e) {
                    root.statusText = "URL cleaner returned invalid output."
                }
            }
        }
    }

    Flickable {
        anchors.fill: parent
        contentHeight: content.implicitHeight
        clip: true

        ColumnLayout {
            id: content
            width: parent.width
            spacing: 12

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: cleaner.implicitHeight + 28
                radius: 10
                color: "#1a1d24"

                ColumnLayout {
                    id: cleaner
                    anchors.fill: parent
                    anchors.margins: 14
                    spacing: 8

                    Label {
                        text: "Clean URL"
                        color: "#f1f3f7"
                        font.bold: true
                    }

                    TextField {
                        id: urlField
                        Layout.fillWidth: true
                        placeholderText: "Paste a URL"
                        onAccepted: urlProcess.exec([
                            root.backendCommand, "url", "clean", text
                        ])
                    }

                    RowLayout {
                        Layout.fillWidth: true

                        Button {
                            text: "Clean"
                            enabled: urlField.text.length > 0
                            onClicked: urlProcess.exec([
                                root.backendCommand, "url", "clean", urlField.text
                            ])
                        }

                        Button {
                            text: "Copy result"
                            enabled: root.cleanResult.length > 0
                            onClicked: Quickshell.clipboardText = root.cleanResult
                        }

                        Item { Layout.fillWidth: true }
                    }

                    TextField {
                        Layout.fillWidth: true
                        readOnly: true
                        visible: root.cleanResult.length > 0
                        text: root.cleanResult
                        selectByMouse: true
                    }

                    Label {
                        Layout.fillWidth: true
                        text: root.statusText
                        color: "#858b98"
                        wrapMode: Text.Wrap
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: 110
                radius: 10
                color: "#1a1d24"

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 14

                    Label {
                        text: "Integration seam"
                        color: "#f1f3f7"
                        font.bold: true
                    }

                    Label {
                        Layout.fillWidth: true
                        text: "This page is intentionally small. Package-manager, PipeWire, capture, clipboard-history, and hardware integrations can be added locally without changing the panel/backend boundary."
                        color: "#aeb4c0"
                        wrapMode: Text.Wrap
                    }
                }
            }
        }
    }
}
