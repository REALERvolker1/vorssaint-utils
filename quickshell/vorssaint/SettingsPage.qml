pragma ComponentBehavior: Bound

import Quickshell.Io
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Item {
    id: root

    required property string backendCommand
    property var settings: ({})
    signal settingsSaved()

    property string statusText: ""

    function save(path, value) {
        settingsProcess.exec([
            backendCommand,
            "settings",
            "set",
            path,
            JSON.stringify(value)
        ])
    }

    Process {
        id: settingsProcess

        stdout: StdioCollector {
            onStreamFinished: {
                try {
                    const result = JSON.parse(text)
                    root.statusText = result.ok
                        ? "Setting saved."
                        : (result.error || "Could not save setting.")
                    if (result.ok)
                        root.settingsSaved()
                } catch (e) {
                    root.statusText = "Settings backend returned invalid output."
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
            spacing: 10

            Label {
                text: "Panel"
                color: "#f1f3f7"
                font.bold: true
                font.pixelSize: 16
            }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: "Width"
                    color: "#c6cad2"
                    Layout.preferredWidth: 180
                }

                SpinBox {
                    id: widthBox
                    from: 480
                    to: 1600
                    value: root.settings.panel?.width || 760
                }

                Button {
                    text: "Apply"
                    onClicked: root.save("panel.width", widthBox.value)
                }

                Item { Layout.fillWidth: true }
            }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: "Height"
                    color: "#c6cad2"
                    Layout.preferredWidth: 180
                }

                SpinBox {
                    id: heightBox
                    from: 360
                    to: 1400
                    value: root.settings.panel?.height || 600
                }

                Button {
                    text: "Apply"
                    onClicked: root.save("panel.height", heightBox.value)
                }

                Item { Layout.fillWidth: true }
            }

            Label {
                text: "System"
                color: "#f1f3f7"
                font.bold: true
                font.pixelSize: 16
                Layout.topMargin: 8
            }

            RowLayout {
                Layout.fillWidth: true

                Label {
                    text: "Metrics refresh (ms)"
                    color: "#c6cad2"
                    Layout.preferredWidth: 180
                }

                SpinBox {
                    id: refreshBox
                    from: 500
                    to: 60000
                    stepSize: 250
                    value: root.settings.system?.refreshMs || 2000
                }

                Button {
                    text: "Apply"
                    onClicked: root.save("system.refreshMs", refreshBox.value)
                }

                Item { Layout.fillWidth: true }
            }

            Label {
                text: "Interface"
                color: "#f1f3f7"
                font.bold: true
                font.pixelSize: 16
                Layout.topMargin: 8
            }

            CheckBox {
                id: compactCheck
                text: "Compact service rows"
                checked: root.settings.ui?.compactRows || false
                onToggled: root.save("ui.compactRows", checked)
            }

            CheckBox {
                id: utilitiesCheck
                text: "Show Utilities tab"
                checked: root.settings.ui?.showUtilities !== false
                onToggled: root.save("ui.showUtilities", checked)
            }

            Label {
                Layout.fillWidth: true
                text: root.statusText
                visible: text.length > 0
                color: "#aeb4c0"
                wrapMode: Text.Wrap
            }

            Label {
                Layout.fillWidth: true
                text: "Settings are stored under the XDG config directory, not beside the executable."
                color: "#686e79"
                font.pixelSize: 10
                wrapMode: Text.Wrap
            }
        }
    }
}
