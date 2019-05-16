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
* The weatherView Item shows different images and animations
* for every weather condition.
* This is realized with states for any condition.
*/
Item {
    id: weatherView

    property real weather: 0
    property bool isDay: true
    property bool animationsHalted: false

    // default is "sunny" state
    Component.onCompleted: state = "sunny"
    state: "sunny"
    // when the weather condition changes we change the state
    onWeatherChanged: {
        var index = weather
        var states = ["sunny", "cloudy", "rainy", "stormy", "thunder", "snow"]
        state = states[index]
    }

    onAnimationsHaltedChanged: {
        if( sun.visible ) {
            if( animationsHalted ) {
                sunAnim.stop()
            }else{
                sunAnim.start()
            }
        }
    }

    Item {
        id: weatherWrapper
        anchors.horizontalCenter: parent.horizontalCenter
        width: 220
        height: 300
        clip: true

        /**
        * Image instance for showing a sun
        */
        Image {
            id: sun
            x: (parent.width - width) / 2
            width: 140
            height: 140
            source: "pics/weather/sun.png"
            rotation: 0
            opacity: 0
            NumberAnimation {
                id: sunAnim
                target: sun
                property: "rotation"
                to: 360
                duration: 30000
                loops: Animation.Infinite
            }
        }

        /**
        * Image instance for showing fancy sun reflex
        */
        Image {
            id: sun_reflex
            anchors.fill: sun
            source: "pics/weather/sun_reflex.png"
            opacity: sun.opacity
            visible: sun.visible
        }

        /**
        * Image instance for showing a moon
        */
        Image {
            id: moon
            x: (parent.width - width) / 2
            width: 146
            height: 152
            source: "pics/weather/moon.png"

            Image {
                id: star1
                x: 40
                y: 36
                source: "pics/particle/star.png"
            }

            Image {
                id: star2
                x: 195
                y: 110
                source: "pics/particle/star.png"
            }

            Image {
                id: star3
                x: -14
                y: -42
                source: "pics/particle/star.png"
            }

            Image {
                id: star4
                x: -108
                y: 36
                source: "pics/particle/star.png"
            }
        }
        /**
        * Image instance for showing a normal cloud
        */
        Cloud {
            id: cloud
            objectName: "cloud"
            x: 60
            y: 64
            width: 267
            height: 157
            animationsHalted: weatherView.animationsHalted
        }

        /**
        * Image instance for showing a storm cloud
        */
        Cloud {
            id: stormCloud1
            objectName: "storm cloud1"
            y: 30
            width: 100
            height: 50
            opacity: 0
            z: -10
            animationsHalted: weatherView.animationsHalted
        }
        /**
        * Image instance for showing another storm cloud
        */
        Cloud {
            id: stormCloud2
            objectName: "storm cloud2"
            y: 50
            width: 150
            height: 60
            opacity: 0
            z: -10
            animationsHalted: weatherView.animationsHalted
        }
    }

    /**
    * For initialisation sun and cloud were set invisible
    */
    function init() {
        sun.visible = false
        cloud.visible = false
    }

    // All states that correspond to the weather condition are
    // defined here.
    // Every state changes propertys and start/stop animations
    states: [
        // Sunny
        State {
            name: "sunny"

            StateChangeScript {
                script: {
                    sunAnim.start()
                }
            }
            PropertyChanges {
                target: cloud
                wind:  ! animationsHalted
                rain:  false
                flash: false
                x: 40
                y: 64
                width: 160
                height: 94
                color1: "#74c4fd"
                color2: "#3690d1"
            }

            PropertyChanges {
                target: sun_reflex
                x: 46
                y: -2
                width: 147
                height: 144
                transformOrigin: "Center"
            }

            PropertyChanges {
                target: sun
                opacity: 1
            }
            PropertyChanges {
                target: weatherWrapper
                width: 220
                height: 300
            }

        },
        // Thunder
        State {
            name: "thunder"
            StateChangeScript {
                script: {
                    sunAnim.stop()
                    sun_reflex.visible = false
                }
            }

            PropertyChanges {
                target: sun
                visible: false
                opacity: 0
            }

            PropertyChanges {
                target: cloud
                particleSize:  20
                particleCount: 10
                particleSource: "pics/particle/raindrop.png"
                particleStretch: 0.5
                particleLifeSpan: 600
                rain:  ! animationsHalted
                wind:  ! animationsHalted
                storm: false
                flash: ! animationsHalted
                x: 0
                y: 20
                width: 214
                height: 126
                color1: "#638299"
                color2: "#35464d"
            }
            PropertyChanges {
                target: weatherWrapper
                width: 220
                height: 300
            }
        },
        // Rainy
        State {
            name: "rainy"
            StateChangeScript {
                script: {
                    sunAnim.stop()
                    sun_reflex.visible = false
                }
            }

            PropertyChanges {
                target: sun
                visible: false
                opacity: 0
            }

            PropertyChanges {
                target: cloud
                particleSize:  20
                particleCount: 10
                particleSource: "pics/particle/raindrop.png"
                particleStretch: 0.5
                particleLifeSpan: 550
                rain:  ! animationsHalted
                wind:  ! animationsHalted
                storm: false
                x: 0
                y: 20
                width: 214
                height: 126
                color1: "#b6dffd"
                color2: "#7d97ab"
            }
            PropertyChanges {
                target: weatherWrapper
                width: 220
                height: 300
            }
        },
        // Snow
        State {
            name: "snow"
            StateChangeScript {
                script: {
                    sunAnim.stop()
                    sun_reflex.visible = false
                }
            }


            PropertyChanges {
                target: sun
                visible: false
                opacity: 0
            }

            PropertyChanges {
                target: cloud
                wind:  false
                storm: false
                flash: false
                rain:  ! animationsHalted
                x: 0
                y: 20
                width: 214
                height: 126
                particleSize:  20
                particleCount: 20
                particleSource: "pics/particle/snowflake.png"
                particleStretch: 1
                particleVelocity: 50
                particleVelocityDeviation: 10
                particleLifeSpan: 1000
                particleLifeSpanDeviation: 500
                particleFadeOutDuration: 500
                color1: "darkGray"
                color2: "lightGray"
            }
            PropertyChanges {
                target: weatherWrapper
                width: 220
                height: 300
            }
        },
        // Cloudy
        State {
            name: "cloudy"

            StateChangeScript {
                script: {
                    sunAnim.stop()
                    sun_reflex.visible = false
                }
            }
            PropertyChanges {
                target: sun
                opacity: 0
            }
            PropertyChanges {
                target: cloud
                color1: Qt.darker( "darkGray", 1.1)
                color2: Qt.darker( "lightGray", 1.1)
                wind:  false
                storm: false
                flash: false
                rain:  false
                opacity: 1
                x: 20
                y: 14
                width: 200
                height: 120
            }
            PropertyChanges {
                target: stormCloud1
                color1: Qt.darker( "darkGray", 1.)
                color2: Qt.darker( "lightGray", 1.)
                wind:  false
                storm: false
                flash: false
                rain:  false
                opacity: 1
                x: 60
                y: 44
                z: 2
                width: 150
                height:100
            }
            PropertyChanges {
                target: stormCloud2
                color1: Qt.darker( "darkGray", 1.4)
                color2: Qt.darker( "lightGray", 1.4)
                wind: false; storm: false; flash: false; rain: false
                opacity: 1
                x: 0
                y: 44
                z: -2
                width: 180
                height:130
            }
            PropertyChanges {
                target: weatherWrapper
                width: 220
                height: 300
            }
        },
        // Stormy
        State {
            name: "stormy"

            StateChangeScript {
                script: {
                    sunAnim.stop()
                    sun_reflex.visible = false
                }
            }
            PropertyChanges {
                target: sun
                opacity: 0
            }
            PropertyChanges {
                target: stormCloud1
                wind:  false
                storm: ! animationsHalted
                cloudSpeed: 2
                opacity: 1
                xStart: -100
                xEnd: 500
                color1: Qt.darker("darkGray", 1.5)
                color2: Qt.darker("lightGray", 1.5)
            }
            PropertyChanges {
                target: stormCloud2
                wind:  false
                storm: ! animationsHalted
                cloudSpeed: 1.8
                opacity: 1
                xStart: -100
                xEnd: 500
                color1: Qt.darker("darkGray", 1.5)
                color2: Qt.darker("lightGray", 1.5)
            }
            PropertyChanges {
                target: cloud
                opacity: 1

                particleSize:  15
                particleCount: 30
                particleSource: "pics/particle/raindrop.png"
                particleStretch: 0.5
                particleLifeSpan: 600

                rain:  ! animationsHalted
                wind:  false
                storm: ! animationsHalted
                flash: false
                x: -24
                y: 14
                xStart: -100
                xEnd: 500
                width: 214
                height: 126
                color1: "#638299"
                color2: "#35464d"
                cloudSpeed: 1.5
            }
            PropertyChanges {
                target: weatherWrapper
                width: weatherView.width
                height: weatherView.height
            }
        }
    ]

    // when the state changes we also check for night/day time
    // so it shows either sun or moon
    onStateChanged: {
        sun.visible     = isDay
        moon.visible    = !isDay
    }

    // Every state change can be transitioned
    // A default NumberAnimation for the stated properties were applied
    transitions: [
        Transition {
            NumberAnimation {
                properties: "x,y,opacity,scale"
                duration: 1000
            }
        }
    ]
}
