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
* This Item shows information about the temperature mainly as texts.
*/
Item {
    id: temperatureView

    property real current: 23.1
    property real minimum: 18.3
    property real maximum: 27.8
    property color textColor: "red"

    function trailingZero(n) {
        return String(n).replace(/^(-?\d+)$/, "$1.0");
    }

    /**
    * This SmartHomeText item shows the current temperature
    */
    Column {

        anchors.horizontalCenter: parent.horizontalCenter
        spacing: parent.height > 200 ? 10 : 5

        Item {
            id: dummy1
            width: parent.width
            height: 5
        }

        Row {

            anchors.horizontalCenter: parent.horizontalCenter

            SHText {
                text: trailingZero(Math.floor(current * 10) / 10)
                font.pointSize: 70
                color: temperatureView.textColor
            }

            /**
            * This SmartHomeText item shows Celsius sign
            */
            SHText {
                id: cText
                text: "°C"
                font.pointSize: 40
                color: temperatureView.textColor
            }
        }

        Item {
            id: dummy2
            width: parent.width
            height: parent.height > 200 ? 10 : 0
        }

        /**
        * This SmartHomeText item shows max temperature for this day
        */
        SHText {
            id: maxTemp
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Max: " + trailingZero(Math.floor(maximum * 10) / 10)
            font.pointSize: 15
            color: temperatureView.textColor
        }
        /**
        * This SmartHomeText item shows min temperature for this day
        */
        SHText {
            id: minTemp
            anchors.horizontalCenter: parent.horizontalCenter
            text: "Min: " + trailingZero(Math.floor(minimum * 10) / 10)
            font.pointSize: 15
            color: temperatureView.textColor
        }
    }
}
