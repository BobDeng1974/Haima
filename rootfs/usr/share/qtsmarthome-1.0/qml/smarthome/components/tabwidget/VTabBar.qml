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

/**
 * The VTabBar displays a vertical tab bar.
 *
 * Properties:
 * tabs:       A list of strings for the tab captions
 * indices:    A list of integers specifying which page index to be used for the tabs
 * icons:      A list of urls specifying the images to be used as the tab icons
 *
 * Signals:
 * clicked(int tab):    Emitted when a tab has been clicked
 */
Rectangle {
    id: tabBar

    property variant tabs: []
    property variant icons: []
    property variant indices
    property bool showOnLeft: true
    property string tabImagePressed
    property string tabImageNormal

    property string tabIconDirectory
    property string tabIconFileType

    property int currentIndex: 0

    signal currentChanged(int index)

    /**
     * Function to set the current tab. As a side effect the tab is drawn in "selected" state and
     * the "clicked()" signal is emitted.
     */
    function setCurrentIndex(cur) {
        currentIndex = cur
        if(indices)
            currentChanged( indices[ currentIndex ])
    }

    visible: false
    color: "transparent"

    /**
     * Initially the tab bar is invisible. This makes sure it's made visible on-demand.
     */
    onIndicesChanged: { visible = true; width = 130; }

    /**
     * For convenience, the Column positioner is used to create a vertical list of tabs.
     */
    Column {
        anchors.verticalCenter: parent.verticalCenter
        id: header
        /**
         * A Repeater is used to dynamically create the desired number of tabs.
         */
        Repeater {
            model: tabBar.tabs.length
            delegate: Rectangle {
                id: rect
                color: "transparent"
                height: Math.min(100, tabBar.height / tabBar.tabs.length); width: tabBar.width

                BorderImage {
                    id: image
                    anchors { fill: parent; margins: 0; rightMargin: 1 }
                    source: tabBar.currentIndex == index ? "../pics/tabs/tabs_pressed.svg" : "../pics/tabs/tabs_normal.svg"
                    opacity: tabBar.currentIndex == index ? 1 : 0.5
                    transform: Scale {
                        origin.x: parent.width / 2 - image.anchors.leftMargin
                        xScale: tabBar.showOnLeft ? -1 : 1
                        yScale: 1
                    }
                }

                Image {
                    id: imageIcon
                    anchors.centerIn: parent
                    source: tabBar.icons.length > index ? tabBar.icons[index] : ""
                }

                Text {
                    smooth: true
                    color: "white"
                    horizontalAlignment: Qt.AlignHCenter; verticalAlignment: Qt.AlignVCenter
                    anchors.top: imageIcon.bottom
                    x: parent.width/2 - width/2
                    text: tabBar.tabs[index]
                    font.family: "Arial"
                    font.pointSize: 12
                    font.bold: tabBar.currentIndex == index
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: {
                        setCurrentIndex(index)
                    }
                }
            }
        }
    }
}
