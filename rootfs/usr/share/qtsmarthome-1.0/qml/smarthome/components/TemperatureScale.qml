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
* This TemperatureScale is a custom item for the temperature scale.
* The temperature scale draws the verticals lines for the temperature
*/
Column {
    id: scaleMain

    property bool enableText: true
    property int lineCount: 10
    property variant scaleModel: ["-10°", "0°", "10°", "20°", "30°", "40°"]

    height: 200
    spacing: 0

    Row {
        anchors.right: parent.right

        Text {
            y: -font.pixelSize/2
            height: 1
            text: scaleModel[scaleModel.length-1]
            color: "white"
            visible: enableText
        }

        Rectangle {
            width: 20
            height: 1
            color: "white"
        }
    }

    Repeater {
        anchors.right: parent.right
        width: parent.width
        id: scaleRep
        model: scaleModel.length - 1

        Column {
            anchors.right: parent.right
            id: scaleColumn
            spacing: scaleMain.height / (lineCount * (scaleModel.length - 1)) - 1

            Rectangle {
                width: 20
                height: 1
                color: "transparent"
            }

            Repeater {
                model: 9
                Rectangle {
                    anchors.right: parent.right
                    width: 10
                    height: 1
                    color: "white"
                }
            }

            Row {

                Text {
                    y: -font.pixelSize / 2
                    height: 1
                    text: scaleModel[scaleModel.length - 2 - index]
                    color: "white"
                    visible: enableText
                }

                Rectangle {
                    width: 20
                    height: 1
                    color: "white"
                }
            }
        }
    }
}

