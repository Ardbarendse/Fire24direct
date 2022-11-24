// Get current sensor readings when the page loads  
window.addEventListener('load', getReadings);

// Button function
/* function OnToggle(event_id) {
  websocket.send(JSON.stringify({'action':event_id}));
  console.log(JSON.stringify({'action':event_id}));
} */
// Create fuel Gauge
var gaugeFuel = new LinearGauge({
  renderTo: 'gauge-fuel',
  width: 80,
  height: 240,
  units: "Fuel %",
  minValue: 0,
  startAngle: 90,
  ticksAngle: 180,
  maxValue: 100,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 2,
  valueInt: 2,
  majorTicks: [
      "0",
      "20",
      "40",
      "60",
      "80",
      "100",
  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
          "from": 0,
          "to": 15,
          "color": "rgba(200, 50, 50, 1)"
      }
  ],
  colorPlate: "#fff",
  colorBarProgress: "#CC2936",
  colorBarProgressEnd: "#049faa",
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  needleWidth: 8,
  needleCircleSize: 7,
  needleCircleOuter: true,
  needleCircleInner: false,
  animation: false,
 // animationDuration: 1500,
 // animationRule: "linear",
  barWidth: 10,
}).draw();
  
// Create Pressure Gauge
var gaugePressure = new RadialGauge({
  renderTo: 'gauge-pressure',
  width: 200,
  height: 200,
  units: "Pressure (%)",
  minValue: 0,
  maxValue: 24,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 2,
  majorTicks: [
      "0",
      "4",
      "8",
      "12",
      "16",
      "20",
      "24"
  ],
  minorTicks: 1,
  strokeTicks: true,
  highlights: [
      {
          "from": 16,
          "to": 24,
          "color": "#03C0C1"
      }
  ],
  colorPlate: "#fff",
  borderShadowWidth: 0,
  borders: false,
  needleType: "line",
  colorNeedle: "#007F80",
  colorNeedleEnd: "#007F80",
  needleWidth: 2,
  needleCircleSize: 3,
  colorNeedleCircleOuter: "#007F80",
  needleCircleOuter: true,
  needleCircleInner: false,
  animation: false,
//  animationDuration: 1500,
//  animationRule: "linear"
}).draw();

 
 
var gateway = `ws://${window.location.hostname}/ws`;
var websocket;
 
// ----------------------------------------------------------------------------
// Initialization
// ----------------------------------------------------------------------------

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
   // initButton();
}

// ----------------------------------------------------------------------------
// WebSocket handling
// ----------------------------------------------------------------------------

function initWebSocket() {
    console.log('Trying to open a WebSocket connection...');
    websocket = new WebSocket(gateway);
    websocket.onopen    = onOpen;
    websocket.onclose   = onClose;
    websocket.onmessage = onMessage;
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function onMessage(event) {
   console.log(event.data);
   var data = JSON.parse(event.data); 
   if ("fuel" in data) {
    gaugeFuel.value = data.fuel;
    }
    if ("pressure" in data) {
     gaugePressure.value = data.pressure;
    }
 } 

function getReadings(){
  websocket.send(JSON.stringify({'action':'send_current'}));
  console.log(JSON.stringify({'action':'send_current'}));
}
