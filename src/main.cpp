#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <CAN.h>
#include <esp_task_wdt.h>
#include <CANController.h>

// WiFi credentials
const char *ssid = "Fire24test";
const char *password =  "test1234";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;
IPAddress apIP(192,168,4,1);

int watertank = 0;
int watertankread = 0;
int foamtank = 0;
int foamtankread = 0;
float pressure = 0;
float pressureread = 0;
int i = 0;
int FuelTank = 0;
int FuelTankread = 0;
int EngCoolTemp = 0;
int EngCoolTempread = 0;
int PTOSpeed = 0;
int PTOSpeedread = 0;

int UpdateInterval = 400; //dashboard update ms
int Lastupdate = 0;

uint8_t PressureByte = 0;
uint8_t WatertankByte = 19;
uint8_t PressureOut = 3;
uint8_t WatertankOut = 5;

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
    if(var == "watertank_up"){
        return String(WatertankOut);
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
    // Serial.println("Wid");
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
  // esp_task_wdt_delete(NULL);
  CAN.filterExtended((long)201589256);
  
  for(;;){
    int packetSize = CAN.parsePacket();
    if (packetSize) {

      DynamicJsonDocument jsonc2w(2048);
      word ID = CAN.packetId();
      // Serial.print("a");

      
        if (ID>= (int)0xc040200 && ID < (int)0xc040300){        //pump pressure
          // Serial.print("b");
          while (CAN.available()){
            pressureread = 0;
            for (int i = 0; i<CAN.packetDlc(); i++){
              switch (i){
                case 0: pressureread += (0.01 * (float)CAN.read());
                break;
                case 1: pressureread += (2.56 * (float)CAN.read());
                break;
                default: CAN.read();
                break;
              }
              // Serial.println(pressureread);
              if (pressureread < 65) {pressure = pressureread;};
            }
          }
        }

        else if (ID==(int)0xc041d0a){            // pump pressure
          // Serial.print("c");
          while (CAN.available()){
            pressureread = 0;
            for (int i = 0; i<CAN.packetDlc(); i++){
              switch (i){
                case 2: pressureread += (0.01 * (float)CAN.read());
                break;
                case 3: pressureread += (2.56 * (float)CAN.read());
                break;
                default: CAN.read();
                break;
              }
            if (pressureread < 650) {pressure = pressureread;};
            }
          }
        }
        
/*         else if (ID >= (int)0xc070800 && ID < (int)0xc080900){                 // water tank level
          // Serial.print("d");
          while (CAN.available()){
            watertankread = 0;
            for (int i = 0; i<CAN.packetDlc(); i++){
              switch (i){
                case 2: watertankread += CAN.read();
                break;
                case 3: watertankread += (256 * CAN.read());
                break;
                default: CAN.read();
                break;
                if (watertankread < 65000) watertank = watertankread;
              }
            }
          }
        }   */
                
        else if (ID >= (int)0xc070000 && ID < (int)0xc070100){         // Water tank level percentage
          // Serial.print("e");
          while (CAN.available()){
            watertankread = 0;
            foamtankread = 0;
            for (int i = 0; i<CAN.packetDlc(); i++){
              switch (i){
                case 0: watertankread += CAN.read();
                break;
                case 1: watertankread += (256 * CAN.read());
                break;
                case 2: foamtankread += CAN.read();
                break;
                case 3: foamtankread += (256 * CAN.read());
                break;
                default: CAN.read();
                break;
              }
            if (watertankread < 1100) {watertank = watertankread / 10;};
            if (foamtankread < 1100) {foamtank = foamtankread / 10;};
            }
          }
        }

        else if (ID == (int)0xc010501){                               // Pump speed
          // Serial.print("e");
          while (CAN.available()){
            PTOSpeedread = 0;
            for (int i = 0; i<CAN.packetDlc(); i++){
              switch (i){
                case 4: PTOSpeedread += CAN.read();
                break;
                case 5: PTOSpeedread += (256 * CAN.read());
                break;
                default: CAN.read();
                break;
              }
              Serial.print(ID);
              Serial.print(" ");
              Serial.println(PTOSpeedread);
            if (PTOSpeedread < 10000) {PTOSpeed = PTOSpeedread;};
            }
          }
        }

        else if (ID >= (int)0xc010200 && ID < (int)0xc010300){          // Fuel tank level
          // Serial.print("f");
          while (CAN.available()){
            FuelTankread = 0;
            for (int i = 0; i<CAN.packetDlc(); i++){
              switch (i){
                case 4: FuelTankread += (1 * CAN.read());
                break;
                case 5: FuelTankread += (256 * CAN.read());
                break;
                default: CAN.read();
                break;
              }
              if (FuelTankread < 65000) {FuelTank = FuelTankread / 10;};
            }
            // FuelTank = FuelTank / 10;
          }
        }

        else if (ID>=(int)0xc010600 && ID < (int)0xc010700){    // engine temp
          // Serial.print("g");
          while (CAN.available()){
            EngCoolTempread = -40;
            for (int i = 0; i<CAN.packetDlc(); i++){
              switch (i){
                case 0: EngCoolTempread += CAN.read();
                break;
                default: CAN.read();
                break;
              }
              if (EngCoolTempread < 200) EngCoolTemp = EngCoolTempread;
            }
          }
        } 
      

    if ((millis() - Lastupdate > UpdateInterval)){
      char buffer[2048];
      jsonc2w["ptospeed"] = PTOSpeed;
      jsonc2w["pressure"] = pressure;
      jsonc2w["watertank"] = watertank;
      jsonc2w["fueltank"] = FuelTank;
      jsonc2w["engcooltemp"] = EngCoolTemp;

      serializeJson(jsonc2w, buffer);
      Serial.println();
      Serial.println(buffer);
      ws.textAll(buffer);
      Lastupdate = millis();
    }
    }
  }
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    DynamicJsonDocument jsonw2c(2048);
    DeserializationError err = deserializeJson(jsonw2c, data);
    if (err) {
      Serial.print(F("deserializeJson() failed with code "));
      Serial.println(err.c_str());
      return;
    }
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
    // Serial.printf("[RAM: %d]\r\n", esp_get_free_heap_size());
    i=0;
  }

  delay(1000);
}

