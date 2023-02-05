#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <CAN.h>
#include <esp_task_wdt.h>
#include <CANController.h>

bool testversion = true;
int SendDelay = 50;

// WiFi credentials
const char *ssid = "Fire24test";
const char *password =  "test1234";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;
IPAddress apIP(192,168,4,1);

int i = 0;

byte ReadBytes [8];               // Storage variables for read messages
uint16_t ReadValue = 0;
float ReadFloat = 0;


int UpdateInterval = 500;         //dashboard update ms
int Lastupdate = 0;

byte Faultbyte = 0xdf;            // variables for sending test warnings
int Faultinterval = 4000;
int Lastfaultswitch;
int FaultCodeTest;
uint8_t PressureByte = 0;         //variables for sending test messages
uint8_t WatertankByte = 19;
uint8_t PressureOut = 3;
uint8_t WatertankOut = 5;
uint8_t PressureByte1 = 0;
uint8_t WaterByte1 = 0;
uint8_t PressureByte2 = 0;
uint8_t WaterByte2 = 0;
uint8_t FuelByte1 = 0;
uint8_t FuelByte2 = 0;
uint8_t EngCoolTempByte = 0;
float sensor = 0;
float sensoravg = 0;

TaskHandle_t Receive, Send;

AsyncWebServer server(http_port);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient* client;
AsyncEventSource events("/events");

