/**
 * ----------------------------------------------------------------------------
 * ESP32 Remote Control with WebSocket
 * ----------------------------------------------------------------------------
 * © 2020 Stéphane Calderoni
 * ----------------------------------------------------------------------------
 */

 var gateway = `ws://${window.location.hostname}/ws`;
 var websocket;
 
 // ----------------------------------------------------------------------------
 // Initialization
 // ----------------------------------------------------------------------------
 
 window.addEventListener('load', onLoad);
 
 function onLoad(event) {
     initWebSocket();
     initButton();
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

   // var data = JSON.parse(event.data); 
    document.getElementById('COUNTER_VALUE').innerHTML = event.data;
    console.log(COUNTER_VALUE);
 } 
 
 // ----------------------------------------------------------------------------
 // Button handling
 // ----------------------------------------------------------------------------
 
 function initButton() {
     document.getElementById('toggle').addEventListener('click', onToggle);
 }
 
 function onToggle(event) {
    websocket.send(JSON.stringify({'action':'toggle'}));
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