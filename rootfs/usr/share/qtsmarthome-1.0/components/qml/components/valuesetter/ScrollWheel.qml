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
This Item shows a scrollwheel. It can be dragged and flicked to adjust a value.

property int value          The value that is currently set.
property int minimum        The minimum value for the scrollwheel.
property int maximum        The maximum value for the scrollwheel.
property int dragDistance   The distance the wheel has to be dragged to change the current value.
property int currentIndex   A property used internally to define which image should be used.
property alias wheelImage1  The first image for the scrollwheel.
property alias wheelImage2  The second image for the scrollwheel.
property alias wheelImage3  The third image for the scrollwheel.

function changeValue(value) A function used internally to set 'currentIndex' and 'value'.

*/
Item {
    id: wheel

    property int value: 60
    property int minimum: 0
    property int maximum: 100
    property int dragDistance: 2
    property int currentIndex: 0
    // There are 3 images to simulate real rotation.
    property alias wheelImage1: scroller1.source
    property alias wheelImage2: scroller2.source
    property alias wheelImage3: scroller3.source

    function changeValue(delta) {
        var imageCount = 3
        if (delta < 0 && minimum <= value + delta) {
            currentIndex = (currentIndex + imageCount - 1) % imageCount // -delta (decrease)
            value += delta;
        } else if (delta > 0 && maximum >= value + delta) {
            currentIndex = (currentIndex + 1) % imageCount // +delta (increase)
            value += delta;
        }
    }

    Image {
        id: scroller1
        width: parent.width
        height: parent.height
        sourceSize.height: height
        sourceSize.width: width
        visible: currentIndex == 0
        source: "../pics/scrollwheel/Beryl/wheel_1.svg"
    }

    Image {
        id: scroller2
        width: parent.width
        height: parent.height
        sourceSize.height: height
        sourceSize.width: width
        visible: currentIndex == 1
        source: "../pics/scrollwheel/Beryl/wheel_2.svg"
    }

    Image {
        id: scroller3
        width: parent.width
        height: parent.height
        sourceSize.height: height
        sourceSize.width: width
        visible: currentIndex == 2
        source: "../pics/scrollwheel/Beryl/wheel_3.svg"
    }

    MouseArea {
        id: area

        property real startY: 0
        property real diff: 0
        property int toValue: 0

        anchors.fill: parent
        onPressed: {
            startY = mouseY
        }
        onPositionChanged: {
            diff = mouseY - startY

            if(Math.abs(diff) > dragDistance) {
                changeValue(diff > 0 ? -1 : 1)
                startY = mouseY
            }
        }
        onReleased: {
            if(Math.abs(diff) > dragDistance) {
                toValue = -(diff * 2)
                fade.running = true
            }
        }

        onToValueChanged: {
            changeValue(toValue)
        }

        NumberAnimation on toValue {
            id:fade
            target: area
            property: "toValue"
            to: 0
            duration: 600
        }
    }
}
