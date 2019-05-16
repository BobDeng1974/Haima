/** This file is part of Medical Device Qt Quick Embedded Widgets Demo**
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
This file defines the multislider widget. It provides a fill bar that shows the
value currently set. The fill bar basically is a svg-image whose y-coordinate
is manipulated to make the bar larger/smaller. This functionality is provided
by a c++-plugin that modifies the image accordingly.Two sliders are used to
define an interval in which the fill bar has to be.

property int upperLimit                 The upper limit of the multislider.
property int lowerLimit                 The lower limit of the multislider.
property int sliderMinValue             The value of the lower slider.
property int sliderMaxValue             The value of the upper slider.
property int setValue                   The current value of the filling bar.
property alias fillTopImage             The image for the top of the filling bar.
property alias fillBottomImage          The image for the bottom of the filling bar.
property alias fillBackgroundImage      The image for the background of the filling bar.
property alias fillImage                The image of the actual filling bar.
property alias sliderBackground         The image for the slider background.
property string sliderMaxHoveredImage   The upper sliders' image when hovered.
property string sliderMaxPressedImage   The upper sliders' image when pressed.
property string sliderMaxImage          The upper sliders' normal image.
property string sliderMinHoveredImage   The lower sliders' image when hovered.
property string sliderMinPressedImage   The lower sliders' image when pressed.
property string sliderMinImage          The lower sliders' normal image.
*/
Item {
    id: multislider

    property int upperLimit: 0
    property int lowerLimit: 0
    property int sliderMinValue: 0
    property int sliderMaxValue: 0
    property int setValue: 0
    property alias fillTopImage:  dripRateValueBarTop.source
    property alias fillBottomImage:  dripRateValueBarBottom.source
    property alias fillBackgroundImage:  dripRateValueBarCenter.source
    property alias fillImage: fillingState.source
    property alias sliderBackground: background.source
    property string sliderMaxHoveredImage: "../pics/multislider/Beryl/slider_max_hovered.svg"
    property string sliderMaxPressedImage: "../pics/multislider/Beryl/slider_max_pressed.svg"
    property string sliderMaxImage: "../pics/multislider/Beryl/slider_max.svg"
    property string sliderMinHoveredImage: "../pics/multislider/Beryl/slider_min_hovered.svg"
    property string sliderMinPressedImage: "../pics/multislider/Beryl/slider_min_pressed.svg"
    property string sliderMinImage: "../pics/multislider/Beryl/slider_min.svg"

    Row {
        id: row

        property int rightSpacing: 15

        anchors.fill: parent
        spacing: parent.width - dripRateValueBar.width - background.width - rightSpacing

        Item {
            id: dripRateValueBar
            width: 20
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            Image {
                id: dripRateValueBarCenter
                anchors.fill: parent
                smooth: true
                source: "../pics/multislider/Beryl/valuebar.svg"
            }

            Image {
                id: dripRateValueBarTop
                anchors.top: parent.top
                width: parent.width
                height: 13
                smooth: true
                source: "../pics/multislider/Beryl/valuebar_top.svg"
            }

            Image {
                id: dripRateValueBarBottom
                anchors.bottom: parent.bottom
                width: parent.width
                height: 13
                smooth: true
                source: "../pics/multislider/Beryl/valuebar_bottom.svg"
            }

            Image {
                id: fillingState
                y: (sliderMaxMouseArea.deltaY - (setValue / upperLimit) * sliderMaxMouseArea.deltaY - sliderMaxMouseArea.imageOffset)
                height: dripRateValueBarBottom.y - y
                width: parent.width
                source: "../pics/multislider/Beryl/valuebar_filled.svg"
            }
        }

        Image {
            id: background
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 16
            smooth: true
            source: "../pics/multislider/Beryl/groove.svg"

            // this image defines the upper slider
            Image {
                id: sliderMax

                property bool hover: false

                y: sliderMaxMouseArea.baseMaxY - (sliderMaxValue / upperLimit * (sliderMaxMouseArea.baseMaxY - sliderMaxMouseArea.drag.minimumY))
                width: 45
                height: 28
                sourceSize.height: height
                sourceSize.width: width
                anchors.horizontalCenterOffset: -3
                anchors.horizontalCenter: background.horizontalCenter
                smooth: true
                source: hover ? sliderMaxHoveredImage : sliderMaxImage

                states: [
                    State {
                        name: "clicked"
                        PropertyChanges {
                            target: sliderMax
                            source: sliderMaxPressedImage
                        }
                    }
                ]

                MouseArea {
                    id: sliderMaxMouseArea

                    property int imageOffset: 4 // there is an offset in the image to the bottom line
                    property int offset: 7      // there is an offset in the image to the bottom line
                    property int maxY: sliderMin.y - sliderMin.height + imageOffset
                    property int baseMaxY: background.height - sliderMax.height - offset
                    property int baseMinY: - (sliderMax.height - dripRateValueBarTop.height)
                    property int deltaY: baseMaxY - baseMinY

                    anchors.fill: parent
                    hoverEnabled: true
                    drag.axis: Drag.YAxis
                    drag.target: parent
                    drag.minimumY: baseMinY
                    drag.maximumY: maxY

                    onEntered: {
                        sliderMax.hover = true
                    }
                    onExited: {
                        sliderMax.hover = false
                        sliderMax.state = ""
                    }
                    onPressed: {
                        sliderMax.state = "clicked"
                    }
                    onReleased: {
                        sliderMax.state = ""
                    }
                    onPositionChanged: {
                        sliderMaxValue = (baseMaxY - parent.y) / (baseMaxY - drag.minimumY) * upperLimit
                    }
                }
            }

            // this image defines the lower slider
            Image {
                id: sliderMin

                property bool hover: false

                y: sliderMinMouseArea.baseMaxY - (sliderMinValue / upperLimit * (sliderMinMouseArea.baseMaxY - sliderMinMouseArea.baseMinY))
                width: 45
                height: 28
                sourceSize.height: height
                sourceSize.width: width
                anchors.horizontalCenterOffset: -2
                anchors.horizontalCenter: background.horizontalCenter
                smooth: true
                source: hover ? sliderMinHoveredImage : sliderMinImage

                states: [
                    State {
                        name: "clicked"
                        PropertyChanges {
                            target: sliderMin
                            source: sliderMinPressedImage
                        }
                    }
                ]

                MouseArea {
                    id: sliderMinMouseArea

                    property int imageOffset: 4 // there is an offset in the image to the top line
                    property int offset: 9      // there is an offset in the image to the top line
                    property int minY: sliderMax.y + sliderMax.height - imageOffset
                    property int baseMinY: offset
                    property int baseMaxY: background.height -  sliderMin.height + dripRateValueBarTop.height
                    property int deltaY: baseMaxY - baseMinY

                    anchors.fill: parent
                    hoverEnabled: true
                    drag.axis: Drag.YAxis
                    drag.target: parent
                    drag.minimumY: minY
                    drag.maximumY: baseMaxY

                    onEntered: {
                        sliderMin.hover = true
                    }
                    onExited: {
                        sliderMin.hover = false
                        sliderMin.state = ""
                    }
                    onPressed: {
                        sliderMin.state = "clicked"
                    }
                    onReleased: {
                        sliderMin.state = ""
                    }
                    onPositionChanged: {
                        sliderMinValue = (baseMaxY - parent.y)/(baseMaxY - baseMinY) * upperLimit
                    }
                }
            }
        }
    }

}

