/*
    Copyright (C) 2018 Sebastian J. Wolf

    This file is part of Funzel.

    Funzel is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Funzel is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Funzel. If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.0
import Sailfish.Silica 1.0
import "../components"


Page {
    id: overviewPage

    allowedOrientations: Orientation.All
    property variant colorAssignments

    states: [
        State { name: "gemini"; when: funzel.deviceInfo.device == Device.GeminiPDA
        },
        State { name: "jp2026"; when: funzel.deviceInfo.device == Device.JollaPhone2026
              PropertyChanges { target: inariSwitch; visible: true }
        },
        State { name: "unsupported"; when: funzel.deviceInfo.device == Device.Invalid
                                            || funzel.deviceInfo.device == Device.Unknown
                                            || !funzel.supportedDeviceFound
              PropertyChanges { target: noSupportedDeviceWarningFlickable; visible: true }
              PropertyChanges { target: noSupportedDeviceWarningBackground; visible: true }
              PropertyChanges { target: funzelSwitch; enabled: false }
        }
    ]

    TheHoffModel {
        id: hoffModel
    }

    Component.onCompleted: {
        funzelSwitch.checked = funzel.getUseAnimation();
        funzelColor.currentIndex = funzel.getAnimationColor();
        overviewPage.colorAssignments = funzel.getColorAssignments();
    }

    Connections {
        target: funzel
        onPowerOn: {
            if (funzelSwitch.checked) {
                hoffModel.setColorIndex(funzelColor.currentIndex);
                hoffModel.startTheHoff();
            }
        }
        onPowerColor: {
            if (funzelSwitch.checked) {
                hoffModel.setColorIndex(colorIndex);
                hoffModel.startTheHoff();
            }
        }
        onPowerOff: {
            hoffModel.stopTheHoff();
        }
        onUseAnimationChanged: {
            funzelSwitch.checked = funzel.getUseAnimation();
        }
        onContactAssignmentsInvalidated: {
            console.log("Contact assignments were invalidated");
            overviewPage.colorAssignments = funzel.getColorAssignments();
        }
    }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: overviewColumn.height

        PullDownMenu {
            MenuItem {
                text: qsTr("About Funzel")
                onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
            }
        }

        Column {
            id: overviewColumn

            width: overviewPage.width
            spacing: Theme.paddingMedium
            PageHeader { id: header
                title: qsTr("Welcome to Funzel")
                description: funzel.deviceInfo.name
            }

            TextSwitch {
                id: funzelSwitch
                text: qsTr("Enable LED animation on incoming call")
                description: qsTr("When your device receives a call, the backside LEDs will show an animation.")
                onCheckedChanged: {
                    funzel.setUseAnimation(checked);
                }
            }

            ComboBox {
                id: funzelColor
                label: qsTr("Default Animation Color")
                menu: ContextMenu {
                    MenuItem {
                        text: qsTr("Red")
                    }
                    MenuItem {
                        text: qsTr("Green")
                    }
                    MenuItem {
                        text: qsTr("Blue")
                    }
                    MenuItem {
                        text: qsTr("Yellow")
                    }
                    MenuItem {
                        text: qsTr("Light Blue")
                    }
                    MenuItem {
                        text: qsTr("Purple")
                    }
                    MenuItem {
                        text: qsTr("White")
                    }
                    MenuItem {
                        text: qsTr("Rainbow")
                    }
                }
                enabled: funzelSwitch.checked
                onCurrentIndexChanged: {
                    funzel.setAnimationColor(currentIndex);
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Test Animation")
                enabled: funzelSwitch.checked
                onClicked: {
                    hoffModel.setColorIndex(funzelColor.currentIndex);
                    hoffModel.startTheHoff();
                    testAnimationTimer.start();
                }
            }

            Timer {
                id: testAnimationTimer
                interval: 7000
                repeat: false
                onTriggered: {
                    hoffModel.stopTheHoff();
                }
            }

            SectionHeader {
                visible: funzel.isContactsDbAvailable()
                text: qsTr("Contact-specific Animation Color")
                font.pixelSize: Theme.fontSizeMedium
                height: Theme.itemSizeSmall
            }

            Button {
                visible: funzel.isContactsDbAvailable()
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Assign Color to Contact")
                enabled: funzelSwitch.checked
                onClicked: {
                    pageStack.push(Qt.resolvedUrl("ContactsSelectionPage.qml"))
                }
            }

            SectionHeader {
                text: qsTr("Red")
                visible: redRepeater.count > 0
            }

            Repeater {
                id: redRepeater
                model: overviewPage.colorAssignments.red
                delegate: AssignedContactsListItem {}
            }

            SectionHeader {
                text: qsTr("Green")
                visible: greenRepeater.count > 0
            }

            Repeater {
                id: greenRepeater
                model: overviewPage.colorAssignments.green
                delegate: AssignedContactsListItem {}
            }

            SectionHeader {
                text: qsTr("Blue")
                visible: blueRepeater.count > 0
            }

            Repeater {
                id: blueRepeater
                model: overviewPage.colorAssignments.blue
                delegate: AssignedContactsListItem {}
            }

            SectionHeader {
                text: qsTr("Yellow")
                visible: yellowRepeater.count > 0
            }

            Repeater {
                id: yellowRepeater
                model: overviewPage.colorAssignments.yellow
                delegate: AssignedContactsListItem {}
            }

            SectionHeader {
                text: qsTr("Light Blue")
                visible: lightBlueRepeater.count > 0
            }

            Repeater {
                id: lightBlueRepeater
                model: overviewPage.colorAssignments.lightBlue
                delegate: AssignedContactsListItem {}
            }

            SectionHeader {
                text: qsTr("Purple")
                visible: purpleRepeater.count > 0
            }

            Repeater {
                id: purpleRepeater
                model: overviewPage.colorAssignments.purple
                delegate: AssignedContactsListItem {}
            }

            SectionHeader {
                text: qsTr("White")
                visible: whiteRepeater.count > 0
            }

            Repeater {
                id: whiteRepeater
                model: overviewPage.colorAssignments.white
                delegate: AssignedContactsListItem {}
            }

            /* Jolla Phone */ 
            TextSwitch {
                id: inariSwitch
                visible: false
                text: qsTr("Enable ToH LED animation")
                //description: qsTr("When your device receives a call, the backside LEDs will show an animation.")
                onCheckedChanged: {
                    funzel.tohLed(checked, 0);
                }
            }

        }
    }

    Rectangle {
        id: noSupportedDeviceWarningBackground
        anchors.fill: parent
        color: "black"
        opacity: 0.8
        visible: false
    }

    SilicaFlickable {
        id: noSupportedDeviceWarningFlickable
        contentHeight: warningColumn.height
        anchors.fill: parent
        visible: false

        Column {
            id: warningColumn
            width: parent.width
            spacing: Theme.paddingLarge

            PageHeader {
                id: warningHeader
                title: qsTr("No supported device found!")
            }

            Image {
                id: warningImage
                source: "image://theme/icon-l-attention"
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }

                fillMode: Image.PreserveAspectFit
                width: Theme.iconSizeLarge
                height: Theme.iconSizeLarge
            }

            Text {
                id: textContentNoSupportedDevice
                width: parent.width - 2 * Theme.horizontalPageMargin
                anchors.horizontalCenter: parent.horizontalCenter
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.primaryColor
                linkColor: Theme.highlightColor
                wrapMode: Text.Wrap
                textFormat: Text.PlainText
                //text: qsTr("It seems that Funzel is not running on a Gemini PDA. As this is the only device which Funzel is supporting, this application will be rather useless on your device.")
                text: qsTr("It seems that Funzel is not running on a supported device. This application will be rather useless on your device.")
                    + "\n" + qsTr("The following devices are supported: %1").arg(funzel.listSupporteDevices).join(",")
            }

            Button {
                text: qsTr("OK")
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
                onClicked: {
                    noSupportedDeviceWarningBackground.visible = false;
                    noSupportedDeviceWarningFlickable.visible = false;
                }
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width  - ( 2 * Theme.horizontalPageMargin )
                font.pixelSize: Theme.fontSizeExtraSmall
                wrapMode: Text.Wrap
                anchors {
                    horizontalCenter: parent.horizontalCenter
                }
            }

            VerticalScrollDecorator {}
        }

    }

}

