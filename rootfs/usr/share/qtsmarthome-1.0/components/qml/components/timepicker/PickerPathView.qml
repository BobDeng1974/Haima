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
* This instance of a PathView Item is showing elements
* along a simple PathLine. This PathLine defines a vertical path that begins in the top
* and ends bottomline.
* As default elements (model) there are numbers from 0 to 55 with steps of 5.
* it is possible to set a different model
*/
PathView {
    id: path_view1

    property variant currentValue

    /**
    * This function is called to set the current value
    */
    function setCurrentValue(value) {
        for( var i = 0; i < model.length; i++ ) {
            if( model[i] == value ) {
                i -= 2
                if( i < 0 )
                    i += model.length
                currentIndex = i
                break
            }
        }
    }

    width: 58
    height: Math.floor( model.length / 3 ) * 200
    y: -40
    model: ["00", "05", "10", "15", "20", "25", "30", "35", "40", "45", "50", "55"]
    // The delegate shows the information as a Text Item
    delegate: Text {
        font.family: "Arial"
        font.bold: true
        font.pixelSize: 50
        smooth: true
        color: "black"
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: modelData
        Component.onCompleted: {
            font.pointSize = font.pointSize * 0.8
        }
    }

    // The vertical path is defined here.
    path: Path {
        id: path
        startX: path_view1.width / 2
        PathLine {
            x: path.startX
            y: path_view1.height
        }
    }

    // if the currentIndex changes we set the current value appropriate
    onCurrentIndexChanged: {
        var index = ( currentIndex + 2 ) % model.length
        currentValue = model[ index]
    }

    /**
    * This mouse area is in the upper area.
    * When pressed inside the currentIndex is inremented
    */
    MouseArea {
        id: topArea
        x: 0
        y: 40
        width: 58
        height: 59
        onClicked: path_view1.incrementCurrentIndex()
    }

    /**
    * This mouse area is in the lower area.
    * When pressed inside the currentIndex is decremented
    */
    MouseArea {
        id: bottomArea
        x: 0
        y: 180
        width: 58
        height: 59
        onClicked: path_view1.decrementCurrentIndex()
    }
}
