pragma ComponentBehavior: Bound

import Quickshell
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ShellRoot {
    Variants {
        model: Quickshell.screens

        PanelWindow {
            id: bar

            required property var modelData
            screen: modelData

            anchors {
                top: true
                left: true
                right: true
            }

            implicitHeight: 38
            color: "#15171c"
            exclusiveZone: 38

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 10
                anchors.rightMargin: 10

                Label {
                    text: bar.modelData.name
                    textFormat: Text.PlainText
                    color: "#858b98"
                    font.pixelSize: 11
                }

                Item { Layout.fillWidth: true }

                SystemClock {
                    id: clock
                    precision: SystemClock.Minutes
                }

                Label {
                    text: Qt.formatDateTime(clock.date, "ddd HH:mm")
                    color: "#c6cad2"
                }

                VorssaintWidget {
                    screen: bar.modelData
                }
            }
        }
    }
}