// SPIFFS initialization
// ----------------------------------------------------------------------------
void initSPIFFS() {
  // SPIFFS.format();
  while (!SPIFFS.begin())delay(1000);
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

// CAN Message receive function
// ------------------------------------------------------------------------------
void onReceive(void * pvParameters) {
  esp_task_wdt_delete(NULL);
  char buffer[2048];
  DynamicJsonDocument jsonc2w(2048);

  for(;;)
  {
    int packetSize = CAN.parsePacket();
    if (packetSize) 
    {
      word ID = CAN.packetId();
      while (CAN.available()){
        for (int i = 0; i<CAN.packetDlc(); i++){
          ReadBytes[i] = CAN.read();
        }
      }
      
      if ((ID >> 4) == (int)0xc04020)                     // pump pressure
      {
        ReadFloat = ReadBytes[0] | (ReadBytes[1] << 8);
        if (ReadFloat < 65000){jsonc2w["pressure"] = ReadFloat / 100;};
      }

      else if (ID == (int)0xc041d0a)                      // pump pressure
      {
        ReadFloat = ReadBytes[2] | (ReadBytes[3] << 8);
        if (ReadFloat < 65000){jsonc2w["pressure"] = ReadFloat / 100;};
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

      else if ((ID >> 4) == (int)0xc01060)                 // engine temp
      {
        ReadValue = ReadBytes[0];
        if (ReadValue < 256){jsonc2w["engcooltemp"] = ReadValue -40;};
      }

      else if ((ID >> 4) == (int)0xcf0010)
      {
        FaultCodeTest = (ReadBytes[3] >> 4) & 0x03;
        if (FaultCodeTest == 1)
        {
          jsonc2w["warning"] = "on";
          jsonc2w["warningtext"] = "warningtest";
        }
        else 
        {
          jsonc2w["warning"] = "off";
          jsonc2w["warningtext"] = " ";
        }
      }

      if (millis() - Lastupdate > UpdateInterval)
      {
        serializeJson(jsonc2w, buffer);
        // Serial.println(buffer);
        ws.textAll(buffer);
        Lastupdate = millis();
        jsonc2w.clear();
       }
    }
  }
}

// CAN
void SendData(){
  for (int i = 0; i < 10; i++)
  {
    sensor++;
    sensoravg = int(1000 * (1 + sin(sensor/60)));
    if (sensor == 720) sensor = 0;
    WaterByte1 = int(sensoravg / 2) % 256;
    WaterByte2 = (sensoravg / 2) / 256;
    FuelByte1 = ((2000 - (int)sensoravg) / 2) % 256;  
    FuelByte2 = ((2000 - sensoravg) / 2) / 256;  
    EngCoolTempByte = ((2000 - sensoravg) / 10);
    // Serial.println(EngCoolTempByte);

    CAN.beginExtendedPacket(0xc040208);           // pump pressure
    CAN.write((byte)(int(sensoravg) & 0xff));
    CAN.write((byte)(int(sensoravg) >> 8));
    CAN.write(3);
    CAN.write(4);
    CAN.write(5);
    CAN.write(6);
    CAN.write(7);
    CAN.write(8);
    CAN.endPacket();

    delay(SendDelay);

    CAN.beginExtendedPacket(0x0c070001);          // water tank level + foam tank level
    CAN.write((byte)(int(sensoravg / 2) & 0xff));
    CAN.write((byte)(int(sensoravg / 2) >> 8));
    CAN.write(0xff);  
    CAN.write(0xff);
    CAN.write(0xff);
    CAN.write(0xff);
    CAN.write('2');
    CAN.write('3');
    CAN.endPacket();

    delay(SendDelay);

    CAN.beginExtendedPacket(0x0c010201);          // fuel tank level
    CAN.write('6');
    CAN.write('7');
    CAN.write((byte)WaterByte1);
    CAN.write((byte)WaterByte2);
    CAN.write((byte)FuelByte1);  
    CAN.write((byte)FuelByte2);
    CAN.write('a');
    CAN.write('b');
    CAN.endPacket();

    delay(SendDelay);

    CAN.beginExtendedPacket(0x0c010601);            // engine temperature
    CAN.write((byte)EngCoolTempByte);
    CAN.write('7');
    CAN.write((byte)WaterByte1);
    CAN.write((byte)WaterByte2);
    CAN.write((byte)FuelByte1);  
    CAN.write((byte)FuelByte2);
    CAN.write('a');
    CAN.write('b');
    CAN.endPacket();

    delay(SendDelay);
    
    CAN.beginExtendedPacket(0x0c070008);          // water tank level + foam tank level
    CAN.write(0xff);
    CAN.write(0xff);
    CAN.write((byte)(int(sensoravg / 2) & 0xff));
    CAN.write((byte)(int(sensoravg / 2) >> 8));
    CAN.write(0xff);
    CAN.write(0xff);
    CAN.write('2');
    CAN.write('3');
    CAN.endPacket();

    delay(SendDelay);

    CAN.beginExtendedPacket(0x0c010509);          // pump speed
    CAN.write(0xff);
    CAN.write(0xff);
    CAN.write(0xff);
    CAN.write(0xff);
    CAN.write((byte)(int(sensoravg) & 0xff));
    CAN.write((byte)(int(sensoravg) >> 8));
    CAN.write('2');
    CAN.write('3');
    CAN.endPacket();

    delay(SendDelay);

    if ((millis() - Lastfaultswitch) > Faultinterval)
    {
      Faultbyte ^= 16;
      Lastfaultswitch = millis();
      // Serial.println ((Faultbyte >> 4));
    }

    CAN.beginExtendedPacket(0x0Cf00107);
    CAN.write(0xff);
    CAN.write(0XFF);
    CAN.write(0XFF);
    CAN.write(Faultbyte);
    CAN.write(0xff);
    CAN.write(0XFF);
    CAN.write(0XFF);
    CAN.write(0XFF);
    CAN.write(0XFF);
    CAN.endPacket();

    delay(SendDelay);
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
    // Serial.println("Starting CAN failed!");
    while (1);
  }
}

void setup() {
    Serial.begin(115200); 
    while (!Serial);
    
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
    if (testversion)
    {
      CAN.loopback();
      Lastfaultswitch = millis();
    };
}

void loop() {
  ws.cleanupClients();

  if (++i==9) {
    Serial.printf("[RAM: %d]\r\n", esp_get_free_heap_size());
    i=0;
  }

  if(testversion)
  {
    SendData();
  }
  else
  {
    delay(1000);
  }
}

