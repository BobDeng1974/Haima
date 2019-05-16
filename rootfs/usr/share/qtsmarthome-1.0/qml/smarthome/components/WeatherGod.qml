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
* Why doing a rain dance or pray for sun.
* Just use the WeatherGod Item to adjust the weater.
* A slider is used to adjust the weather.
* Warning it changes the weather just for this app. Sorry.
* TODO: Connect to Petrus or other weather entities as you like.
* There was no intention to discriminate any religion.
*/
Item {
    id: god

    property real newWeatherCondition: conditionSlider.value || 0
    property variant weatherStates: ["sunny", "cloudy", "rainy", "stormy", "thunder", "snow"]

    SHText {
        id: lord
        x: conditionSlider.x
        text: "CHOOSE YOUR WEATHER"
        font.pointSize: 20
    }

    /**
    * This image shows the state of the weather as text
    */
    Image {
        id: balloon

        property string text: weatherStates[Math.round(newWeatherCondition)]

        anchors {
            top: lord.bottom
            horizontalCenter: god.horizontalCenter
            topMargin: 50
        }
        source: "pics/weathergod/preset.svg"

        SHText {
            anchors.centerIn: parent
            font.pointSize: 14
            color: "black"
            text: balloon.text
        }
    }

    /**
    * Draws the fancy weather icons upon the rest
    */
    Image {
        id: iconsline
        z: conditionSlider.z + 1
        anchors {
            top: balloon.bottom
            horizontalCenter: god.horizontalCenter
            topMargin: 40
        }
        source: "pics/weathergod/icons_line.png"
    }

    /**
    * An ActualValueSlider is used to adjust the weather. (\see ActualValueSlider)
    */
    ActualValueSlider {
        id: conditionSlider

        anchors.top: iconsline.bottom
        anchors.horizontalCenter: god.horizontalCenter
        anchors.topMargin: -98

        width: 660
        valueSliderHeight: 54
        backgroundImage: "pics/weathergod/weathergod_scale.png"

        /**
         * styling the balloon
         */
        balloonHeight: 56
        balloonTriangleHeight: 20
        balloonImage: "pics/weathergod/balloon/balloon_minsize.svg"
        balloonTriangleImage: "pics/weathergod/balloon/balloon_triangle.svg"

        /**
         * styling the knob
         */
        knobImagePressed: "pics/fader/button_pressed.svg"
        knobImageNormal: "pics/fader/button_normal.svg"

        /**
         * styling the needle
         */
        needleImage: "pics/weathergod/needle.svg"
        needleVisible: false

        /**
         * styling the focus
         */
        focusVisible: true
        focusImage: "pics/weathergod/focus.svg"

        minValue: 0
        maxValue: 5
        snapToValue: true
    }
}
