#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Updated global variables to store joystick positions with unique names to avoid conflicts
volatile float joystickX1 = 0, joystickY1 = 0, joystickX2 = 0, joystickY2 = 0;

const char index_html[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>
    <meta name='viewport' content='width=device-width,         initial-scale=1.0,         user-scalable=no' />
    <meta charset="utf-8" />


    <title>Joystick test</title>


    <style>
        #container {
            width: 100%;
            height: 49vh;
            background-color: #333;
            display: flex;
            align-items: center;
            justify-content: center;
            overflow: hidden;
            border-radius: 7px;
            touch-action: none;
        }

        #item {
            width: 400px;
            height: 400px;
            background-color: rgb(245, 230, 99);
            border: 10px solid rgba(136, 136, 136, .5);
            border-radius: 50%;
            touch-action: none;
            user-select: none;
        }

        #item:active {
            background-color: rgba(168, 218, 220, 1.00);
        }

        #item:hover {
            cursor: pointer;
            border-width: 20px;
        }

        #area {
            position: fixed;
            right: 0;
            top: 0;
        }
    </style>


    <script type="text/javascript">
        var JoyStick = function (t, e) { var i = void 0 === (e = e || {}).title ? "joystick" : e.title, n = void 0 === e.width ? 0 : e.width, o = void 0 === e.height ? 0 : e.height, h = void 0 === e.internalFillColor ? "#00AA00" : e.internalFillColor, r = void 0 === e.internalLineWidth ? 2 : e.internalLineWidth, d = void 0 === e.internalStrokeColor ? "#003300" : e.internalStrokeColor, a = void 0 === e.externalLineWidth ? 2 : e.externalLineWidth, l = void 0 === e.externalStrokeColor ? "#008000" : e.externalStrokeColor, c = document.getElementById(t), u = document.createElement("canvas"); u.id = i, 0 == n && (n = c.clientWidth), 0 == o && (o = c.clientHeight), u.width = n, u.height = o, c.appendChild(u); var s = u.getContext("2d"), f = 0, v = 2 * Math.PI, g = (u.width - 110) / 2, w = g + 5, C = g + 30, m = u.width / 2, p = u.height / 2, L = u.width / 10, E = -1 * L, S = u.height / 10, k = -1 * S, W = m, G = p; function x() { s.beginPath(), s.arc(m, p, C, 0, v, !1), s.lineWidth = a, s.strokeStyle = l, s.stroke() } function y() { s.beginPath(), W < g && (W = w), W + g > u.width && (W = u.width - w), G < g && (G = w), G + g > u.height && (G = u.height - w), s.arc(W, G, g, 0, v, !1); var t = s.createRadialGradient(m, p, 5, m, p, 200); t.addColorStop(0, h), t.addColorStop(1, d), s.fillStyle = t, s.fill(), s.lineWidth = r, s.strokeStyle = d, s.stroke() } "ontouchstart" in document.documentElement ? (u.addEventListener("touchstart", function (t) { f = 1 }, !1), u.addEventListener("touchmove", function (t) { t.preventDefault(), 1 == f && (W = t.touches[0].pageX, G = t.touches[0].pageY, W -= u.offsetLeft, G -= u.offsetTop, s.clearRect(0, 0, u.width, u.height), x(), y()) }, !1), u.addEventListener("touchend", function (t) { f = 0, W = m, G = p, s.clearRect(0, 0, u.width, u.height), x(), y() }, !1)) : (u.addEventListener("mousedown", function (t) { f = 1 }, !1), u.addEventListener("mousemove", function (t) { 1 == f && (W = t.pageX, G = t.pageY, W -= u.offsetLeft, G -= u.offsetTop, s.clearRect(0, 0, u.width, u.height), x(), y()) }, !1), u.addEventListener("mouseup", function (t) { f = 0, W = m, G = p, s.clearRect(0, 0, u.width, u.height), x(), y() }, !1)), x(), y(), this.GetWidth = function () { return u.width }, this.GetHeight = function () { return u.height }, this.GetPosX = function () { return W }, this.GetPosY = function () { return G }, this.GetX = function () { return ((W - m) / w * 100).toFixed() }, this.GetY = function () { return ((G - p) / w * 100 * -1).toFixed() }, this.GetDir = function () { var t = "", e = W - m, i = G - p; return i >= k && i <= S && (t = "C"), i < k && (t = "N"), i > S && (t = "S"), e < E && ("C" == t ? t = "W" : t += "W"), e > L && ("C" == t ? t = "E" : t += "E"), t } };
    </script>
</head>


