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
This item provides a secure slideswitch. A SlideSwitch is a switch that can
not be pressed accidentally. The user has to slide to knob to the other side
to toggle. There are two skins available.

property alias on                       A property to check if the button is on.
property string backgroundImageNormal   The normal background image.
property string backgroundImageHover    The background image when hovered.
property string knobImageOn             The knob image when the switch is on.
property string knobImageOff            The knob image when the switch is off.
*/
Item {
    id: slideswitch

    property alias on: button.on
    property string theme: "Beryl"
    property string backgroundImageNormal: "../pics/svgslideswitch/" + theme + "/background.svg"
    property string backgroundImageHover: "../pics/svgslideswitch/" + theme + "/background_hover.svg"
    property string knobImageOn: "../pics/svgslideswitch/" + theme + "/knob_on.svg"
    property string knobImageOff: "../pics/svgslideswitch/" + theme + "/knob_off.svg"

    Image {
        id: background

        property bool hover: false

        width: parent.width
        height: parent.height
        sourceSize.width: width
        sourceSize.height: height
        anchors.fill: parent
        source: hover ? backgroundImageHover : backgroundImageNormal
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true

        onEntered: {
            background.hover = true
        }
        onExited: {
            background.hover = false
        }
    }

    Image {
        id: button

        property bool on: false

        source: button.on ? knobImageOn : knobImageOff
        height: background.height
        width: height
        sourceSize.width: width
        sourceSize.height: height

        MouseArea {
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: 0
            drag.maximumX: background.width - button.width

            onReleased: {
                if(button.x > ((background.width - button.width) / 2)) {
                    slideswitch.state = ""
                    slideswitch.state = "on"
                }
                else {
                    slideswitch.state = ""
                    slideswitch.state = "off"
                }
            }
        }
    }

    // using states to differentiate between "on" and "off"
    states: [
        State {
            name: "on"
            PropertyChanges {
                target: button
                x: background.width - button.width
                on: true
            }
        },
        State {
            name: "off"
            PropertyChanges {
                target: button
                x: 0
                on: false
            }
        }
    ]

    transitions: Transition {
        NumberAnimation {
            properties: "x"
            duration: 400
            easing.type: Easing.OutQuad
        }
    }
}
