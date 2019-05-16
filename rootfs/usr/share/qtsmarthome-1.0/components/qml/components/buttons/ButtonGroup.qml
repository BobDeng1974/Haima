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
This defines a visual element containing the buttons to switch between the tabs
and some spacers between the buttons. When a button in the group is clicked
every other button has to be disabled. A javascript function is used to
propagate the change. The currently set button is made visible to the outside
via currentIndex.
*/
Item {
    id: buttonGroup

    property int currentIndex: 0

    function updateTabs(index)
    {
        var i = 0;
        for(;i < children.length; i++)
        {
            if(children[i].objectName == "tabButton")
            {
                if(children[i].index != index)
                {
                    children[i].active = false
                }
            }
        }
        buttonGroup.currentIndex = index
    }

    Rectangle {
        id: leftSpacer
        anchors.top: parent.top
        anchors.topMargin: 26
        width: 231
        height: 2
        color: "#b4b4b4"
    }

    TabButton {
        id: tabButton1
        index: 0
        anchors.top: parent.top
        anchors.topMargin: 11
        anchors.left: leftSpacer.right
        width: 58
        height: 26
        buttonText: "Gauges"
        active: true
        onActivated: updateTabs(index)
    }

    Rectangle {
        id: spacer1
        anchors.top: parent.top
        anchors.topMargin: 26
        anchors.left: tabButton1.right
        anchors.leftMargin: 2
        width: 7
        height: 2
        color: "#b4b4b4"
    }

    TabButton {
        id: tabButton2
        index: 1
        anchors.top: parent.top
        anchors.topMargin: 11
        anchors.left: spacer1.right
        height: 26
        buttonText: "Buttons"
        active: false
        onActivated: updateTabs(index)
    }

    Rectangle {
        id: spacer2
        anchors.top: parent.top
        anchors.topMargin: 26
        anchors.left: tabButton2.right
        anchors.leftMargin: 2
        width: 8
        height: 2
        color: "#b4b4b4"
    }

    TabButton {
        id: tabButton3
        index: 2
        anchors.top: parent.top
        anchors.topMargin: 11
        anchors.left: spacer2.right
        width: 71
        height: 26
        buttonText: "Advanced"
        active: false
        onActivated: updateTabs(index)
    }

    Rectangle {
        id: spacer3
        anchors.top: parent.top
        anchors.topMargin: 26
        anchors.left: tabButton3.right
        anchors.leftMargin: 2
        width: 8
        height: 2
        color: "#b4b4b4"
    }

    TabButton {
        id: tabButton4
        index: 3
        anchors.top: parent.top
        anchors.topMargin: 11
        anchors.left: spacer3.right
        width: 54
        height: 26
        buttonText: "Sliders"
        active: false
        onActivated: updateTabs(index)
    }

    Rectangle {
        id: spacer4
        anchors.top: parent.top
        anchors.topMargin: 26
        anchors.left: tabButton4.right
        anchors.leftMargin: 2
        width: 7
        height: 2
        color: "#b4b4b4"
    }

    TabButton {
        id: tabButton5
        index: 4
        anchors.top: parent.top
        anchors.topMargin: 11
        anchors.left: spacer4.right
        width: 56
        height: 26
        buttonText: "Graphs"
        active: false
        onActivated: updateTabs(index)
    }

    Rectangle {
        id: spacer5
        anchors.top: parent.top
        anchors.topMargin: 26
        anchors.left: tabButton5.right
        anchors.leftMargin: 2
        width: 7
        height: 2
        color: "#b4b4b4"
    }

    TabButton {
        id: tabButton6
        index: 5
        anchors.top: parent.top
        anchors.topMargin: 11
        anchors.left: spacer5.right
        width: 56
        height: 26
        buttonText: "Tabs"
        active: false
        onActivated: updateTabs(index)
    }

    Rectangle {
        id: rightSpacer
        anchors.top: parent.top
        anchors.topMargin: 26
        anchors.left: tabButton6.right
        width: 234
        height: 2
        color: "#b4b4b4"
    }
}
