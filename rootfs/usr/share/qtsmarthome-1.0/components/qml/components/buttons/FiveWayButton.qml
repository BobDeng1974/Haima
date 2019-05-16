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
This file provides a five-way-button. It is used to change the display mode of
the heart rate graph. If a button is pressed a signal is sent that the graph
can receive and pass on to the c++-plugin. Regrettably all five buttons are not
able to imitate the the behavior of their c++-counterparts. Qt provides a
mechanism that can filter mouse clicks based on alpha values of the image (like
a round button that has an rectangular image where not every part of the image
is actually belonging to the button). This pixel precise picking is currently
not possible with QML (the feature is being voted for inclusion in a future
release) or only can be approximated, for example by using more than one mouse
area to define a button. In this case the round buttons are only approximated
by one box which is still large enough to click the buttons. This solution
might not be fitting when using more exotically shaped buttons.

property alias backgroundImage          The background image for the buttons.
property alias centerButtonImageNormal  The image for the center button in normal state.
property alias centerButtonImageHovered The image for the center button when hovered.
property alias centerButtonImagePressed The image for the center button when pressed.
property alias upperButtonImageNormal   The image for the upper button in normal state.
property alias upperButtonImageHovered  The image for the upper button when hovered.
property alias upperButtonImagePressed  The image for the upper button when pressed.
property alias rightButtonImageNormal   The image for the right button in normal state.
property alias rightButtonImageHovered  The image for the right button when hovered.
property alias rightButtonImagePressed  The image for the right button when pressed.
property alias lowerButtonImageNormal   The image for the lower button in normal state.
property alias lowerButtonImageHovered  The image for the lower button when hovered.
property alias lowerButtonImagePressed  The image for the lower button when pressed.
property alias leftButtonImageNormal    The image for the left button in normal state.
property alias leftButtonImageHovered   The image for the left button when hovered.
property alias leftButtonImagePressed   The image for the left button when pressed.

signal centerButtonPressed()    A signal that is sent when the center button was clicked.
signal upperButtonPressed()     A signal that is sent when the upper button was clicked.
signal rightButtonPressed()     A signal that is sent when the right button was clicked.
signal lowerButtonPressed()     A signal that is sent when the lower button was clicked.
signal leftButtonPressed()      A signal that is sent when the left button was clicked.

