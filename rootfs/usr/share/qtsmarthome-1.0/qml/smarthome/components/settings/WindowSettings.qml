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
import "../"
import "../tabwidget"


/**
* This Item shows all settings for the windows
* Mainly it contains a tabwidget (/see TabWidget)
* that contains separate settings for rain and wind
*/
SettingsItem {
    id: base

    property int itemIndex: 0

    property alias currentWindValue: windowWindTab.currentSliderValue
    property alias currentRainValue: windowRainTab.currentSliderValue

    /**
    * This function controls the logics.
    * This function will be called by every value change.
    * It checks the conditions to close/open the window
    */
    function checkState() {
        var tooMuchRain = windowRainTab.currentSliderActualValue > windowRainTab.currentSliderValue
        var tooMuchWind = windowWindTab.currentSliderActualValue > windowWindTab.currentSliderValue

        // if currentRain is higher than rainsetting -> close window
        // if wind force is stronger than wind setting -> close window
        // on is open
        hardwareModels.windowModel.setProperty( base.itemIndex, "on", !tooMuchRain && !tooMuchWind)
    }
    onCurrentWindValueChanged: {
        checkState()
    }
    onCurrentRainValueChanged: {
        checkState()
    }

    // is called when the itemindex is changed
    onItemIndexChanged: tabWidget.updateValues()

    Item {
        anchors.fill: parent
        id: contents

        /**
        * The TabWidget (\see TabWidget) is a custom item for a tabwidget like behaviour
        * It contains multiple items that can be accesed by clicking the tabs.
        */
        TabWidget
        {
            id: tabWidget

            /**
            * This function updates the items with the model values
            */
            function updateValues() {
                var cw  = hardwareModels.windowModel.get(itemIndex).closeWind
                windowWindTab.currentSliderValue = cw / 12 * 60

                var cr  = hardwareModels.windowModel.get(itemIndex).closeRain
                windowRainTab.currentSliderValue = cr
            }

            color: "transparent"
            width: parent.width
            height: parent.height
            anchors {
                leftMargin: 2
                rightMargin: 2
                topMargin: 10
                bottomMargin: 10
            }

            state: "init"

            // all pages of the tabwidget are declared here
            pages: [
                WindowRainTab {
                    id: windowRainTab
                    tabIndex: 0
                    tabIconPath: "../pics/icons/rain.png"
                    tabText: "Rain"
                    tabAlignment: tabWidget.alignRight
                    anchors.fill: parent
                    titleText: "Settings: Windows"
                    description: "Windows close automatically if it rains more than"
                    explanationText1: ""
                    explanationText2: ""
                    iconSource: hardwareModels.windowModel.get(base.itemIndex).on ?
                                    "../pics/icons/svg_icons/window_open.svg" : "../pics/icons/svg_icons/window_close.svg"
                    currentRainValue: currentRain
                },
                WindowWindTab {
                    id: windowWindTab
                    tabIndex: 1
                    tabIconPath: "../pics/icons/storm_force.png"
                    tabText: "Wind"
                    tabAlignment: tabWidget.alignRight
                    anchors.fill: parent
                    titleText: "Settings: Windows"
                    description: "Windows close automatically if wind force is higher than"
                    explanationText1: ""
                    explanationText2: ""
                    iconSource: hardwareModels.windowModel.get(base.itemIndex).on ?
                                    "../pics/icons/svg_icons/window_open.svg" : "../pics/icons/svg_icons/window_close.svg"
                    currentWindForceValue: currentWindForce
                }
            ]

            // There are just a init state, that will be called at the beginning once.
            // For initialisation purpose.
            // Definitely could be done differently.
            states: [
                State {
                    name: "init"
                    StateChangeScript {
                        script: tabWidget.updateValues()
                    }
                }
            ]
        }
    }
}
