import QtQuick 2.0

Rectangle {
    id: root

    property int updateInterval: 333
    property alias enabled: drawTimer.running
    property int graphType: 0

    property string backgroundColor1: "transparent"
    property string backgroundColor2: "transparent"
    property string backgroundColor3: "transparent"

    //property int tickCount: 0

    anchors.fill: parent

    gradient: Gradient {
        GradientStop { position: 0.0; color: backgroundColor1 }
        GradientStop { position: 0.33; color: backgroundColor2 }
        GradientStop { position: 1.0; color: backgroundColor3 }
    }

    function resetZoom() {
        canvas.widthDivisor = 10;
        canvas.heightDivisor = 30;
    }

    function zoomYIn() {
        if (canvas.heightDivisor > 18)
            canvas.heightDivisor -= 3;
    }

    function zoomXIn() {
        if (canvas.widthDivisor > 4)
            canvas.widthDivisor -= 3;
    }

    function zoomYOut() {
        if (canvas.heightDivisor < 42)
            canvas.heightDivisor += 3;
    }

    function zoomXOut() {
        if (canvas.widthDivisor < 22)
            canvas.widthDivisor += 3;
    }

    onGraphTypeChanged: {
        canvas.tickState = 0
        canvas.tickReminder = 0
        canvas.currentX = 0
        canvas.lastY = canvas.height / 2
        canvas.requestPaint();
    }

    Canvas {
        id: canvas

        /* !!!IMPORTANT!!!: the heightDivisor mustn't be smaller than the multipliers for yUnit in the switch/-case part,
                otherwise the function drawGraphLine would try to draw a line beyond the boundaries of the canvas.
                Also both divisors mustn't be 0 to avoid division by 0. */
        property int heightDivisor: 30
        property int widthDivisor: 10

        property real xUnit: (canvas.width / widthDivisor) / 9
        property real yUnit: halfHeight / heightDivisor
        property real halfHeight: canvas.height/2

        property int tickState: 0
        property int tickReminder: 0
        property int currentX: 0
        property double lastY: canvas.height / 2

        property var ctx: undefined

        anchors.fill: parent
        smooth: true
        renderTarget: Canvas.FramebufferObject
        renderStrategy: Canvas.Threaded

        onAvailableChanged: {
            if( available && canvas.ctx == undefined ){
                //console.log("onAvailableChanged: initializing drawing context")
                canvas.ctx = getContext("2d");
            }
        }

        function drawGraphLine(ctx) {

            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.lineWidth = 2;
            ctx.strokeStyle = '#ffffff';
            ctx.miterLimit = canvas.width / xUnit + 1;

            ctx.beginPath();
            ctx.moveTo(0, lastY);
            tickReminder = tickState;
            for (currentX = xUnit; currentX < canvas.width; currentX += xUnit ) {
                switch (tickState) {
                case 0:
                    lastY = halfHeight - yUnit;
                    tickState = 1;
                    break;
                case 1:
                    lastY = halfHeight + yUnit;
                    tickState = 2;
                    break;
                case 2:
                    lastY = halfHeight - yUnit;
                    tickState = 3;
                    break;
                case 3:
                    lastY = halfHeight + yUnit;
                    tickState = 4;
                    break;
                case 4:
                    lastY = halfHeight - yUnit * 2;
                    tickState = 5;
                    break;
                case 5:
                    lastY = halfHeight;
                    tickState = 6;
                    break;
                case 6:
                    lastY = halfHeight - yUnit * 15;
                    ctx.lineTo(currentX, lastY);
                    lastY = halfHeight + yUnit * 10;
                    tickState = 7;
                    break;
                case 7:
                    lastY = halfHeight - yUnit;
                    tickState = 8;
                    break;
                case 8:
                    lastY = halfHeight;
                    tickState = 0;
                    break;
                }
                ctx.lineTo(currentX, lastY);
            }

            ctx.stroke();
            ctx.closePath();

            if( tickReminder < 8 ) {
                tickState = tickReminder + 1;
            } else {
                tickState = 0;
            }

        }

        function drawSinusWave(ctx) {

            var scale = 60;

            ctx.clearRect(0, 0, canvas.width, canvas.height);
            ctx.lineWidth = 2;
            ctx.strokeStyle = '#ffffff';

            ctx.beginPath();
            ctx.moveTo(0, lastY);

            tickReminder = tickState;

            for (currentX = xUnit; currentX < canvas.width; currentX += xUnit ) {

                var rad = tickState + currentX * (Math.PI/(canvas.width/xUnit));
                lastY = halfHeight - (scale * Math.sin(rad));

                canvas.ctx.lineTo(currentX, lastY);
            }

            ctx.stroke();

            if( tickReminder < 31 ) {
                tickState = tickReminder + 1;
            } else {
                tickState = 0;
            }

        }

        onPaint: {
            if( canvas.ctx == undefined ) {
                console.log("Note: Graph Canvas::onPaint found ctx undefined, initializing drawing context now.")
                canvas.ctx = getContext("2d")
            }
            switch (graphType) {
            case 0:
                drawGraphLine(canvas.ctx);
                break;
            case 1:
                drawSinusWave(canvas.ctx);
                break;
            }
        }

        Timer {
            id: drawTimer
            interval: root.updateInterval; running: false; repeat: true
            onTriggered: {
                canvas.requestPaint();
            }
        }
    }

}
