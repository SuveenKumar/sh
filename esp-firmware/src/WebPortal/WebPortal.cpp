#include "WebPortal.h"
#include <LittleFS.h>

void WebPortal::begin(CredentialStorage* store, WiFiManager* wifi) {
    this->credentialStorage = store;
    this->wifiManager = wifi;

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
                    Serial.println("📶 Starting async WiFi scan...");
                    WiFi.scanNetworks(true);
                    scanInProgress = true;
                    scanClientId = client->id();
                } else {
                    Serial.println("⏳ Scan already in progress");
                }
            } 
            else if (msg.startsWith("submit:")) {
                int sep = msg.indexOf(",");
                if (sep > 7) {
                    String ssid = msg.substring(7, sep);
                    String pass = msg.substring(sep + 1);

                    Serial.println("📝 Received credentials via WebSocket:");
                    Serial.println("   SSID: " + ssid);
                    Serial.println("   PASS: " + pass);

                    wifiManager->pendingSSID = ssid;
                    wifiManager->pendingPASS = pass;
                    wifiManager->pendingStore = credentialStorage;
                    wifiManager->pendingConnect = true;
                } else {
                    Serial.println("⚠️ Invalid submit format. Expected submit:ssid,password");
                }
            } 
            else if (msg == "reset") {
                Serial.println("🧼 Received WiFi reset request");
                LittleFS.remove("/wifi.json");
                WiFi.disconnect(true);
                client->text("status:" + wifiManager->getStatusHtml());
            } 
            else {
                Serial.println("⚠️ Unknown WebSocket message received: " + msg);
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
        request->send(204); // No Content
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

        Serial.println("📡 Scan complete. Sending to client: " + json);
        AsyncWebSocketClient* target = ws.client(scanClientId);
        if (target && target->canSend()) {
            target->text("scan:" + json);
        }

        scanInProgress = false;
        scanClientId = -1;
        WiFi.scanNetworks(true); // Restart for future
    } 
    else if (scanInProgress && result == -2) {
        Serial.println("⚠️ WiFi scan failed");
        scanInProgress = false;
        scanClientId = -1;
        WiFi.scanNetworks(true);
    }
}
