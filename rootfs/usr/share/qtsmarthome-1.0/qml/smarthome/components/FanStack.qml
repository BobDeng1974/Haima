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
This component defines a fannable stack of QML elements. When the user clicks on the stack, it
spreads out to reveal the stacked elements. Clicking on such an element will bring up an additional
element with arbitraty contents.

Properties:
int animationDuration:  The duration of the animated transition effects
bool inactive:          Controls whether this stack is "inactive". Inactive stacks are drawn semi
                        transparent.
bool doFanOut:          Controls whether this stack is fanned out.
variant model:          The model to be used by this stack's elements
itemlist pages:         Access to the stack elements from outside this component.
bool flipped:           A flag indicating whether the additional QML element is currently shown
string backPage:        The url to a QML element to be shown on the backside of a stack element

Methods:
checkStates():          Calls the "checkState()" function of all backside elements. This is called
                        periodically by a Timer element.
Signals:
fanOut(string name):    Emitted when the stack fans out. "name" is the object name of this FanStack

*/
Rectangle {
    id: fanStack

    property int animationDuration: 1200
    property int index: 0
    property bool inactive: false
    property alias doFanOut: stack.fanOut

    property alias model: rep.model
    property alias pages: stack.children
    property bool flipped: false

    property string backPage: ""

    property int flippedXPos: 0
    property int flippedYPos: 0
    property int flippedWidth: parent.width
    property int flippedHeight: parent.height
    property int xSpacing: 0
    property int ySpacing: 0
    property int startingFlipppedWidth: 0

    property int currentMinute: 0
    property int currentHour: 0
    property int currentRain: 0
    property int currentWindForce: 0
    property int currentLux: 0
    property int currentTemperature: 0

    signal fanOut(string name)

    /*
    When an element is clicked and the stack is fanned in, "fanOut" is emitted and the stack is
    fanned out.
    */
    function elementClicked(index) {
        if ( !stack.fanOut ) {
            stack.fanOut = true
            inactive = false
            fanOut(objectName)
        }
    }

    function origState() {
        for( var i = 0; i < pages.length; i++ ) {
            if( pages[i].objectName == "flipable")
                pages[i].state = ""
        }
    }

    /*
    This function dispatches a call to checkState to all back side elements.
    */
    function checkStates() {
        for( var i = 0; i < pages.length; i++ ) {
            if( (pages[i].objectName == "flipable") && (pages[i].state == "flipped"))
                pages[i].checkState()
        }
        for( var i = 0; i < pages.length; i++ ) {
            if( (pages[i].objectName == "flipable") && (pages[i].state != "flipped"))
                pages[i].checkState()
        }
    }

    function rgb(r,g,b) {
        return Qt.rgba( r / 255, g / 255, b / 255, 1)
    }

    color: Qt.rgba(0, 0, 0, 0)

    Component {
        id: stackDelegate

        /*
        The actual stack elements are implemented by this delegate component using a Flippable QML
        element as base (all stack elements actually ARE flippable).
        */
        Flipable {
            id: flipable
            /*
            This function dispatchs the checkState() call to the back side element, if it is
            loaded and ready.
            */
            function checkState() {
                if( loader.status == Loader.Ready ) {
                    loader.item.checkState()
                    loader.item.currentMinute = fanStack.currentMinute
                    loader.item.currentHour = fanStack.currentHour
                    loader.item.currentRain = fanStack.currentRain
                    loader.item.currentWindForce = fanStack.currentWindForce
                    loader.item.currentLux = fanStack.currentLux
                    loader.item.currentTemperature = fanStack.currentTemperature
                }
                return true
            }
            /*
            The object name is set to "flipable" to be able to identify this element later.
            */
            objectName: "flipable"


            state: ""
            /*
            Calculate initial values for the x and y coordinate depenting on the internal index of
            this element in the stack
            */
            x: index * 10
            y: index * 10

            width: 100
            height: 100

            /*
            This property is used for the rotation animation effect
            */
            property int angle: 0
            transform: Rotation {
                id: rotation
                axis {x:0; y:1; z:0}
                angle: flipable.angle
            }

            /*
            The front side of the Flipable element is made up from a Rectangle element containing
            a Text, an Image and a MouseArea
            */
            front:  Rectangle {
                id: rect
                radius: 4
                smooth: smartHome.smooth
                anchors.fill: flipable
                color: Qt.rgba(0.1, 0.1, 0.1, 1.0)
                border.width: 4
                border.color: "lightgray"
                SHText {
                    id: itemText
                    text: name
                    color: "white"
                    font.pointSize: 14
                    font.bold: false
                    x: (parent.width - width) / 2
                    y: 3
                }
                Image {
                    id: image
                    source: on ? iconOn : iconOff
                    anchors.centerIn: parent
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        if( stack.fanOut && loader.status == Loader.Ready && !fanStack.flipped ) {
                            flipable.state = "flipped"
                            fanStack.flipped = true
                        }

                        elementClicked(index)
                    }
                }
            }
            /*
            Since the back side is a dynamic element which is not mandatory to exist at all, a QML
            Loader element is used here to conveniently handle the on-demand-loading of QML
            components.
            */
            back: Loader {
                id: loader
                source: fanStack.backPage
                width: flippedWidth
                height: flippedHeight
                onStatusChanged: {
                    if( status == Loader.Ready ) {
                        item.itemIndex = index

                        rotation.origin.x = loader.item.width/2
                        rotation.origin.y = loader.item.height/2

                        item.currentMinute = fanStack.currentMinute
                        item.currentHour = fanStack.currentHour
                        item.currentRain = fanStack.currentRain
                        item.currentWindForce = fanStack.currentWindForce
                        item.currentLux = fanStack.currentLux
                        item.currentTemperature = fanStack.currentTemperature
                    }
                }

                /*
                eat mouse events
                */
                MouseArea {
                    anchors.fill: loader.item
                }
            }

            /*
            Three states are defined for the stack delegate:
            - the default state
            - the "fanOut" state
            - and finally the "flipped" state
            */

            states: [
                /*
                State characteristics:
                - Delegate text at top of element
                - Rotation angle 0
                - x and y coordinates depending on internal index
                */
                State {
                    name: ""
                    when: !stack.fanOut
                    PropertyChanges {
                        target: flipable
                        x: index * 10
                        y: index * 10
                        angle: 0
                        rotation: 0
                    }
                    PropertyChanges {
                        target: fanStack
                        x: parent.calculateX(fanStack.index, fanStack.parent.children.length)
                    }
                    PropertyChanges {
                        target: itemText
                        x: (parent.width - width) / 2
                        y: 2
                    }
                },

                /*
                State characteristics:
                - Delegate text at bottom of element
                - Rotation angle 360 (animated to create a full rotation)
                - x and y coordinates calculated to fit the screen
                - background color opaque
                */
                State {
                    name: "fanOut"
                    when: stack.fanOut
                    PropertyChanges {
                        target: flipable
                        x: menu.calculateX( index, rep.model.count )
                        y: ( ( rep.model.count - 1) * 10 ) / 2
                        rotation: 360
                    }
                    PropertyChanges {
                        target: rect
                        color: Qt.rgba(0.1, 0.1, 0.1, 1)
                    }

                    PropertyChanges {
                        target: fanStack
                        x: 0
                        flipped: false
                    }
                    PropertyChanges {
                        target: itemText
                        x: (rect.width - itemText.width) / 2
                        y: rect.height - itemText.height - 1
                    }
                },
                /*
                State characteristics:
                - angle of the flippable set to 180 (animated to show the "flipping" of the element)
                - Rotation angle 0
                - x and y coordinates calculated to fit the screen
                */
                State {
                    name: "flipped"
                    PropertyChanges {
                        target: flipable
                        angle: 180
                        x: flippedXPos + flippedWidth - startingFlipppedWidth + xSpacing
                        y: flippedYPos + ySpacing
                        z: 10
                        rotation: 0
                    }
                }
            ]
            /*
            Transitions for all properties that change in a state to make everything look smooth
            */
            transitions: [
                Transition {
                    from: ""
                    to: "fanOut"
                    reversible: true
                    NumberAnimation {
                        properties: "x,y,rotation"
                        duration: fanStack.animationDuration
                        easing.type: Easing.InOutQuad
                    }
                },
                Transition {
                    from: "fanOut"
                    to: "flipped"
                    reversible: true

                        SequentialAnimation {
                            NumberAnimation {
                                id: flipAnimation

                                //NOTE: By adding the line "to: 100" we achieve much smoother animation on slow hardware
                                to: 100

                                properties: "z,x,y,angle"
                                duration: fanStack.animationDuration
                                easing.type: Easing.InOutQuad
                            }
                        }
                },
                Transition {
                    from: "flipped"
                    to: ""
                        SequentialAnimation {

                                NumberAnimation {
                                    properties: "x,y"
                                }
                            NumberAnimation {
                                properties: "x,y,z,angle"
                                duration: fanStack.animationDuration
                                easing.type: Easing.InOutQuad
                            }
                        }
                }
            ]
        }
    }

    /*
    This Item element is actually a container for the stack elements. It uses a QML Repeater element
    to create the stack elements based on a given model.
    */
    Item {
        id: stack
        property bool fanOut: false

        Repeater {
            id: rep
            delegate: stackDelegate
        }
        Component.onCompleted: {
            fanStack.width = ( rep.model.count - 1 ) * 10 + 100
            fanStack.height = ( rep.model.count - 1 ) * 10 + 100
        }
    }

    /*
    Each FanStack can either be "active" or "inactive". Since "active" is considered the default
    state, we only need to define the "inactive" state.
    */
    states: [
        State {
            name: "inactive"
            when: inactive && !stack.fanOut
            PropertyChanges {
                target: fanStack
                y: 50
                z: -10
                opacity: 0.9
            }
        }
    ]

    transitions:
        Transition {
            reversible: true
            NumberAnimation {
                properties: "z,y,opacity"
                duration: 400
            }
        }

    /*
    Some magic to store the index of this Stack in it's parent. Once determined, the index is stored
    in the custom property "index" of this component.
    */
    Component.onCompleted: {
        for( var i = 0; i < parent.children.length; i++ ) {
            if ( parent.children[i].objectName == objectName ) {
                index = i
                break
            }
        }
        startingFlipppedWidth = flippedWidth
    }
}
