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
import "../../components"
import "../tabwidget"

/**
* This Item shows all settings for the heating
* Mainly it contains a tabwidget (/see TabWidget)
* that contains separate settings for temperature and time
*/
SettingsItem {
    id: base

    property int itemIndex: 0

    /**
    * This function controls the logics.
    * This function will be called by every value change.
    * It checks the conditions to turn the heating on or off
    */
    function checkState() {
        var cTime = currentHour * 100 + currentMinute
        var dayNightTemp
        if((cTime > 1800) || (cTime < 600))
            dayNightTemp = temperatureTab.secondTemperature
        else
            dayNightTemp = temperatureTab.firstTemperature

        var temperatureOn = temperatureTab.actualTemperature < dayNightTemp
        var startHour = timepickerTab.currentStartTime1
        var startMinute = timepickerTab.currentStartTime2
        var endHour = timepickerTab.currentEndTime1
        var endMinute = timepickerTab.currentEndTime2

        var sTime = Math.floor(startHour * 100) + Math.floor(startMinute)
        var eTime = Math.floor(endHour * 100) + Math.floor(endMinute)

        var toggle = false
        if( sTime > eTime ) {
            toggle = true
            var help = sTime
            sTime = eTime
            eTime = help
        }

        var timeOn = (cTime >= sTime) && (cTime <= eTime)
        if( toggle ) {
            timeOn = !timeOn
        }

        // if heating is not off and temp is lower than currentTemp, turn it on
        hardwareModels.heatingModel.setProperty( base.itemIndex, "on", !timeOn && temperatureOn )
    }

    // is called when the itemindex is changed
    onItemIndexChanged: tabWidget.updateValues()

    /**
    * This Image Item shows weather the heating is on or off (with different images for on/off)
    */
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
                var startHour = hardwareModels.heatingModel.get(itemIndex).startTimeHours
                var startMinute = hardwareModels.heatingModel.get(itemIndex).startTimeMinutes
                var endHour = hardwareModels.heatingModel.get(itemIndex).endTimeHours
                var endMinute = hardwareModels.heatingModel.get(itemIndex).endTimeMinutes
                timepickerTab.setStartTime(startHour,startMinute)
                timepickerTab.setEndTime(endHour,endMinute)

                var dt  = hardwareModels.heatingModel.get(itemIndex).dayTemp
                var nt  = hardwareModels.heatingModel.get(itemIndex).nightTemp
                temperatureTab.firstTemperature = dt
                temperatureTab.secondTemperature = nt
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
                HeatingSettingsTab {
                    id: temperatureTab
                    tabIndex: 0
                    tabIconPath: "../pics/icons/temperature_high.png"
                    //tabText: "Temp"
                    tabText: hardwareModels.heatingModel.get(base.itemIndex).on ? "Temp ON" : "Temp OFF"
                    tabAlignment: tabWidget.alignRight
                    anchors.fill: parent
                    titleText: "Settings: Heating"
                    description: "Set heating temperature points"
                    iconSource: hardwareModels.heatingModel.get(base.itemIndex).on ?
                                    "../pics/icons/svg_icons/heating_on.svg" : "../pics/icons/svg_icons/heating_off.svg"
                    explanationText1: "Set-point day temperature"
                    explanationText2: "Set-point night temperature"
                    actualTemperature: currentTemperature
                    onCheckStateTriggered: checkState()
                },
                TimePickerTab {
                    id: timepickerTab
                    tabIndex: 1
                    tabIconPath: "../pics/icons/time.png"
                    tabText: "Time"
                    tabAlignment: tabWidget.alignRight
                    anchors.fill: parent
                    titleText: "Settings: Heating"
                    description: "Heating shut down between"
                    explanationText1: ""
                    explanationText2: ""
                    iconSource: hardwareModels.heatingModel.get(base.itemIndex).on ?
                                    "../pics/icons/svg_icons/heating_on.svg" : "../pics/icons/svg_icons/heating_off.svg"
                    onCheckStateTriggered: checkState()
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
