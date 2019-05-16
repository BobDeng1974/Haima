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

/*
* The Windslider customizes the ActualValueSlider
* Mainly it sets the type to "Wind" and adds fancy ballon texts.
*/
ActualValueSlider {
    //type: "wind"
    actualValue: 11

    width: 520
    valueSliderHeight: 120
    backgroundImage: "pics/wind/wind_scale.png"

    /**
     * style the balloon
     */
    balloonHeight: 56
    balloonTriangleHeight: 20
    balloonImage: "pics/wind/balloon/balloon_minsize.svg"
    balloonTriangleImage: "pics/wind/balloon/balloon_triangle.svg"

    /**
     * style the knob
     */
    knobImagePressed: "pics/fader/button_pressed.svg"
    knobImageNormal: "pics/fader/button_normal.svg"

    /**
     * style the needle
     */
    needleImage: "pics/wind/needle.svg"

    /**
     * style the focus
     */
    focusVisible: false
    focusImage: ""

    value: 40
    balloonText: "strong breeze"
    minValue: 0
    maxValue: 60

    // Every value change will change the ballontext
    onValueChanged: {
        if(value < 10)
            balloonText = "calm"
        else if(value <= 20)
            balloonText = "gentle breeze"
        else if(value <= 30)
            balloonText = "breeze"
        else if(value <= 40)
            balloonText = "strong breeze"
        else if(value <= 50)
            balloonText = "stormy wind"
        else
            balloonText = "hurricane"
    }
}
