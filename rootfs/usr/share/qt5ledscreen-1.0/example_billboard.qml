import QtQuick 2.0
import "ledscreencomponent"

Rectangle {
    id: root

    property int scrollSpeed: 500

    width: 854
    height: 480
    color: "#000000"

    Image {
        id: backgroundImage
        anchors.centerIn: parent
        source: "images/billboard.png"
    }

    Item {
        id: sourceArea
        width: 41
        height: 15
        Text {
            id: textItem
            // This is used to force x to animate int instead of float
            property int textXPos
            x: textXPos
            anchors.verticalCenter: parent.verticalCenter
            // Select suitable font family and size
            font.family: "Fixedsys"
            font.pixelSize: 14
            font.bold: true
            color: "#ffffff"
            smooth: false
            text: "ATTENTION! Qt5 is coming...  "
            NumberAnimation on textXPos {
                loops: Animation.Infinite
                from: sourceArea.width; to: -textItem.paintedWidth; duration: textItem.text.length*scrollSpeed
            }
        }
    }

    LedScreen {
        id: ledScreen
        sourceItem: sourceArea
        width: 656
        height: 240
        anchors.centerIn: backgroundImage
        anchors.horizontalCenterOffset: 40
        anchors.verticalCenterOffset: -10
        transform: [
            Rotation { origin.x: backgroundImage.width/2; origin.y: backgroundImage.height/2; axis { x: 0; y: 0; z: 1 } angle: -4 },
            Rotation { origin.x: backgroundImage.width/2; origin.y: backgroundImage.height*2; axis { x: 0; y: 1; z: 0 } angle: 19 },
            Rotation { origin.x: backgroundImage.width/2; origin.y: backgroundImage.height/2; axis { x: 1; y: 0; z: 0 } angle: 20 }
        ]
        ledSize: 16
        threshold: 0.48
        Image {
            anchors.fill: parent
            source: "images/reflection.png"
        }
    }
}
