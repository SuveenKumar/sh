#include "WebPortal.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

bool ledStates[4] = {false, false, false, false};
const uint8_t LED_PINS[4] = {D3, D4, D5, D6};

void loadLedStates() {
    if (LittleFS.exists("/leds.json")) {
        File file = LittleFS.open("/leds.json", "r");
        if (file) {
            DynamicJsonDocument doc(256);
            DeserializationError err = deserializeJson(doc, file);
            if (!err) {
                for (int i = 0; i < 4; i++) {
                    ledStates[i] = doc["led" + String(i)] | false;
                    //Active Low !ledStates[i]
                    digitalWrite(LED_PINS[i], !ledStates[i]);
                }
            }
            file.close();
        }
    }
}

void saveLedStates() {
    DynamicJsonDocument doc(256);
    for (int i = 0; i < 4; i++) {
        doc["led" + String(i)] = ledStates[i];
    }
    File file = LittleFS.open("/leds.json", "w");
    serializeJson(doc, file);
    file.close();
}

void WebPortal::begin(CredentialStorage* store, WiFiManager* wifi) {
    this->credentialStorage = store;
    this->wifiManager = wifi;

    for (int i = 0; i < 4; i++) {
        pinMode(LED_PINS[i], OUTPUT);
    }
    loadLedStates();

    setupWebSocket();
    setupCaptivePortalRoutes();

    server.addHandler(&ws);

    if (LittleFS.exists("/index.html")) {
        Serial.println("✅ index.html found");
    } else {
        Serial.println("❌ index.html missing");
    }

    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
    server.begin();

    WiFi.mode(WIFI_AP_STA);
    delay(100);
    WiFi.scanNetworks(true);

    Serial.println("🚀 WebPortal started");
}

void WebPortal::setupWebSocket() {
    ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {

        if (type == WS_EVT_CONNECT) {
            Serial.printf("🔌 WebSocket client connected: %s\n", client->remoteIP().toString().c_str());
            client->text("status:" + wifiManager->getStatusHtml());
        } 
        else if (type == WS_EVT_DATA) {
            String msg;
            for (size_t i = 0; i < len; i++) msg += (char)data[i];
            Serial.println("📩 WebSocket received message: " + msg);

            if (msg == "scan") {
                if (!scanInProgress) {
                    WiFi.scanNetworks(true);
                    scanInProgress = true;
                    scanClientId = client->id();
                }
            } 
            else if (msg.startsWith("submit:")) {
                int sep = msg.indexOf(",");
                if (sep > 7) {
                    String ssid = msg.substring(7, sep);
                    String pass = msg.substring(sep + 1);

                    wifiManager->pendingSSID = ssid;
                    wifiManager->pendingPASS = pass;
                    wifiManager->pendingStore = credentialStorage;
                    wifiManager->pendingConnect = true;
                }
            } 
            else if (msg == "reset") {
                LittleFS.remove("/wifi.json");
                WiFi.disconnect(true);
                //client->text("status:" + wifiManager->getStatusHtml());
                notifyAll("status:" + wifiManager->getStatusHtml());
            } 
            else if (msg == "led_status") {
                DynamicJsonDocument doc(128);
                JsonArray arr = doc.to<JsonArray>();
                for (int i = 0; i < 4; i++) arr.add(ledStates[i]);
                String json;
                serializeJson(doc, json);
                client->text("leds:" + json);
            }
            else if (msg.startsWith("led:")) {
                int first = msg.indexOf(":", 4);
                if (first > 0) {
                    int index = msg.substring(4, first).toInt();
                    bool state = msg.substring(first + 1) == "1";
                    if (index >= 0 && index < 4) {
                        ledStates[index] = state;
                        //Active Low !state
                        digitalWrite(LED_PINS[index], !state);
                        saveLedStates();
                        DynamicJsonDocument doc(128);
                        JsonArray arr = doc.to<JsonArray>();
                        for (int i = 0; i < 4; i++) arr.add(ledStates[i]);
                        String json;
                        serializeJson(doc, json);
                        notifyAll("leds:" + json);
                    }
                }
            }
        }
    });

    ws.enable(true);
    Serial.println("🧩 WebSocket initialized");
}

void WebPortal::setupCaptivePortalRoutes() {
    server.on("/generate_204", HTTP_ANY, serveIndex);
    server.on("/hotspot-detect.html", HTTP_ANY, serveIndex);
    server.on("/redirect", HTTP_ANY, serveIndex);
    server.on("/ncsi.txt", HTTP_ANY, serveIndex);
    server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(204);
    });
    server.on("/leds", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(LittleFS, "/leds.html", "text/html");
    });
    server.onNotFound(serveIndex);
}

void WebPortal::serveIndex(AsyncWebServerRequest* request) {
    request->send(LittleFS, "/index.html", "text/html");
}

void WebPortal::notifyAll(const String& message) {
    Serial.println("📣 Notifying all WebSocket clients: " + message.substring(7));
    ws.textAll(message);
}

void WebPortal::loop() {
    int result = WiFi.scanComplete();

    if (scanInProgress && result >= 0) {
        int n = result;
        String json = "[";
        for (int i = 0; i < n; ++i) {
            if (i > 0) json += ",";
            json += "\"" + WiFi.SSID(i) + "\"";
        }
        json += "]";
        WiFi.scanDelete();

        AsyncWebSocketClient* target = ws.client(scanClientId);
        if (target && target->canSend()) {
            target->text("scan:" + json);
        }

        scanInProgress = false;
        scanClientId = -1;
        WiFi.scanNetworks(true);
    } 
    else if (scanInProgress && result == -2) {
        scanInProgress = false;
        scanClientId = -1;
        WiFi.scanNetworks(true);
    }
}
