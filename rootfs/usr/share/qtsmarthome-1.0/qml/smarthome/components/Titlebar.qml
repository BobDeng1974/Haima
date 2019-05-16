/** This file is part of Home Control Qt Quick Embedded Widgets Demo**
 *
 * Copyright © 2010 Nokia Corporation and/or its subsidiary(-ies).*
 * All rights reserved.
 *
 * Contact:  Nokia Corporation (qt-info@nokia.com)
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * “Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.”
 */

import QtQuick 2.0

/**
 * The titleBar shows the date, time, label and the god button
 * The god button leads to weather god control
 */
Image {
    id:titleBar

    property alias titleBarText: titleBarText.text
    property alias titleHomeButton: homeButton
    property alias dateAndTime: dateText.text
    property bool godMode: godIcon.godAwareness

    signal homePressed();

    source: "pics/screenelements/titlebar.png"

    Image {
        id:qtLogo
        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        source: "pics/logos/qt-logo-small.png"
    }

    BorderImage {
        id: dateImage
        source: "pics/screenelements/text.svg"
        width: dateText.width + 40; height: parent.height - 16
        anchors.left: qtLogo.right
        anchors.verticalCenter: parent.verticalCenter
        border { left: 25; right: 25; top: 16; bottom: 16 }
        horizontalTileMode: BorderImage.Repeat
        verticalTileMode: BorderImage.Repeat

        SHText {
            id: dateText
            anchors.centerIn: parent
            color: "white"
            font {
                pointSize: titleBar.width > 840 ? 14 : 11
            }
        }
    }

    BorderImage {
        id: borderGod
        source: "pics/screenelements/text.svg"
        width: parent.height
        height: parent.height - 16
        anchors.left: dateImage.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 20
        border { left: 25; right: 25; top: 16; bottom: 16 }
        horizontalTileMode: BorderImage.Repeat
        verticalTileMode: BorderImage.Repeat

        Image {
            id: godIcon

            property bool godAwareness: false
            source: godAwareness ? "pics/screenelements/close_weathergod.svg" : "pics/screenelements/open_weathergod.svg"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }

        MouseArea {
            anchors.fill: parent

            onPressed: {
                godIcon.godAwareness = !godIcon.godAwareness
            }
        }
    }

    SHText {
        id: titleBarText
        //anchors.horizontalCenter: parent.horizontalCenter
        anchors.left: borderGod.right
        anchors.leftMargin: 8
        anchors.verticalCenter: parent.verticalCenter
        color: "white"
        font { pointSize: 21; bold: true }
    }

    BorderImage {
        id: homeButton
        source: "pics/screenelements/text.svg"
        width: parent.height
        height: parent.height - 16
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.rightMargin: 4
        border { left: 25; right: 25; top: 16; bottom: 16 }
        horizontalTileMode: BorderImage.Repeat
        verticalTileMode: BorderImage.Repeat

        Image {
            id: homeIcon

            property bool homePressed: false

            anchors.centerIn:parent

            source: "pics/screenelements/home.svg"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
        }

        MouseArea {
            anchors.fill: parent

            onPressed: {
                homeIcon.homePressed = true
            }

            onReleased: {
                titleBar.homePressed();
                homeIcon.homePressed = false
            }
        }
    }

}
