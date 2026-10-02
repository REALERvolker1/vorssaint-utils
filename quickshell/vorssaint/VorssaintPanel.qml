pragma ComponentBehavior: Bound

import Quickshell
import Quickshell.Io
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    required property var screen
    signal requestClose()

    property string backendCommand: "vorssaintctl"
    property var settings: ({
        panel: { width: 760, height: 600 },
        system: { refreshMs: 2000 },
        services: { defaultScope: "all", limit: 100 },
        ui: { compactRows: false, showUtilities: true }
    })

    readonly property int preferredWidth:
        settings.panel && settings.panel.width ? settings.panel.width : 760
    readonly property int preferredHeight:
        settings.panel && settings.panel.height ? settings.panel.height : 600

    color: "#111318"
    radius: 14
    border.color: "#30333b"
    border.width: 1

    function reloadSettings() {
        settingsProcess.exec([backendCommand, "settings", "dump"])
    }

    Process {
        id: settingsProcess

        stdout: StdioCollector {
            onStreamFinished: {
                try {
                    root.settings = JSON.parse(text)
                } catch (e) {
                    console.warn("vorssaint settings parse failed:", e)
                }
            }
        }
    }

    Component.onCompleted: reloadSettings()

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 10

        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Vorssaint"
                color: "#f1f3f7"
                font.pixelSize: 20
                font.bold: true
            }

            Label {
                text: root.screen.name
                color: "#858b98"
                font.pixelSize: 11
            }

            Item { Layout.fillWidth: true }

            ToolButton {
                text: "×"
                onClicked: root.requestClose()
            }
        }

        TabBar {
            id: tabs
            Layout.fillWidth: true

            TabButton { text: "System" }
            TabButton { text: "Windows" }
            TabButton { text: "Services" }
            TabButton {
                text: "Utilities"
                visible: root.settings.ui ? root.settings.ui.showUtilities !== false : true
            }
            TabButton { text: "Settings" }
        }

        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: tabs.currentIndex

            SystemPage {
                backendCommand: root.backendCommand
                refreshMs: root.settings.system ? root.settings.system.refreshMs : 2000
            }

            WindowsPage {
                screen: root.screen
                backendCommand: root.backendCommand
            }

            ServicesPage {
                backendCommand: root.backendCommand
                defaultScope: root.settings.services ? root.settings.services.defaultScope : "all"
                resultLimit: root.settings.services ? root.settings.services.limit : 100
                compactRows: root.settings.ui ? root.settings.ui.compactRows : false
            }

            UtilitiesPage {
                backendCommand: root.backendCommand
            }

            SettingsPage {
                backendCommand: root.backendCommand
                settings: root.settings
                onSettingsSaved: root.reloadSettings()
            }
        }
    }
}
