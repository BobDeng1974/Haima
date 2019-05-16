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
 * The TimePicker component is a widget to let the user select numbers (or, more unspecific,
 * elements) from a device with movable barrels. Currently two barrels are available. The elements
 * to be displayed can be set through properties. The component supports kinetic scrolling through
 * the QML PathView element.
 *
 * To create an another picker for different numbers, you can adapt this example.
 * You need to set a different model for your desired numbers as shown in Item with id "leftBarrel".
 * Maybe you would like to have a picker for a single number. In that case you could make the second "barrel" invisible
 * and/or maybe change the background image to fit your needs. The second PickerPathView instance isn't needed then.
 *
 * Properties:
 * variant currentValue1:     The current element on the first barrel
 * variant currentValue2:     The current element on the second barrel
 * string caption:            The caption of the widget
 * model leftBarrelModel:  The model to be used by the first PathView. The format of the model
 *                            is a list of strings.
 * model rightBarrelModel: The model to be used by the second PathView. The format of the model
 *                            is a list of strings.
 *
 * Signals:
 * valueChanged()           Emitted when either value changes
 */
Rectangle {
    id: timePicker

    property variant currentValue1: leftBarrel.currentValue
    property variant currentValue2: rightBarrel.currentValue
    property string caption: "Time Picker"
    property alias leftBarrelModel: leftBarrel.model
    property alias rightBarrelModel: rightBarrel.model
    property alias backgroundImagePath: backgroundImage.source

    signal valueChanged

    function setCurrentValues(value1,value2) {
        leftBarrel.setCurrentValue(value1)
        rightBarrel.setCurrentValue(value2)
    }

    width: backgroundImage.width
    height: backgroundImage.height
    color: "transparent"

    onCurrentValue1Changed: {
        valueChanged()
    }
    onCurrentValue2Changed: {
        valueChanged()
    }

    Image {
        id: backgroundImage
        x: 0
        y: 0
        source: "../pics/timer/double_picker_background.svg"

        /*
         * The TimePicker uses a PathView to animate the elements. The part of the view which
         * lies directly over the barrel should be visible. The rest must be clipped. This is
         * done using an ordinaty QML Item. The Item acts as the clipping area.
         */
        Item {
            id: maskItem
            x: 15
            y: 47
            width: 200
            height: 200

            /*
             * Set clip to true to enable clipping of child items.
             */
            clip: true

            PickerPathView {
                id: leftBarrel
                x: 23
                model: ["00","01","02","03","04","05","06","07","08","09","10","11","12",
                        "13","14","15","16","17","18","19","20","21","22","23"]
            }

            PickerPathView {
                id: rightBarrel
                x: 118
                width: 58
                height: 800
            }
        }

        /*
         * The caption
         */
        Text {
            id: caption
            x: 15
            y: 11
            width: 200
            height: 20
            color: "#ffffff"
            text: timePicker.caption
            font.bold: true
            font.pointSize: 13
            verticalAlignment: "AlignVCenter"
            horizontalAlignment: "AlignHCenter"
        }

    }

    /*
     * To finish the look of the element, a semi transparent graphic is rendered on top of the
     * barrels:
     */
    Image {
        id: image1
        x: 0
        y: 0
        source: "../pics/timer/double_picker_reflex.svg"
    }
}
