#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <CAN.h>
#include <esp_task_wdt.h>

// WiFi credentials
const char *ssid = "Fire24test";
const char *password =  "test1234";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;
IPAddress apIP(192,168,4,1);

int fuel = 1;
int pressure = 2;
int i = 0;

uint8_t PressureByte = 0;
uint8_t FuelByte = 19;
uint8_t PressureOut = 3;
uint8_t FuelOut = 5;

TaskHandle_t Receive, Send;

AsyncWebServer server(http_port);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient* client;
AsyncEventSource events("/events");

// SPIFFS initialization
// ----------------------------------------------------------------------------

void initSPIFFS() {
  while (!SPIFFS.begin());
  Serial.println("SPIFF Init done.");
}

// Connecting to the WiFi network
// ----------------------------------------------------------------------------

void initWiFi() {
  WiFi.softAP(ssid, password);
  delay(2000);
  WiFi.softAPConfig(apIP,apIP,IPAddress(255,255,255,0));
  Serial.println();
  Serial.println("AP running");
  Serial.print("My IP address: ");
  Serial.println(WiFi.softAPIP());
}

// Web server initialization
// ----------------------------------------------------------------------------

String processor(const String &var) {
    if(var == "fuel_up"){
        return String(FuelOut);
    }
    else if(var == "pressure_up"){
        return String(PressureOut);
    }
    return String();
}

void onRootRequest(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/index.html", "text/html", false, processor);
}

void initWebServer() {
    server.on("/", onRootRequest);
    server.serveStatic("/", SPIFFS, "/");
    server.begin();
    Serial.println("Wid");
}
/*
void notifyClients(const char *id, String cont) {
    // const uint8_t size = JSON_OBJECT_SIZE(1);
    DynamicJsonDocument json(2048);
    json[id] = cont;
    char buffer[2048];
    serializeJson(json, buffer);
    Serial.println(buffer);
    ws.textAll(buffer);
}*/

void onReceive(void * pvParameters) {
  esp_task_wdt_delete(NULL);
  // esp_task_wdt_status();
  for(;;){
    int packetSize = CAN.parsePacket();
    if (packetSize) {
    Serial.print("Received ");

    DynamicJsonDocument jsonc2w(2048);
    word ID = CAN.packetId();

    switch (ID){
      case 0x12:
        while (CAN.available()){
          for (int i = 0; i<CAN.packetDlc(); i++){
            if (i==1)jsonc2w["pressure"] = (String)CAN.read();
            else CAN.read();
          }
        }
      break;

      case 0xabcdef:
        while (CAN.available()){
          for (int i = 0; i<CAN.packetDlc(); i++){
            if (i==2)jsonc2w["fuel"] = (String)CAN.read();
            else CAN.read();
          }
        }
      break; 
    }

    char buffer[2048];
    serializeJson(jsonc2w, buffer);
    Serial.println(buffer);
    ws.textAll(buffer);
 
    // Serial.println();
    }
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

       // const uint8_t size = JSON_OBJECT_SIZE(1);
        DynamicJsonDocument jsonw2c(2048);
        DeserializationError err = deserializeJson(jsonw2c, data);
        if (err) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(err.c_str());
            return;
        }

/*   */
    }
}

void onEvent(AsyncWebSocket       *server,
             AsyncWebSocketClient *client,
             AwsEventType          type,
             void                 *arg,
             uint8_t              *data,
             size_t                len) {

    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            break;
         case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break; 
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}

void initWebSocket() {
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void initCAN() {
      // start the CAN bus at 250 kbps
  if (!CAN.begin(250E3)) {
    Serial.println("Starting CAN failed!");
    while (1);
  }
}

/* void SendPacket(void * pvParameters){
  for(;;) {
         // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  // Serial.printf("Sending packet from... %d\r\n", xPortGetCoreID());
  CAN.beginPacket(0x12);
  CAN.write(1);
  CAN.write((byte)(PressureByte++%24));
  CAN.write(3);
  CAN.write(4);
  CAN.write(5);
  CAN.write(6);
  CAN.write(7);
  CAN.write(8);
  CAN.endPacket();

  delay(1000);

  // send extended packet: id is 29 bits, packet can contain up to 8 bytes of data
   Serial.print("Sending extended packet ... ");

  CAN.beginExtendedPacket(0xabcdef);
  CAN.write('6');
  CAN.write('7');
  CAN.write((byte)(FuelByte++)%100);
  CAN.write('9');
  CAN.write('0');  
  CAN.write('1');
  CAN.write('2');
  CAN.write('3');
  CAN.endPacket();
  
  delay(1000);
  }
} */

void setup() {
    Serial.begin(115200); 
    while (!Serial);
    
   /*  xTaskCreatePinnedToCore(
      SendPacket,
      "Sends CAN messages",
      10000,
      NULL,
      0,
      &Send,
      0);
 */
    xTaskCreatePinnedToCore(
    onReceive,
    "Receives CAN messages",
    10000,
    NULL,
    0,
    &Receive,
    0); 
    
    initSPIFFS();
    initWiFi();
    initWebSocket();
    initWebServer();
    initCAN();
    // CAN.loopback();
   // CAN.onReceive(onReceive);
}

void loop() {
  ws.cleanupClients();

  if (++i==9) {
    Serial.printf("[RAM: %d]\r\n", esp_get_free_heap_size());
    i=0;
  }
        // send packet: id is 11 bits, packet can contain up to 8 bytes of data
  // Serial.printf("Sending packet from... %d\r\n", xPortGetCoreID());
/*   CAN.beginPacket(0x12);
  CAN.write(1);
  CAN.write((byte)(PressureByte++%24));
  CAN.write(3);
  CAN.write(4);
  CAN.write(5);
  CAN.write(6);
  CAN.write(7);
  CAN.write(8);
  CAN.endPacket();

  delay(1000);

  // send extended packet: id is 29 bits, packet can contain up to 8 bytes of data
  //  Serial.print("Sending extended packet ... ");

  CAN.beginExtendedPacket(0xabcdef);
  CAN.write('6');
  CAN.write('7');
  CAN.write((byte)(FuelByte++)%100);
  CAN.write('9');
  CAN.write('0');  
  CAN.write('1');
  CAN.write('2');
  CAN.write('3');
  CAN.endPacket();
   */
  delay(1000);
}

