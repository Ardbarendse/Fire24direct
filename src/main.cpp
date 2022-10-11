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

int teller = 0;
String count = "0";

struct Led {
    // state variables
    uint8_t pin;
    bool    on;

    // methods
    void update() {
        Serial.printf("Led update");
    }
};

#define LED_PIN   26
Led    led         = { LED_PIN, true };

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
    if(var == "COUNTER_VALUE"){
        return String(count);
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

void notifyClients() {
    // const uint8_t size = JSON_OBJECT_SIZE(1);
    DynamicJsonDocument json(2048);
    json["status"] = count;
    char buffer[50];
    serializeJson(json, buffer);
    Serial.print("capacity: ");
    Serial.println(json.capacity());
    Serial.print("overflowed: ");
    Serial.println(json.overflowed());
    Serial.print("buffer: ");
    Serial.println(buffer);
    // events.send(count, "COUNTER_VALUE", millis());
    ws.textAll(count);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {

       // const uint8_t size = JSON_OBJECT_SIZE(1);
        DynamicJsonDocument json(2048);
        DeserializationError err = deserializeJson(json, data);
        if (err) {
            Serial.print(F("deserializeJson() failed with code "));
            Serial.println(err.c_str());
            return;
        }

        const char *action = json["action"];
        if (strcmp(action, "toggle") == 0) {
            count = ++teller;
            notifyClients();
            Serial.println("teller:");
            Serial.println(count);
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

// Initialization
// ----------------------------------------------------------------------------

void setup() {
    Serial.begin(115200); delay(500);

    initSPIFFS();
    initWiFi();
    initWebSocket();
    initWebServer();
    notifyClients();
}

// Main control loop
// ----------------------------------------------------------------------------

void loop() {
    ws.cleanupClients();
    Serial.printf("[RAM: %d]\r\n", esp_get_free_heap_size());
    delay(1000);
}

