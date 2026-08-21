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
import "."

CoverBackground {

    property bool isSwitchedOn: false

    function setStatusText(switchedOn) {
        isSwitchedOn = switchedOn;
    }

    Component.onCompleted: {
        setStatusText(funzel.getUseAnimation());
    }

    CoverActionList {
        enabled: funzel.deviceInfo.device == funzel.Device.GeminiPDA
        CoverAction {
            iconSource: isSwitchedOn ? "image://theme/icon-cover-mute" : "image://theme/icon-cover-unmute"
            onTriggered: {
                isSwitchedOn ? funzel.setUseAnimation(false) : funzel.setUseAnimation(true);
            }
        }
    }

    Image {
        source: "../../images/background.png"
        anchors {
            verticalCenter: parent.verticalCenter

            bottom: parent.bottom
            bottomMargin: Theme.paddingMedium

            right: parent.right
            rightMargin: Theme.paddingMedium
        }

        fillMode: Image.PreserveAspectFit
        opacity: 0.2
    }

    Connections {
        target: funzel
        onUseAnimationChanged: {
            setStatusText(funzel.getUseAnimation());
        }
    }

    Label {
        id: statusText
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: Theme.fontSizeExtraLarge
        text: isSwitchedOn ? qsTr("On") : qsTr("Off")
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
    }

}