<body>

    <div id='outerContainer'>
        <div style="height: 5vh;"></div>
        
	<div id="joyDiv" style="width:300px;height:300px;margin:auto;bottom:400px;left:250px;"></div>

        <div style="height: 30vh;"></div>
        <div id="joy2Div" style="width:300px;height:300px;margin:auto;bottom:10px;left:250px;"></div>

    </div>

    <script>


        const view = document.getElementById('stream');
        const WS_URL = "ws://" + window.location.host + ":82";
        const ws = new WebSocket(WS_URL);

        ws.onmessage = message => {
            if (message.data instanceof Blob) {
                var urlObject = URL.createObjectURL(message.data);
                view.src = urlObject;
            }
        };

      document.addEventListener("DOMContentLoaded", function() {
      var ws = new WebSocket('ws://' + window.location.hostname + '/ws');
      ws.onopen = function() {
        console.log('WebSocket is connected.');
      };
      ws.onerror = function(error) {
        console.log('WebSocket Error ', error);
      };
      ws.onmessage = function(e) {
        console.log('Server: ', e.data);
      };

        function sendJoystickData(joystickX1, joystickY1, joystickX2, joystickY2) {
        if (ws.readyState === WebSocket.OPEN) {
          var data = {x1: joystickX1, y1: joystickY1, x2: joystickX2, y2: joystickY2};
          ws.send(JSON.stringify(data));
          }
        }
	
	setInterval(function() {
        sendJoystickData(joy.GetX(), joy.GetY(), joy2.GetX(),joy2.GetY());
      }, 1000);

    </script>
</body>

<script type="text/javascript">
    // Create JoyStick object into the DIV 'joyDiv'
    var joy = new JoyStick('joyDiv');
    var inputPosX = document.getElementById("posizioneX");
    var inputPosY = document.getElementById("posizioneY");
    var direzione = document.getElementById("direzione");
    var x = document.getElementById("X");
    var y = document.getElementById("Y");


	// var joy2Param = { "title": "joystick2" };
	// var Joy2 = new JoyStick('joy2Div', joy2Param);
	var joy2 = new JoyStick('joy2Div');
	var joy2IinputPosX = document.getElementById("joy2PosizioneX");
	var joy2InputPosY = document.getElementById("joy2PosizioneY");
	var joy2Direzione = document.getElementById("joy2Direzione");
	var joy2X = document.getElementById("joy2X");
	var joy2Y = document.getElementById("joy2Y");



    setInterval(function () { send(joy.GetX()); }, 30);
    setInterval(function () { send(joy.GetY()); }, 30);
    setInterval(function () { send(joy2.GetX()); }, 30);
    setInterval(function () { send(joy2.GetY()); }, 30);
</script>

</html>


)rawliteral";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void setup() {
  Serial.begin(115200);

  // Set up the ESP32 as an Access Point
  WiFi.softAP(ssid, password);
  Serial.println("AP Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // WebSocket event handler
  ws.onEvent([](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type,
                void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_DATA) {
      // Assuming data is a JSON string in the format {"x1":val,"y1":val,"x2":val,"y2":val}
      data[len] = 0; // Ensure null-termination for string processing

      // Parse the joystick data
      float x1, y1, x2, y2;
      sscanf((const char*)data, "{\"x1\":%f,\"y1\":%f,\"x2\":%f,\"y2\":%f}", &x1, &y1, &x2, &y2);

      // Update the global variables
      joystickX1 = x1;
      joystickY1 = y1;
      joystickX2 = x2;
      joystickY2 = y2;

      Serial.printf("Received joystick data: x1=%f, y1=%f, x2=%f, y2=%f\n", joystickX1, joystickY1, joystickX2, joystickY2);
    }
  });
  server.addHandler(&ws);

  // Serve the HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
  });

  server.begin();
}

void loop() {
  // Continuously print the joystick values
  Serial.printf("Joystick 1: X=%f, Y=%f | Joystick 2: X=%f, Y=%f\n", joystickX1, joystickY1, joystickX2, joystickY2);
  delay(1000); // Adjust based on how frequently you want to print
}

/*
var lastText, lastSend, sendTimeout;
        // limit sending to one message every 30 ms
        // https://github.com/neonious/lowjs_esp32_examples/blob/master/neonious_one/cellphone_controlled_rc_car/www/index.html
        function send(txt) {
            var now = new Date().getTime();
            if (lastSend === undefined || now - lastSend >= 30) {
                try {
                    ws.send(txt);
                    lastSend = new Date().getTime();
                    return;
                } catch (e) {
                    console.log(e);
                }
            }
            lastText = txt;
            if (!sendTimeout) {
                var ms = lastSend !== undefined ? 30 - (now - lastSend) : 30;
                if (ms < 0)
                    ms = 0;
                sendTimeout = setTimeout(() => {
                    sendTimeout = null;
                    send(lastText);
                }, ms);
            }
        }
*/