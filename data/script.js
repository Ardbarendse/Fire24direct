// Get current sensor readings when the page loads  
window.addEventListener('load', getReadings);

// Button function
function OnToggle(event_id) {
  websocket.send(JSON.stringify({'action':event_id}));
  console.log(JSON.stringify({'action':event_id}));
}
// Create Temperature Gauge
var gaugeWatertank = new RadialGauge({
  renderTo: 'gauge-watertank',
  title: "Watertank",
  fontTitleSize: 80,
  fontTitleWeight: "bold",
  width: 200,
  height: 200,
  units: "%",
  fontUnitsSize: 80,
  minValue: 0,
  maxValue: 100,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 3,
  valueDec: 0,
  majorTicks: [
      "0",
      "20",
      "40",
      "60",
      "80",
      "100"
  ],
  fontValueSize: 40,
  fontNumbersSize: 30,
  minorTicks: 4,
  strokeTicks: true,
  colorPlate: "white",
  highlights: [
      {
          "from": 0,
          "to": 20,
          "color": "red"
      },
      {
          "from": 40,
          "to": 100,
          "color": "blue"
      },
      {   "from": 20,
          "to": 40,
          "color": "yellow"
      }
  ],
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  colorNeedle: "grey",
  colorNeedleEnd: "grey",
  needleWidth: 4,
  needleCircleSize: 6,
  colorNeedleCircleOuter: "grey",
  needleCircleOuter: true,
  needleCircleInner: false,
  animation: false
}).draw();
  
// Create Humidity Gauge
var gaugePressure = new RadialGauge({
  renderTo: 'gauge-pressure',
  title: "Pressure",
  fontTitleSize: 80,
  fontTitleWeight: "bold",
  width: 200,
  height: 200,
  units: "BAR",
  fontUnitsSize: 80,
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
      "24"
  ],
  fontValueSize: 40,
  fontNumbersSize: 40,
  minorTicks: 4,
  strokeTicks: true,
  colorPlate: "white",
  highlights: [
      {
          "from": 16,
          "to": 24,
          "color": "red"
      },
      {
          "from": 0,
          "to": 12,
          "color": "green"
      },
      {   "from": 12,
          "to": 16,
          "color": "yellow"
      }
  ],
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  colorNeedle: "grey",
  colorNeedleEnd: "grey",
  needleWidth: 4,
  needleCircleSize: 6,
  colorNeedleCircleOuter: "grey",
  needleCircleOuter: true,
  needleCircleInner: false,
  animation: false
}).draw();

var gaugeFueltank = new RadialGauge({
    renderTo: 'gauge-fueltank',
    title: "Fueltank",
    fontTitleSize: 80,
    fontTitleWeight: "bold",
  width: 200,
  height: 200,
    units: "%",
    fontUnitsSize: 80,
    minValue: 0,
    maxValue: 100,
    colorValueBoxRect: "#049faa",
    colorValueBoxRectEnd: "#049faa",
    colorValueBoxBackground: "#f1fbfc",
    valueInt: 3,
    valueDec: 0,
    majorTicks: [
        "0",
        "20",
        "40",
        "60",
        "80",
        "100"
    ],
    fontValueSize: 40,
    fontNumbersSize: 30,
    minorTicks: 4,
    strokeTicks: true,
    colorPlate: "white",
    highlights: [
        {
            "from": 0,
            "to": 10,
            "color": "red"
        },
        {
            "from": 10,
            "to": 20,
            "color": "yellow"
        },
        {   "from": 20,
            "to": 100,
            "color": "grey"
        }
    ],
    borderShadowWidth: 0,
    borders: false,
    needleType: "arrow",
    colorNeedle: "grey",
    colorNeedleEnd: "grey",
    needleWidth: 4,
    needleCircleSize: 6,
    colorNeedleCircleOuter: "grey",
    needleCircleOuter: true,
    needleCircleInner: false,
    animation: false
  }).draw();
    
  var gaugeEngtemp = new RadialGauge({
    renderTo: 'gauge-enginetemp',
    title: "Engine temp",
    fontTitleSize: 80,
    fontTitleWeight: "bold",
  width: 200,
  height: 200,
    units: "C",
    fontUnitsSize: 80,
    minValue: -40,
    maxValue: 240,
    colorValueBoxRect: "#049faa",
    colorValueBoxRectEnd: "#049faa",
    colorValueBoxBackground: "#f1fbfc",
    valueInt: 3,
    valueDec: 0,
    majorTicks: [
        "-40",
        "30",
        "100",
        "170",
        "240",
    ],
    fontValueSize: 40,
    fontNumbersSize: 30,
    minorTicks: 4,
    strokeTicks: true,
    colorPlate: "white",
    highlights: [
        {
            "from": -40,
            "to": 90,
            "color": "blue"
        },
        {
            "from": 90,
            "to": 105,
            "color": "green"
        },
        {   "from": 105,
            "to": 240,
            "color": "red"
        }
    ],
    borderShadowWidth: 0,
    borders: false,
    needleType: "arrow",
    colorNeedle: "grey",
    colorNeedleEnd: "grey",
    needleWidth: 4,
    needleCircleSize: 6,
    colorNeedleCircleOuter: "grey",
    needleCircleOuter: true,
    needleCircleInner: false,
    animation: false
  }).draw();
    
  // Create PTOSPpeed Gauge
