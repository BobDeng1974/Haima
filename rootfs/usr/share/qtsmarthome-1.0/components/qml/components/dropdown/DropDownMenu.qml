/** This file is part of Medical Device Qt Quick Embedded Widgets Demo**
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
Here a drop-down-menu is defined. The behavior of its c++-counterpart is
recreated as close as possible. It uses a listview to display the contents
of the menu.

property alias model:   The model for the menu.
property bool menuOpen: Flag to check if the menu is open or not.
property int itemIndex: The index of the currently selected item.
*/
Rectangle {
    id: box

    property alias model: list.model
    property bool menuOpen: false
    property int itemIndex: 0

    radius: 2
    smooth: true
    border.width: 2
    border.color: "gray"
    z: 10

    Text {
        id: boxText
        smooth: true
        // always display the first entry at the beginning
        text: list.model.get(0).name
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: 2
    }

    Timer {
        id: closeTimer
        interval: 300
        onTriggered: menuOpen = false
    }

    Rectangle {
        id: menuButton
        width: 30
        height: parent.height
        color: "#8b8b8b"
        radius: 2
        smooth: true
        anchors.right: parent.right
        anchors.top: parent.top

        Text {
            id: arrow
            text: menuOpen ? "^" : "v"
            anchors.centerIn: parent
            smooth: true
        }

        MouseArea {
            anchors.fill: parent
            onClicked: menuOpen = !menuOpen
        }
    }

    Rectangle {
        id: menu
        width: parent.width
        height: list.count * 16
        visible: menuOpen
        smooth: true
        radius: 2
        anchors.top: parent.bottom
        anchors.topMargin: 1
        anchors.left: parent.left

        ListView {
            id: list
            width: box.width
            height: menu.height
            highlight: Rectangle {
                width: list.width
                color: "lightsteelblue"
                radius: 2
            }
            highlightFollowsCurrentItem: true
            focus: true
            clip: true
            delegate: Item {
                property alias text: listText.text
                width: parent.width
                height: 20
                Text {
                    id: listText
                    anchors.verticalCenter: parent.verticalCenter
                    text: name
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true

                onPressed: {
                    boxText.text = list.currentItem.text
                    closeTimer.start()
                    itemIndex = list.currentIndex
                }

                onMouseYChanged: {
                    list.currentIndex = list.indexAt(mouseX, mouseY)
                }
                onMouseXChanged: {
                    list.currentIndex = list.indexAt(mouseX, mouseY)
                }
            }
        }
    }
}
