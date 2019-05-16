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
* The TabWidget is a special version of the ordinary tab widget in the Qt library or the one from
* the QML examples by Nokia. This widhet features up to four tab bars, one at the top, the bottom, and
* on the left and right side of the widhet. All tab bars access a shared pool of "pages". The
* configuration of this TabWidget is a little bit more difficult than what you might expect. This is
* due to the fact that the widget is organized in separete components (the widget stack and the four
* tab bars). So one has to inform the individual tab bar which pages of the stack it should manage.
* How this can be done is illustrated in this little example:
*
* TabWidget
* {
*    id: tabWidget
*     width: 640
*     height: 480
*
*     state: "init"
*
*     pages: [
*         Rectangle { color: "yellow"; width: 50; height: 300},
*         Rectangle { color: "black"; width: 100; height: 200 }
*     ]
*     states: [
*         State {
*             name: "init"
*             PropertyChanges {
*                 target: tabWidget;
*                 topBar    { tabs: ["foo", "bar"]; indices: [0, 1] }
*                 leftBar   { tabs: ["Left1", "Left2"]; indices: [1, 0] }
*                 bottomBar { tabs: ["Unten1", "Unten2", "Unten3"]; indices: [0, 0, 1] }
*                 rightBar  { tabs: ["Right #1", "Right2"]; indices: [1, 0] }
*             }
*         }
*     ]
* }
*
* In this example a TabWidget with all four tab bars is instantiated. There are only two pages (in
* this case two Rectangles). That's why there are only two indices, namely 0 and 1. In the states
* section a state named "init" is defined, featuring a PropertyChanges element where the actual
* configuration of the TabWidget takes place. Each tab bar has the properties
*
* tabs:       A list of strings for the tab captions
* indices:    A list of integers specifying which page index to be used for the tabs
*
* The contents of the internal widget stack (accessible through the "pages" property) can be of
* arbitrary type.
*
* The building blocks of the TabWidgets are these custom QML components:
* HTabBar:      A horizontal tab bar. Can be either the top or bottom tab bar
* VTabBar:      A vertical tab bar. Can be either the left or right tab bar
* WidgetStack:  The container for the pages of the TabWidget
*
* properties:
* topBar:       Top tab bar
* leftBar:      Left tab bar
* rightBar:     Right tab bar
* bottomBar:    Bottom tab bar
* pages:        The list of contained elements
*/

Rectangle {
    id: tabWidget

    property alias topBar: topBar
    property alias leftBar: leftBar
    property alias rightBar: rightBar
    property alias bottomBar: bottomBar

    property int currentTab: 0
    property alias pages: stack.children

    property int alignLeft: 0
    property int alignRight: 1
    property int alignTop: 2
    property int alignBottom: 3

    function showPage(index)
    {
        currentTab = index
        hidePages()
    }

    function hidePages()
    {
        for (var i = 0; i < pages.length; ++i) {
            pages[i].visible = (i == currentTab)
        }
    }

    /*
     * This function sets the tab text
     */
    function setTabText(index, label) {
        if(index < stack.widgets.length) {
            stack.widgets[index].tabText = label
        }
    }

    /*
     * This function sets the tab icon
     */
    function setTabIcon(index, label) {
        if(index < stack.widgets.length) {
            stack.widgets[index].tabIconPath = label
        }
    }

    /*
     * This function gets the tab text
     */
    function tabText(index, label) {
        if(index < stack.widgets.length) {
            return stack.widgets[index].tabText
        }
        return ""
    }

    /*
     * This function gets the tab icon
     */
    function tabIcon(index, label) {
        if(index < stack.widgets.length) {
            return stack.widgets[index].tabIconPath;
        }
        return ""
    }

    /*
     * This function sets the content of the stack to the corresponding
     * tabBar. E.g indices 1, 2, 3 are going to rightBar and 4, 5 going to topBar
     */
    function addContentToTabs(content, bar, alignment) {
        var indices = [];
        var tabText = [];
        var iconPaths = [];
        for (var i = 0; i < content.length; ++i) {
            if(content[i].tabAlignment == alignment) {
                tabText[i] = content[i].tabText
                iconPaths[i] = content[i].tabIconPath
                indices[i] = content[i].tabIndex
            }
        }
        if(indices.length > 0) {
            bar.tabs = tabText
            bar.icons = iconPaths
            bar.indices = indices
        }
    }

    /*
     * container for the tab widget pages.
     */
    Item {
        id: stack
        property alias widgets: stack.children
        anchors {
            top: topBar.bottom
            left: leftBar.right
            right: parent.right
            bottom: bottomBar.top
        }
        // children changed were prefered but didnt work
        // the height changes when a child is added
        onHeightChanged: {
            addContentToTabs(widgets, leftBar, alignLeft)
            addContentToTabs(widgets, rightBar, alignRight)
            addContentToTabs(widgets, topBar, alignTop)
            addContentToTabs(widgets, bottomBar, alignBottom)
        }
    }

    /*
    Here the building blocks of the TabWidget are created.
    Intentionally no QML positioners are used. Instead all positioning is done via property
    binding.
    */
    HTabBar {
        id: topBar
        x: leftBar.visible ? leftBar.width : 0
        height: 0
        width: tabWidget.width - ( leftBar.visible ? leftBar.width : 0 )
        - ( rightBar.visible ? rightBar.width : 0 )
        visible: false
        onCurrentChanged: showPage( index )
    }

    VTabBar {
        id: leftBar
        y : topBar.visible ? topBar.height : 0
        width: 0
        height: tabWidget.height - ( topBar.visible ? topBar.height : 0 )
        - ( bottomBar.visible ? bottomBar.height : 0 )
        visible: false
        onCurrentChanged: showPage( index )
    }

    VTabBar {
        id: rightBar
        indices: [0, 1]
        showOnLeft: false
        x : tabWidget.width - width
        y : topBar.visible ? topBar.height : 0
        width: 0
        height: tabWidget.height - ( topBar.visible ? topBar.height : 0 )
        - ( bottomBar.visible ? bottomBar.height : 0 )
        visible: false
        onCurrentChanged: showPage( index )
    }

    HTabBar {
        id: bottomBar
        showOnTop: false
        x: leftBar.visible ? leftBar.width : 0
        y: tabWidget.height - height
        width: tabWidget.width - ( leftBar.visible ? leftBar.width : 0 )
        - ( rightBar.visible ? rightBar.width : 0 )
        height: 0
        visible: false
        onCurrentChanged: showPage( index )
    }

    Component.onCompleted: showPage( 0 )
}
