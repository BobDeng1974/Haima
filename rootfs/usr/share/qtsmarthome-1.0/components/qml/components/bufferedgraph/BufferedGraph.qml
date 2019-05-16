import QtQuick 2.0

Rectangle {
    id: root

    property int updateInterval: 50
    property alias enabled: drawTimer.running
    property int graphType: 0

    property alias useBuffering: canvas.useBuffering

    property alias drawPartialLineSegments: canvas.drawPartialLineSegments

    property string backgroundColor1: "transparent"
    property string backgroundColor2: "transparent"
    property string backgroundColor3: "transparent"

    anchors.fill: parent

    gradient: Gradient {
        GradientStop { position: 0.0;  color: backgroundColor1 }
        GradientStop { position: 0.33; color: backgroundColor2 }
        GradientStop { position: 1.0;  color: backgroundColor3 }
    }

    onBackgroundColor1Changed: { canvas.linePoints = [] }
    onBackgroundColor2Changed: { canvas.linePoints = [] }
    onBackgroundColor3Changed: { canvas.linePoints = [] }

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
        canvas.linePoints = []
        canvas.tickState = 0
        canvas.tickReminder = 0
        canvas.currentX = 0
        canvas.lastY = canvas.height / 2
        canvas.requestPaint();
    }


    Canvas {
        id: canvas

        property bool useBuffering: true

        property bool drawPartialLineSegments: true

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

        property variant gradient: root.gradient

        anchors.fill: parent
        smooth: true

        renderTarget:   Canvas.Image
        renderStrategy: Canvas.Immediate

        onWidthDivisorChanged:  { linePoints = [] }
        onHeightDivisorChanged: { linePoints = [] }
        onWidthChanged:         { linePoints = [] }
        onHeightChanged:        { linePoints = [] }

        onAvailableChanged: {
            if( available && canvas.ctx == undefined ){
                //console.log("onAvailableChanged: initializing drawing context")
                canvas.ctx = getContext("2d");
            }
        }


        // a global array of points for drawGraphLine()
        property var linePoints: []
        property int linePointsCount: 0
        property var bufferedImage

        function storeCoordinate(xVal, yVal) {
            var point = {x: xVal, y: yVal};
            linePoints.push( point );
            console.log("point:" + point.x + "  " + point.y)
        }

        function initLinePoints() {
            console.log("------- initializing the line points -------");
            lastY = canvas.height / 2;
            storeCoordinate(0, lastY);
            var xVal = xUnit;
            for (tickState = 0; tickState < 9; tickState++ ) {
                //console.log("tickState: " + tickState);
                var yVal;
                switch (tickState) {
                case 0:
                    yVal = halfHeight - yUnit;
                    break;
                case 1:
                    yVal = halfHeight + yUnit;
                    break;
                case 2:
                    yVal = halfHeight - yUnit;
                    break;
                case 3:
                    yVal = halfHeight + yUnit;
                    break;
                case 4:
                    yVal = halfHeight - yUnit * 2;
                    break;
                case 5:
                    yVal = halfHeight;
                    break;
                case 6:
                    yVal = halfHeight - yUnit * 15;
                    storeCoordinate(xVal, yVal);
                    yVal = halfHeight + yUnit * 10;
                    break;
                case 7:
                    yVal = halfHeight - yUnit;
                    break;
                case 8:
                    yVal = halfHeight;
                    break;
                }
                storeCoordinate(xVal, yVal);
                xVal += xUnit;
            }
            linePointsCount = linePoints.length;
        }

        function drawGraphLine(ctx) {

            if( ! linePoints.length ) {
                // initialize the points array
                initLinePoints();

                // initialize the image buffer
                if( useBuffering ) {
                    var maxX = 0;
                    for (var i = 1; i < linePointsCount; i++ )
                        maxX = Math.max(maxX, linePoints[ i ].x);

                    // first fill the background with the current gradient
                    var lineGrad = ctx.createLinearGradient(0,0, 0,canvas.height);
                    if (gradient != undefined ) {

                        for (var i= 0 ; i< gradient.stops.length; ++i) {
                            console.log("building background gradient: " +
                                        gradient.stops[i].color + " at " + gradient.stops[i].position);
                            lineGrad.addColorStop(gradient.stops[i].position, gradient.stops[i].color);
                        }

                        ctx.fillStyle = lineGrad;
                    }
                    ctx.fillRect(0,0, maxX,canvas.height);

                    // now draw the line segment
                    ctx.lineWidth = 2;
                    ctx.strokeStyle = '#ffffff';
                    ctx.miterLimit = canvas.width / xUnit + 1;

                    ctx.beginPath();
                    ctx.moveTo( linePoints[0].x, linePoints[0].y );

                    for (var i = 1; i < linePointsCount; i++ ) {
                        var point = linePoints[ i ];
                        ctx.lineTo( point.x, point.y );
                    }
                    ctx.stroke();
                    ctx.closePath();

                    // store the line segment with its background pixels in our buffer
                    bufferedImage = ctx.getImageData(0,0, maxX, canvas.height);
                }
            }

            if( useBuffering ) {
                if( bufferedImage != undefined ) {
                    // Draw the buffered image repeatedly:
                    // Because the image includes the background pixels,
                    // no clearRect() is needed!

                    var waveLength = bufferedImage.width;
                    var lastX = -currentX;
                    while ( lastX < canvas.width ) {
                        ctx.putImageData(bufferedImage, lastX,0);
                        lastX += waveLength;
                    }

                    if( currentX+1 < waveLength )
                        currentX += 1;
                    else
                        currentX = 0;
                }

            } else {
                // Clear all and draw the line at the full width of the canvas:

                ctx.clearRect(0, 0, canvas.width, canvas.height);
                ctx.lineWidth = 2;
                ctx.strokeStyle = '#ffffff';
                ctx.miterLimit = canvas.width / xUnit + 1;

                ctx.beginPath();
                ctx.moveTo( linePoints[0] );

                var waveLength = 0;
                var xVal = 0 - currentX;
                var lastX = 0;

                tickState = 0;
                while ( lastX < canvas.width ) {
                    var point = linePoints[ tickState ];
                    lastX = xVal + point.x;
                    if( drawPartialLineSegments || lastX < canvas.width ) {
                        ctx.lineTo( lastX, point.y );
                    }

                    if( tickState < linePointsCount-1 ) {
                        tickState++;
                    } else {
                        tickState = 0;
                        xVal += xUnit * (linePointsCount-2);
                        waveLength = point.x;
                    }
                }

                if( currentX+1 < waveLength )
                    currentX += 1;
                else
                    currentX = 0;

                ctx.stroke();
                ctx.closePath();
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
