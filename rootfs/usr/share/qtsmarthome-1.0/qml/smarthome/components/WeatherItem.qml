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
import "../components"
/**
* The WeatherItem provides all values for the weather.
* The alogrithm to manipulate the weather and its values is nested.
*/
Item {
    // weather condition for god mode
    property real weatherCondition: 0
    property real weatherConditionMin: 0
    property real weatherConditionMax: 5

    // today
    property real currentTemperature: privateItem.updateTemperature()
    property real currentHighTemperature: (weatherCondition < 5) ? privateItem.updateHighTemperature() :   0
    property real currentLowTemperature:  (weatherCondition < 5) ? privateItem.updateLowTemperature()  : -10
    property real currentWindForce: privateItem.updateWindForce()
    property real currentRain: privateItem.updateRain()
    property real currentLux: privateItem.updateLux()

    // tommorrow
    property real oneDayTemperature: 21.7
    property real oneDayHighTemperature: 26.1
    property real oneDayLowTemperature: 17.5
    property real oneDayWindForce: 7.2
    property real oneDayRainPropability: 75
    property real oneDayRain: 18.5

    // day after tommorrow
    property real twoTemperature: 14.3
    property real twoDayHighTemperature: 18.2
    property real twoDayLowTemperature: 12.8
    property real twoDayWindForce: 10.1
    property real twoDayRain: 48.5

    // the current time could change the weather so it is needed here also
    property int hour: 12
    property int minute: 12
    property string currentDateAndTime: ""

    signal checkState()

    // This function updates the values for current time
    function updateTime() {
        var dateAndTime = new Date();
        var timeString = Qt.formatDateTime(dateAndTime, "dddd, dd.MM.yyyy | hh:mm");
        hour = Qt.formatDateTime(dateAndTime, "hh");
        minute = Qt.formatDateTime(dateAndTime, "mm");
        currentDateAndTime = timeString;
    }

    // is that needed?
    onHourChanged: privateItem.updateTemperature()
    onWeatherConditionChanged: privateItem.updateTemperature()

    /**
    * Some parameters are encapsulated here.
    * These are needed for the updateWeather() function
    */
    Item {
        id: privateItem

        property real minTemp: 20.2
        property real maxTemp: 37.1
        property real diffTemp: maxTemp - minTemp
        property real hAdjust: 1 - Math.abs(hour - 12) / 12

        property real minWind: 1
        property real maxWind: 12
        property real diffWind: maxWind - minWind
        property real wAdjust: 1 - Math.abs(weatherCondition - 3) / 5

        property real minLux: 1
        property real maxLux: 12
        property real diffLux: maxLux - minLux
        property real lAdjust: Math.abs(weatherCondition - 3) / 5

        property real minRain: 0.2
        property real maxRain: 49.7
        property real diffRain: maxRain - minRain
        property real rAdjust:  1 - Math.abs(weatherCondition - 2) / 5
        property real rAdjust2: (1 - Math.abs(weatherCondition - 4) / 5) * 0.5

        function updateTemperature()
        {
            if(weatherCondition >= 5) {
                return -7
            }
            else
            {
                return minTemp + diffTemp * hAdjust - weatherCondition / 5 * 21.2
            }
        }

        function updateHighTemperature()
        {
            return privateItem.maxTemp - weatherCondition/ 5 * 21
        }

        function updateLowTemperature()
        {
            return privateItem.minTemp - weatherCondition/ 5 * 21.2
        }

        function updateWindForce()
        {
            return ((privateItem.minWind + (privateItem.diffWind)* privateItem.wAdjust)) - 3.2
        }

        function updateRain()
        {
            return ((privateItem.minRain + (privateItem.diffRain)* privateItem.rAdjust))
                    + privateItem.diffRain * privateItem.rAdjust2 - 22.1
        }

        function updateLux()
        {
            return ((privateItem.minLux + (privateItem.diffLux)* privateItem.lAdjust)) + 2
        }
    }

    Timer {
        // check the time and states every interval
        interval: 2000
        running:true
        repeat: true
        triggeredOnStart: true
        onTriggered:{
            updateTime()
            checkState()
        }
    }

    // A behaviour (animation) defined for every change of currentTemperature
    Behavior on currentTemperature {
        id: behavior1
        NumberAnimation { duration: 10000; easing.type: Easing.OutBounce }
    }

    // A behaviour (animation) defined for every change of currentWindForce
    Behavior on currentWindForce {
        id: behavior2
        NumberAnimation { duration: 10000; easing.type: Easing.OutBounce }
    }

    // A behaviour (animation) defined for every change of currentRain
    Behavior on currentRain {
        id: behavior3
        NumberAnimation { duration: 10000; easing.type: Easing.OutBounce }
    }

    // A behaviour (animation) defined for every change of currentLux
    Behavior on currentLux {
        id: behavior4
        NumberAnimation { duration: 10000; easing.type: Easing.OutBounce }
    }
}
