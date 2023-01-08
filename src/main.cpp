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

// int watertank = 0;
// int foamtank = 0;
// float pressure = 0;
// int FuelTank = 0;
// int EngCoolTemp = 0;
// int PTOSpeed = 0;

int i = 0;

byte ReadBytes [8];
uint16_t ReadValue = 0;

int UpdateInterval = 250; //dashboard update ms
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

void onReceive(void * pvParameters) {
  // esp_task_wdt_delete(NULL);
  CAN.filterExtended((long)201589256);
  
  for(;;)
  {
    int packetSize = CAN.parsePacket();
    if (packetSize) 
    {

      DynamicJsonDocument jsonc2w(2048);
      // char buffer[2048];
      word ID = CAN.packetId();
      Serial.println(ID >> 4);
      while (CAN.available()){
        for (int i = 0; i<CAN.packetDlc(); i++){
          ReadBytes[i] = CAN.read();
        }
      }
      
      if ((ID >> 4) == (int)0xc04020)                     // pump pressure
      {
        ReadValue = ReadBytes[0] | (ReadBytes[1] << 8);
        if (ReadValue < 65000){jsonc2w["pressure"] = ReadValue / 100;};
      }

      else if (ID == (int)0xc041d0a)                      // pump pressure
      {
        ReadValue = ReadBytes[2] | (ReadBytes[3] << 8);
        if (ReadValue < 65000){jsonc2w["pressure"] = ReadValue / 100;};
      }
       
      else if ((ID >> 4) == (int)0xc07000)                // Water tank level percentage
      {
        ReadValue = ReadBytes[0] | (ReadBytes[1] << 8);
        if (ReadValue < 65000){jsonc2w["watertank"] = ReadValue / 10;}; 
        ReadValue = ReadBytes[2] | (ReadBytes[3] << 8);
        if (ReadValue < 65000){jsonc2w["foamtank"] = ReadValue / 10;};
      }  

      else if ((ID >> 4) == (int)0xc01050)                 // Pump speed
      {
        ReadValue = ReadBytes[4] | (ReadBytes[5] << 8);
        if (ReadValue < 65000){jsonc2w["ptospeed"] = ReadValue;};
      }

      else if ((ID >> 4) == (int)0xc01020)                // Fuel tank level
      {
        ReadValue = ReadBytes[4] | (ReadBytes[5] << 8);
        if (ReadValue < 65000){jsonc2w["fueltank"] = ReadValue / 10;};
      } 

      else if ((ID >> 4) ==(int)0xc01060)                 // engine temp
      {
        ReadValue = ReadBytes[0];
        if (ReadValue < 256){jsonc2w["engcooltemp"] = ReadValue -40;};
      }

      if ((millis() - Lastupdate > UpdateInterval)){
        char buffer[2048];
        // jsonc2w["ptospeed"] = PTOSpeed;
        // jsonc2w["pressure"] = pressure;
        // jsonc2w["watertank"] = watertank;
        // jsonc2w["fueltank"] = FuelTank;
        // jsonc2w["engcooltemp"] = EngCoolTemp;

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

