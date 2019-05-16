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
A simple button containing text that changes when the button is clicked. To
get a horizontal gradient the button is rotated 90 degrees.

property bool clicked:       Flag to check if the button was clicked.
property bool active:        Flag to see if the button is active.
property string buttonText1: First text on the button.
property string buttonText2: Second text on the button.

signal activated()
*/
Rectangle {
    id: button

    property bool clicked: false
    property bool active: true
    property string buttonText1: ""
    property string buttonText2: ""

    signal activated()

    rotation: 90
    smooth: true
    radius: 4
    border.width: 2
    border.color: clicked ? "#B4B4B4" : "#4B4B4B"
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#dcdcdc"
        }

        GradientStop {
            position: 0.5
            color: "#7f7f7f"
        }

        GradientStop {
            position: 1
            color: "#dcdcdc"
        }
    }

    MouseArea {
        anchors.fill: parent

        onPressed: {
            active = !active
            button.clicked = !button.clicked
            button.activated()
        }
        onReleased: {
            button.clicked = !button.clicked
        }
    }

    Text {
        id: buttonText
        text: active ? buttonText1 : buttonText2
        anchors.centerIn: parent
        rotation: -90
        font.bold: true
    }
}
