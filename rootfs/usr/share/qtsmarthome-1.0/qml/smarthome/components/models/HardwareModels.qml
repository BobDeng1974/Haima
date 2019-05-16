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
* This Item defines all hardware models used in smarthome.
* Hardware means something as Lights, Heating, Blinds, Windows.
* The default values and icons for every hardware entity are defined here.
* In models you can define any property you need.
* This feature is used here alot.
* These models and its infos are used mainly in FanStack (/see FanStack)
* and in all settings in the components/settings folder.
*/
Item {
    property alias lightModel: light
    property alias heatingModel: heating
    property alias blindModel: blind
    property alias windowModel: window

    /**
    * A model for all lights in the house that can be controlled
    */
    ListModel {
        id: light

        ListElement {
            name: "Bedroom"
            iconOn: "pics/icons/light_on.png"
            iconOff: "pics/icons/light_off.png"
            on: true
            startTimeHours: 7
            endTimeHours: 19
            startTimeMinutes: 00
            endTimeMinutes: 45
            lowestLux: 6
        }

        ListElement {
            name: "Hall"
            iconOn: "pics/icons/light_on.png"
            iconOff: "pics/icons/light_off.png"
            on: false
            startTimeHours: 7
            endTimeHours: 19
            startTimeMinutes: 00
            endTimeMinutes: 45
            lowestLux: 7
        }

        ListElement {
            name: "Bathroom"
            iconOn: "pics/icons/light_on.png"
            iconOff: "pics/icons/light_off.png"
            on: false
            startTimeHours: 6
            endTimeHours: 18
            startTimeMinutes: 00
            endTimeMinutes: 45
            lowestLux: 4
        }
    }

    /**
    * A model for all heatings in the house that can be controlled
    */
    ListModel {
        id: heating

        ListElement {
            name: "Entrance"
            iconOn: "pics/icons/radiator_on.png"
            iconOff: "pics/icons/radiator_off.png"
            on: true
            startTimeHours: 20
            endTimeHours: 7
            startTimeMinutes: 00
            endTimeMinutes: 00
            dayTemp: 24
            nightTemp: 14
        }

        ListElement {
            name: "Hall"
            iconOn: "pics/icons/radiator_on.png"
            iconOff: "pics/icons/radiator_off.png"
            on: true
            startTimeHours: 19
            endTimeHours: 6
            startTimeMinutes: 00
            endTimeMinutes: 30
            dayTemp: 24
            nightTemp: 14
        }

        ListElement {
            name: "Kitchen"
            iconOn: "pics/icons/radiator_on.png"
            iconOff: "pics/icons/radiator_off.png"
            on: false
            startTimeHours: 19
            endTimeHours: 10
            startTimeMinutes: 00
            endTimeMinutes: 55
            dayTemp: 24
            nightTemp: 14
        }

        ListElement {
            name: "Kids"
            iconOn: "pics/icons/radiator_on.png"
            iconOff: "pics/icons/radiator_off.png"
            on: false
            startTimeHours: 19
            endTimeHours: 6
            startTimeMinutes: 00
            endTimeMinutes: 20
            dayTemp: 24
            nightTemp: 14
        }
    }

    /**
    * A model for all blinds in the house that can be controlled
    */
    ListModel {
        id: blind

        ListElement {
            name: "Kitchen"
            iconOn: "pics/icons/jalousie_down.png"
            iconOff: "pics/icons/jalousie_up.png"
            on: true
            startTimeHours: 19
            endTimeHours: 7
            startTimeMinutes: 00
            endTimeMinutes: 45
            closeTemp: 22
            closeWind: 7
            automaticWindClose: true
        }

        ListElement {
            name: "Bedroom"
            iconOn: "pics/icons/jalousie_down.png"
            iconOff: "pics/icons/jalousie_up.png"
            on: false
            startTimeHours: 21
            endTimeHours: 10
            startTimeMinutes: 00
            endTimeMinutes: 30
            closeTemp: 18
            closeWind: 6
            automaticWindClose: false
        }
    }

    /**
    * A model for all windows in the house that can be controlled
    */
    ListModel {
        id: window

        ListElement {
            name: "Kitchen"
            iconOn: "pics/icons/window_open.png"
            iconOff: "pics/icons/window_closed.png"
            on: false
            closeWind: 6
            closeRain: 32
        }

        ListElement {
            name: "Bedroom"
            iconOn: "pics/icons/window_open.png"
            iconOff: "pics/icons/window_closed.png"
            on: true
            closeWind: 7
            closeRain: 38
        }

        ListElement {
            name: "Library"
            iconOn: "pics/icons/window_open.png"
            iconOff: "pics/icons/window_closed.png"
            on: false
            closeWind: 6
            closeRain: 18
        }
    }
}
