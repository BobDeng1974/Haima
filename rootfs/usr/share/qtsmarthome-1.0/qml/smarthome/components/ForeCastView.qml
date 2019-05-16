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
* This item shows information about the forecast
*/
Item {
    id: foreCast

    property real wind: 5 // just some defaults
    property real rainProp: 10 // just some defaults
    property real rain: 10 // just some defaults

    // when the wind changes the desription will be adjusted
    onWindChanged: {
        var desc = ["still", "calm", "gentle breeze", "steady breeze", "stiff breeze", "windy"
                , "strong wind", "stormy", "hurricane"]
        var index = Math.floor( (wind / 12) * 9 )
        windDescription.text = desc[index]
    }

    // when the rain propability changes a corresponding image and
    // the correct probability of rain will be shown
    onRainPropChanged: {
        if( rainProp < 30 )
            rainPropability.source = "pics/icons/possibility_of_rain_low.png"
        else
            rainPropability.source = "pics/icons/possibility_of_rain_high.png"
        rainPropabilityText.text = Math.floor(rainProp) + "%"
    }
    // when the rain changes we show the amount as text
    onRainChanged: {
        rainAmountText.text = Math.floor(rain) + " ml/m²"
    }

    /**
    * All forecast informations are aligned in a grid.
    * 2 Columns are preset.
    */
    Grid {
        id: grid
        columns: 2
        spacing: 10
        /**
        * Image instance to show storm_force icon
        */
        Image {
            id: storm
            source: "pics/icons/storm_force.png"
        }
        /**
        * SHText instance to show wind description
        */
        SHText {
            id: windDescription
            font.pointSize: 21
            height: storm.height
        }
        /**
        * Image instance to show rain propability icon
        */
        Image {
            id: rainPropability
            source: "pics/icons/possibility_of_rain_low.png"
        }
        /**
        * SHText instance to show rain propability text
        */
        SHText {
            id: rainPropabilityText
            font.pointSize: 21
            height: rainPropability.height
        }
        /**
        * Image instance to show quantity of rain icon
        */
        Image {
            id: rainAmount
            source: "pics/icons/quantity_of_rain.png"
        }
        /**
        * SHText instance to show quantity of rain text
        */
        SHText {
            id: rainAmountText
            font.pointSize: 21
            height: rainAmount.height
        }
    }
    /**
    * A Column positioner is used here.
    */
    Column {
        x: 10
        y: 55
        spacing: 65
        /**
        * A simple white rectangle with a border is drawn
        * This is used as a seperator line
        */
        Rectangle {
            width: foreCast.width - 15
            height: 4; radius: 2
            color: "white"
            border {color: "lightGray"; width: 1}
        }
        Rectangle {
            width: foreCast.width - 15
            height: 4; radius: 2
            color: "white"
            border {color: "lightGray"; width: 1}
        }
    }
}
