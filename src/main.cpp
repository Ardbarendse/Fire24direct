/**
 * ----------------------------------------------------------------------------
 * ESP32 Remote Control with WebSocket
 * ----------------------------------------------------------------------------
 * © 2020 Stéphane Calderoni
 * ----------------------------------------------------------------------------
 */

#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// WiFi credentials
const char *ssid = "Fire24test";
const char *password =  "test1234";
const int dns_port = 53;
const int http_port = 80;
const int ws_port = 1337;

int teller_1 = 1;
int teller_2 = 2;
int i = 0;

AsyncWebServer server(http_port);
AsyncWebSocket ws("/ws");
AsyncWebSocketClient* client;
AsyncEventSource events("/events");

// SPIFFS initialization
// ----------------------------------------------------------------------------

void initSPIFFS() {
  if (SPIFFS.begin()) {
    Serial.println("Initiated SPIFFS volume...");
    }
    else {
      Serial.print("Attempting initiating SPIFFS");
      while (1) {
        Serial.print(".");
    }
    Serial.println("Done!");
  }
}

// Connecting to the WiFi network
// ----------------------------------------------------------------------------

void initWiFi() {
  WiFi.softAP(ssid, password);
  Serial.println();
  Serial.println("AP running");
  Serial.print("My IP address: ");
  Serial.println(WiFi.softAPIP());
}

// Web server initialization
// ----------------------------------------------------------------------------

String processor(const String &var) {
    if(var == "COUNTER_1_VALUE"){
        return String(teller_1);
    }
    else if(var == "COUNTER_2_VALUE"){
        return String(teller_2);
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
    Serial.println("Webserver initialized");
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

        DynamicJsonDocument jsonc2w(2048);
        const char *action = jsonw2c["action"];
        if (strcmp(action, "toggle_1") == 0) {
            jsonc2w["teller_1"] = (String)++teller_1;
        }
        if (strcmp(action, "toggle_2") == 0) {
            jsonc2w["teller_2"] = (String)++teller_2;
        }
        if (strcmp(action, "toggle_1+2") == 0) {
            jsonc2w["teller_1"] = (String)++teller_1;
            jsonc2w["teller_2"] = (String)++teller_2;
        }
        char buffer[2048];
        serializeJson(jsonc2w, buffer);
        Serial.println(buffer);
        ws.textAll(buffer);
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

// Initialization
// ----------------------------------------------------------------------------

void setup() {
    Serial.begin(115200); delay(500);

    initSPIFFS();
    initWiFi();
    initWebSocket();
    initWebServer();
}

// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    ws.cleanupClients();
    if (++i==9) {
        Serial.printf("[RAM: %d]\r\n", esp_get_free_heap_size());
        i=0;
        }
    delay(1000);
}

