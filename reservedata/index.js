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
    if ("teller_1" in data) {
    document.getElementById('COUNTER_1_VALUE').innerHTML = data.teller_1;}
    if ("teller_2" in data) {
      gaugeTemp.value = data.teller_2;
    document.getElementById('COUNTER_2_VALUE').innerHTML = data.teller_2;}
 } 
 
 // ----------------------------------------------------------------------------
 // Button handling
 // ----------------------------------------------------------------------------
 /*
 function initButton() {
     document.getElementById('toggle_1').addEventListener('click', onToggle, 'toggle_1');
     document.getElementById('toggle_2').addEventListener('click', onToggle, 'toggle_2');
    }
 */

 function OnToggle(event_id) {
    websocket.send(JSON.stringify({'action':event_id}));
    console.log(JSON.stringify({'action':event_id}));
 }

 if (!!window.EventSource) {
    var source = new EventSource('/events');
    
    source.addEventListener('open', function(e) {
     console.log("Events Connected");
    }, false);
    source.addEventListener('error', function(e) {
     if (e.target.readyState != EventSource.OPEN) {
       console.log("Events Disconnected");
     }
    }, false);
    
    source.addEventListener('message', function(e) {
     console.log("message", e.data);
    }, false);
    
    source.addEventListener('temperature', function(e) {
     console.log("temperature", e.data);
     document.getElementById("temp").innerHTML = e.data;
    }, false);
    
    source.addEventListener('humidity', function(e) {
     console.log("humidity", e.data);
     document.getElementById("hum").innerHTML = e.data;
    }, false);
    
    source.addEventListener('pressure', function(e) {
     console.log("pressure", e.data);
     document.getElementById("pres").innerHTML = e.data;
    }, false);
   }

    let opts = {
    angle: 0, // The span of the gauge arc
    lineWidth: 0.44, // The line thickness
    radiusScale: 0.83, // Relative radius
    pointer: {
      length: 0.6, // // Relative to gauge radius
      strokeWidth: 0.035, // The thickness
      color: '#000000' // Fill color
    },
    limitMax: false, // If false, max value increases automatically if value > maxValue
    limitMin: false, // If true, the min value of the gauge will be fixed
    colorStart: '#6FADCF', // Colors
    colorStop: '#8FC0DA', // just experiment with them
    strokeColor: '#E0E0E0', // to see which ones work best for you
    generateGradient: true,
    highDpiSupport: true // High resolution support
  }
/*   
  let target = document.querySelector('#gauge') // your canvas element
  
  let gaugeChart = new Gauge(target).setOptions(opts) // create sexy gauge!
  gaugeChart.maxValue = 3000 // set max gauge value
  gaugeChart.setMinValue(0) // Prefer setter over gauge.minValue = 0
  gaugeChart.animationSpeed = 32 // set animation speed (32 is default value)
  gaugeChart.set(COUNTER_2_VALUE) // set actual value 
 */
  // Create Temperature Gauge
var gaugeTemp = new LinearGauge({
  renderTo: 'gauge-temperature',
  width: 120,
  height: 400,
  units: "Temperature C",
  minValue: 0,
  startAngle: 90,
  ticksAngle: 180,
  maxValue: 40,
  colorValueBoxRect: "#049faa",
  colorValueBoxRectEnd: "#049faa",
  colorValueBoxBackground: "#f1fbfc",
  valueDec: 2,
  valueInt: 2,
  majorTicks: [
      "0",
      "5",
      "10",
      "15",
      "20",
      "25",
      "30",
      "35",
      "40"
  ],
  minorTicks: 4,
  strokeTicks: true,
  highlights: [
      {
          "from": 30,
          "to": 40,
          "color": "rgba(200, 50, 50, .75)"
      }
  ],
  colorPlate: "#fff",
  colorBarProgress: "#CC2936",
  colorBarProgressEnd: "#049faa",
  borderShadowWidth: 0,
  borders: false,
  needleType: "arrow",
  needleWidth: 2,
  needleCircleSize: 7,
  needleCircleOuter: true,
  needleCircleInner: false,
  animationDuration: 1500,
  animationRule: "linear",
  barWidth: 10,
}).draw();