*/
Item {
    id: fiveWayButton

    property alias backgroundImage:          background.imageSource
    property alias centerButtonImageNormal:  centerButton.imageSourceNormal
    property alias centerButtonImageHovered: centerButton.imageSourceHovered
    property alias centerButtonImagePressed: centerButton.imageSourcePressed
    property alias upperButtonImageNormal:   upperButton.imageSourceNormal
    property alias upperButtonImageHovered:  upperButton.imageSourceHovered
    property alias upperButtonImagePressed:  upperButton.imageSourcePressed
    property alias rightButtonImageNormal:   rightButton.imageSourceNormal
    property alias rightButtonImageHovered:  rightButton.imageSourceHovered
    property alias rightButtonImagePressed:  rightButton.imageSourcePressed
    property alias lowerButtonImageNormal:   lowerButton.imageSourceNormal
    property alias lowerButtonImageHovered:  lowerButton.imageSourceHovered
    property alias lowerButtonImagePressed:  lowerButton.imageSourcePressed
    property alias leftButtonImageNormal:    leftButton.imageSourceNormal
    property alias leftButtonImageHovered:   leftButton.imageSourceHovered
    property alias leftButtonImagePressed:   leftButton.imageSourcePressed

    signal centerButtonPressed()
    signal upperButtonPressed()
    signal rightButtonPressed()
    signal lowerButtonPressed()
    signal leftButtonPressed()

    width: 150
    height: 150

    Image {
        id: background

        property string imageSource

        width: parent.width
        height: parent.height
        sourceSize.height: height
        sourceSize.width: width
        anchors.fill: parent
        source: imageSource

        // this image defines the center button
        Image {
            id: centerButton

            property bool hover: false
            property string imageSourceNormal
            property string imageSourceHovered
            property string imageSourcePressed

            width: 134
            height: 134
            sourceSize.height: height
            sourceSize.width: width
            anchors.centerIn: parent
            source: hover ? imageSourceHovered : imageSourceNormal

            states: [
                State {
                    name: "clicked"
                    PropertyChanges {
                        target: centerButton
                        source: imageSourcePressed
                    }
                }
            ]

            // the center buttons' mouse area
            MouseArea {
                width: 38
                height: 38
                anchors.centerIn: parent
                hoverEnabled: true

                onEntered: {
                    centerButton.hover = true
                }
                onExited: {
                    centerButton.hover = false
                    centerButton.state = ""
                }
                onPressed: {
                    centerButton.state = "clicked"
                    fiveWayButton.centerButtonPressed()
                }
                onReleased: {
                    centerButton.state = ""
                }
            }
        }

        // this image defines the upper button
        Image {
            id: upperButton

            property bool hover: false
            property string imageSourceNormal
            property string imageSourceHovered
            property string imageSourcePressed

            anchors.top: parent.top
            anchors.topMargin: 15
            anchors.horizontalCenter: parent.horizontalCenter
            width: 120
            height: 60
            sourceSize.width: width
            sourceSize.height: height
            source: hover ? imageSourceHovered : imageSourceNormal

            states: [
                State {
                    name: "clicked"
                    PropertyChanges {
                        target: upperButton
                        source: imageSourcePressed
                    }
                }
            ]

            // the upper buttons' mouse area
            MouseArea {
                width: 58
                height: 31
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                hoverEnabled: true

                onEntered: {
                    upperButton.hover = true
                }
                onExited: {
                    upperButton.hover = false
                    upperButton.state = ""
                }
                onPressed: {
                    upperButton.state = "clicked"
                    fiveWayButton.upperButtonPressed()
                }
                onReleased: {
                    upperButton.state = ""
                }
            }
        }

        // this image defines the right button
        Image {
            id: rightButton

            property bool hover: false
            property string imageSourceNormal
            property string imageSourceHovered
            property string imageSourcePressed

            anchors.right: parent.right
            anchors.rightMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            width: 60
            height: 120
            sourceSize.height: height
            sourceSize.width: width
            source: hover ? imageSourceHovered : imageSourceNormal

            states: [
                State {
                    name: "clicked"
                    PropertyChanges {
                        target: rightButton
                        source: imageSourcePressed
                    }
                }
            ]

            // the right buttons' mouse area
            MouseArea {
                width: 33
                height: 55
                anchors.verticalCenter: parent.verticalCenter
                anchors.right: parent.right
                hoverEnabled: true

                onEntered: {
                    rightButton.hover = true
                }
                onExited: {
                    rightButton.hover = false
                    rightButton.state = ""
                }
                onPressed: {
                    rightButton.state = "clicked"
                    fiveWayButton.rightButtonPressed()
                }
                onReleased: {
                    rightButton.state = ""
                }
            }
        }

        // this image defines the lower button
        Image {
            id: lowerButton

            property bool hover: false
            property string imageSourceNormal
            property string imageSourceHovered
            property string imageSourcePressed

            anchors.bottom: parent.bottom
            anchors.bottomMargin: 15
            anchors.horizontalCenter: parent.horizontalCenter
            width: 120
            height: 60
            sourceSize.width: width
            sourceSize.height: height
            source: hover ? imageSourceHovered : imageSourceNormal

            states: [
                State {
                    name: "clicked"
                    PropertyChanges {
                        target: lowerButton
                        source: imageSourcePressed
                    }
                }
            ]

            // the lower buttons' mouse area
            MouseArea {
                width: 54
                height: 34
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.bottom: parent.bottom
                hoverEnabled: true

                onEntered: {
                    lowerButton.hover = true
                }
                onExited: {
                    lowerButton.hover = false
                    lowerButton.state = ""
                }
                onPressed: {
                    lowerButton.state = "clicked"
                    fiveWayButton.lowerButtonPressed()
                }
                onReleased: {
                    lowerButton.state = ""
                }
            }
        }

        // this image defines the left button
        Image {
            id: leftButton

            property bool hover: false
            property string imageSourceNormal
            property string imageSourceHovered
            property string imageSourcePressed

            anchors.left: parent.left
            anchors.leftMargin: 15
            anchors.verticalCenter: parent.verticalCenter
            width: 60
            height: 120
            sourceSize.height: height
            sourceSize.width: width
            source: hover ? imageSourceHovered : imageSourceNormal

            states: [
                State {
                    name: "clicked"
                    PropertyChanges {
                        target: leftButton
                        source: imageSourcePressed
                    }
                }
            ]

            // the left buttons' mouse area
            MouseArea {
                width: 30
                height: 55
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                hoverEnabled: true

                onEntered: {
                    leftButton.hover = true
                }
                onExited: {
                    leftButton.hover = false
                    leftButton.state = ""
                }
                onPressed: {
                    leftButton.state = "clicked"
                    fiveWayButton.leftButtonPressed()
                }
                onReleased: {
                    leftButton.state = ""
                }
            }
        }
    }
}