var gaugePTOspeed = new RadialGauge({
  renderTo: 'gauge-ptospeed',
  title: "PUMP RPM",
  fontTitleSize: 80,
  fontTitleWeight: "bold",
  width: 200,
  height: 200,
  units: "/MIN",
  fontUnitsSize: 80,
  minValue: 0,
  maxValue: 5000,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueInt: 4,
  valueDec: 0,
  majorTicks: [
      "0",
      "1000",
      "2000",
      "3000",
      "4000",
      "5000"
  ],
  fontValueSize: 40,
  fontNumbersSize: 30,
  minorTicks: 4,
  strokeTicks: true,
  colorPlate: "white",
  highlights: [
      {
          "from": 4000,
          "to": 5000,
          "color": "red"
      },
      {
          "from": 3000,
          "to": 4000,
          "color": "yellow"
      },
      {   "from": 0,
          "to": 3000,
          "color": "green"
      }
  ],
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  colorNeedle: "grey",
  colorNeedleEnd: "grey",
  needleWidth: 4,
  needleCircleSize: 6,
  colorNeedleCircleOuter: "grey",
  needleCircleOuter: true,
  needleCircleInner: false,
  animation: false
}).draw();

var gaugeFoamtank = new RadialGauge({
    renderTo: 'gauge-foamtank',
    title: "Foamtank",
    fontTitleSize: 80,
    fontTitleWeight: "bold",
    width: 200,
    height: 200,
    units: "%",
    fontUnitsSize: 80,
    minValue: 0,
    maxValue: 100,
    colorValueBoxRect: "#049faa",
    colorValueBoxRectEnd: "#049faa",
    colorValueBoxBackground: "#f1fbfc",
    valueInt: 3,
    valueDec: 0,
    majorTicks: [
        "0",
        "20",
        "40",
        "60",
        "80",
        "100"
    ],
    fontValueSize: 40,
    fontNumbersSize: 30,
    minorTicks: 4,
    strokeTicks: true,
    colorPlate: "white",
    highlights: [
        {
            "from": 0,
            "to": 20,
            "color": "red"
        },
        {
            "from": 40,
            "to": 100,
            "color": "blue"
        },
        {   "from": 20,
            "to": 40,
            "color": "yellow"
        }
    ],
    borderShadowWidth: 0,
    borders: false,
    needleType: "arrow",
    colorNeedle: "grey",
    colorNeedleEnd: "grey",
    needleWidth: 4,
    needleCircleSize: 6,
    colorNeedleCircleOuter: "grey",
    needleCircleOuter: true,
    needleCircleInner: false,
    animation: false
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
    if ("watertank" in data) {
        gaugeWatertank.value = data.watertank;
    }
    if ("pressure" in data) {
        gaugePressure.value = data.pressure;
    }
    if ("fueltank" in data) {
        gaugeFueltank.value = data.fueltank;
    }
    if ("engcooltemp" in data) {
        gaugeEngtemp.value = data.engcooltemp;
    }
    if ("ptospeed" in data) {
        gaugePTOspeed.value = data.ptospeed;
    }
    if ("foamtank" in data) {
        gaugeFoamtank.value = data.foamtank;
    }
    if ("warning" in data) {
        document.getElementById('led').className = data.warning;
    }
    if ("warningtext" in data) {
        document.getElementById('warningtext').innerHTML = data.warningtext;
    }
} 

function getReadings(){
/*   gaugeHum.value = 7;
  gaugeTemp.value = 13; */
}
