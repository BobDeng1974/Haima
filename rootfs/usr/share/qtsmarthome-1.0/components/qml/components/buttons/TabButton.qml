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
A special button that can switch between the tabs.
*/
Rectangle {
    id: tabButton

    property int index: 0
    property bool hover: false
    property alias buttonText: bText.text
    property color topColor: "#707072"
    property color bottomColor: "#403e41"
    property bool active: false

    objectName: "tabButton"

    onActiveChanged: state = active ? "active" : ""

    signal activated()

    width: 58
    height: 28
    smooth: true
    radius: 4
    border.color: hover ? "#4B4B4B" : "#b4b4b4"
    border.width: 2

    gradient: Gradient {
        GradientStop {
            position: 0
            color: topColor
        }

        GradientStop {
            position: 1
            color: bottomColor
        }
    }

    Text {
        id: bText
        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        color: hover ? "black" : "white"
    }

    states: [
        State {
            name: "hovered"
            PropertyChanges {
                target: tabButton
                topColor: "#b4b4b4"
                bottomColor: "#b4b4b4"
            }
        },
        State {
            name: "active"
            PropertyChanges {
                target: tabButton
                topColor: "#b4b4b4"
                bottomColor: "#b4b4b4"
                border.color: "#b4b4b4"
            }
            PropertyChanges {
                target: bText
                color: "black"
            }
        }
    ]

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            tabButton.hover = true
            tabButton.state = "hovered"
        }
        onExited: {
            tabButton.hover = false
            if(tabButton.active == true)
            {
                tabButton.state = "active"
            } else {
                tabButton.state = ""
            }
        }
        onClicked: {
            tabButton.active = true
            tabButton.activated()
        }
    }
}
