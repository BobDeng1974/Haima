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
This file defines a gauge. The overlay of the gauge can be switched off. The
origin of the needle can be set.

property string backgroundImage:    The background image of the gauge.
property string needleImage:        The image of the needle.
property string needleShadowImage:  The image of the needles' shadow.
property string overlayImage:       The image of the overlay.
property bool showOverlay:          Flag to set the visibility of the overlay.
property int value:                 The current value of the gauge.
property real originFactorX:        Value to offset the needles' offset in x-direction.
property real originFactorY:        Value to offset the needles' offset in y-direction.
property real needleScale:          Value of the scale of the needle.
*/
Item {
    property string backgroundImage
    property string needleImage
    property string needleShadowImage
    property string overlayImage
    property bool showOverlay: true
    property int value: 0
    property real originFactorX: 0.0
    property real originFactorY: 0.0
    property real needleScale: 1.0

    Image {
        id: background
        source: backgroundImage
        smooth: true
        width: parent.width
        height: parent.height
        sourceSize.width: width
        sourceSize.height: height

        Image {
            id: needle
            source: needleImage
            width: background.width
            height: background.height
            sourceSize.width: background.width
            sourceSize.height: background.height
            smooth: true
            scale: needleScale
            z: 2
            transform: Rotation {
                origin.x: background.width * originFactorX
                origin.y: background.height * originFactorY
                angle: value
            }
        }

        Image {
            id: needleShadow
            source: needleShadowImage
            width: background.width
            height: background.height
            sourceSize.width: background.width
            sourceSize.height: background.height
            smooth: true
            scale: needleScale
            z: 1
            transform: Rotation {
                origin.x: background.width * originFactorX
                origin.y: background.height * originFactorY
                angle: value
            }
        }

        Image {
            id: overlay
            source: overlayImage
            width: background.width
            height: background.height
            sourceSize.width: background.width
            sourceSize.height: background.height
            smooth: true
            z: 3
            visible: showOverlay
        }
    }
}
