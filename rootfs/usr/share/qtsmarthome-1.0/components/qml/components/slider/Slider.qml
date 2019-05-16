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
This file defines a simple slider. Since only seems to support horizontal
gradients the background of the slider is a slightly rotated rectangle
with a gradient that is clipped by a rectangle defining the actual slider
box.

property int value:      The value that the slider currently has.
property int lowerLimit: The lower limit of the slider.
property int upperLimit: The upper limit of the slider.
*/
Rectangle {
    property int value: 0
    property int lowerLimit: 0
    property int upperLimit: 0

    id: background
    color: "transparent"
    smooth: true
    radius: 2
    border.width: 2
    border.color: "#b4b4b4"

    // workaround to have non-horizontal/non-vertical gradients:
    // define a item that clips a rectangle that is rotated
    Item {
        width: parent.width - 2
        height: parent.height - 2
        anchors.centerIn: parent
        clip: true

        Rectangle {
            x: -61
            y: -44
            width: 254
            height: 93
            rotation: -11
            gradient: Gradient {
                GradientStop {
                    position: 0
                    color: "#ffffff"
                }

                GradientStop {
                    position: 1
                    color: "#000000"
                }
            }      
        }
    }

    Rectangle {
        id: button
        x: 6
        height: 26
        gradient: Gradient {
            GradientStop {
                position: 0
                color: "#cccccc"
            }

            GradientStop {
                position: 0.4
                color: "#818181"
            }

            GradientStop {
                position: 1
                color: "#d3d3d3"
            }
        }
        width: 18
        smooth: true
        border.width: 2
        border.color: "#4B4B4B"
        anchors.verticalCenter: parent.verticalCenter
        rotation: 90

        MouseArea {
            anchors.fill: parent
            drag.target: parent
            drag.axis: Drag.XAxis
            drag.minimumX: 6
            drag.maximumX: background.width - button.width - 6

            onPressed: {
                button.border.color = "#b4b4b4"
            }
            onReleased: {
                button.border.color = "#4B4B4B"
            }
            onPositionChanged: {
                value = lowerLimit + ((parent.x - 6) / (drag.maximumX - drag.minimumX)) * (Math.abs(upperLimit) + Math.abs(lowerLimit))
            }
        }
    }
}
