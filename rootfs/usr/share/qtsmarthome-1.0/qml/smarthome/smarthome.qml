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

//import QtQuick 2.0
import QtQuick 2.0
import "components/models"
import "components"

/**
smarthome.qml
This is the main QML file for the smarthome application. Here the main screen is defined and
configured.
Basically the main screen consists of two main portions:
the "The Weather God" screen and the smart home main screen. The two screens are aligned vertically
on top of each other. The WeatherGod Screen can be activated using the icon in the right corner of the
title bar.
The two screens are implemented using the "Rectangle" QML Element and a number of custom defined
elements. The screen switch logic is implemented using QML States and QML Transitions (see below)
*/

Item {
    id: smartHome
    width: 800
    height: 480
    smooth: false

    property int parentOpacity: (parent == null) ? 1 : parent.opacity

    property int currentWeather: 0
    property int titlebarHeight: 50
    property int spacing: 10

    /**    This Element defines the main screen area which is visible most of the time.
    */
    MainScreen {
        id: mainScreen
        width: parent.width
        height: parent.height
        xSpacing: smartHome.spacing
        ySpacing: titlebarHeight+smartHome.spacing

        animationsHalted: (smartHome.parentOpacity == 0)

        // we need to round because WeatherGodView also does
        weather: Math.round(currentWeather.weatherCondition)
        forecastWind: currentWeather.oneDayWindForce
        forecastRainProp: currentWeather.oneDayRainPropability
        forecastRain: currentWeather.oneDayRain
        rain: currentWeather.currentRain
        wind: currentWeather.currentWindForce
        temperature: currentWeather.currentTemperature
        temperatureHigh: currentWeather.currentHighTemperature
        temperatureLow: currentWeather.currentLowTemperature
        hour: currentWeather.hour
        minute: currentWeather.minute
        lux: currentWeather.currentLux

        Titlebar {
            id: titlebar

            width: parent.width
            height: titlebarHeight
            anchors.top: parent.top
            titleBarText: "Smart Home Control"
            dateAndTime: currentWeather.currentDateAndTime
            titleHomeButton.visible: mainScreen.menuIsCovered

            onGodModeChanged: godMode ? smartHome.state = "weathergod" : smartHome.state = ""
            onHomePressed: {
                smartHome.state = ""
                mainScreen.home()
            }
        }
    }

    /**
    Here is the Screen displaying the "Weather God". It consists of an QML Image
    element (the background image) and the custom "WeatherGod" (see components/weatherGod.qml)
    Element
    */
    WeatherGodScreen {
        id: weatherGodScreen
        width: parent.width
        height: parent.height - titlebarHeight
        anchors.bottom: mainScreen.top
        onNewWeatherConditionChanged: currentWeather.weatherCondition = newWeatherCondition
    }

    WeatherItem {
        id: currentWeather
        onCheckState: mainScreen.checkState();
    }

    /*
    The main screen has two states.
    - the default state
    - the "weathergod" state
    Animated state switching is done using QML transistions.
    */
    states: [
        State {
            name: "weathergod"
            PropertyChanges {
                target: mainScreen
                y: smartHome.height - titlebarHeight
            }
            PropertyChanges {
                target: titlebar
                titleBarText: "Weather God Control"
            }
        }
    ]
    /*
    Here the transitions are defined. A simple NumberAnimation is used to animate the "y" property
    of the mainScreen.
    */
    transitions: [
        Transition {
            from: "*"
            to: "weathergod"
            NumberAnimation {
                properties: "y"
                duration: 1000
                easing.type: Easing.OutBounce
            }

        },
        Transition {
            from: "weathergod"
            to: "*"
            NumberAnimation {
                properties: "y"
                duration: 1000
                easing.type: Easing.OutBounce
            }
        }
    ]
}
