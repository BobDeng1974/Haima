import QtQuick 2.0
import "ledscreencomponent"

Rectangle {
    id: root

    width: 854
    height: 480
    color: "#000000"

    Image {
        anchors.fill: parent
        source: "images/background2.png"
    }

    Item {
        id: sourceArea
        width: 106
        height: 60
        Image {
            id: logoImage
            property int imageIndex: 1
            function changeImage() {
                imageIndex = imageIndex < 3 ? ++imageIndex : 1;
            }

            anchors.centerIn: parent
            source: "images/logo" + imageIndex + ".png"

            ParallelAnimation {
                running: true
                loops: Animation.Infinite
                SequentialAnimation {
                    NumberAnimation { target: logoImage; property: "rotation"; to: 50; duration: 7000; easing.type: Easing.OutElastic }
                    NumberAnimation { target: logoImage; property: "rotation"; to: -20; duration: 2000; easing.type: Easing.InOutSine  }
                }
                SequentialAnimation {
                    NumberAnimation { target: logoImage; property: "scale"; to: 0.5; duration: 6000; easing.type: Easing.InOutSine }
                    NumberAnimation { target: logoImage; property: "scale"; to: 1.0; duration: 3000; easing.type: Easing.OutBounce }
                }
            }
            Behavior on source {
                SequentialAnimation {
                    NumberAnimation { target: logoImage; property: "anchors.verticalCenterOffset"; to: 100; duration: 400; easing.type: Easing.InSine }
                    PropertyAction { target: logoImage; property: "source" }
                    NumberAnimation { target: logoImage; property: "anchors.verticalCenterOffset"; to: 0; duration: 800; easing.type: Easing.OutBack }
                }
            }
        }
        Text {
            anchors.centerIn: parent
            font.pixelSize: 12
            text: "CLICK\nHERE!"
            color: "#ffff00"
            style: Text.Outline
            styleColor: "#000000"
            font.bold: true
            SequentialAnimation on scale {
                loops: Animation.Infinite
                NumberAnimation { to: 2.0; duration: 1000; easing.type: Easing.InOutSine  }
                NumberAnimation { to: 1.0; duration: 1000; easing.type: Easing.InOutSine  }
            }
        }
    }

    LedScreen {
        anchors.fill: parent
        sourceItem: sourceArea
        ledSize: root.height / sourceArea.height
        threshold: 0.1
        useSourceColors: true
        MouseArea {
            anchors.fill: parent
            onClicked: logoImage.changeImage();
        }
    }
}
