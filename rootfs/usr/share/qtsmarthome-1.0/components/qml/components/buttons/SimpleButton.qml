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
A simple button that changes its appearance when clicked. The themeing requires
the images to be in a directory with the same name as the themes' name.

property string theme:          The theme to be used.
property string imageNormal:    The image of the button when not pressed.
property string imageHover:     The image of the button when hovered.
property string imagePressed:   The image of the button when pressed.
property bool hover:            Internal flag to see if the button is hovered.
*/
Image {
    id: button

    property string theme: "Beryl"
    property string imageNormal: "../pics/svgbutton/"+theme+"/normal.svg"
    property string imageHover: "../pics/svgbutton/"+theme+"/hovered.svg"
    property string imagePressed: "../pics/svgbutton/"+theme+"/pressed.svg"
    property bool hover: false

    source: hover ? imageHover : imageNormal

    states: [
        State {
            name: "pressed"
            PropertyChanges {
                target: button
                source: imagePressed
            }
        }
    ]

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            button.hover = true
        }
        onExited: {
            button.hover = false
            button.state = ""
        }
        onPressed: {
            button.state = "pressed"
        }
        onReleased: {
            button.state = ""
        }
    }
}
