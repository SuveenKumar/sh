#include "WebPortal.h"
#include <LittleFS.h>

void WebPortal::begin(CredentialStorage* store, WiFiManager* wifi) {
    this->credentialStorage = store;
    this->wifiManager = wifi;

    setupWebSocket();
    server.addHandler(&ws);
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
            Serial.println("🔌 WebSocket client connected");
            client->text("status:" + wifiManager->getStatusHtml());
        } 
        else if (type == WS_EVT_DATA) {
            AwsFrameInfo *info = (AwsFrameInfo *)arg;
            String msg;
            for (size_t i = 0; i < len; i++) msg += (char)data[i];

            Serial.println("📩 WebSocket received message: " + msg);

            if (msg == "scan") 
            {
                if (!scanInProgress)
                {
        Serial.println("📶 Starting async WiFi scan...");
        WiFi.scanNetworks(true);           // Start async scan
        scanInProgress = true;
        scanClientId = client->id();       // Store client to reply later
    }
    else 
    {
        Serial.println("⏳ Scan already in progress");
    }
    
}

            else if (msg.startsWith("submit:")) 
            {
                int sep = msg.indexOf(",");
                if (sep > 7) 
                {
                    String ssid = msg.substring(7, sep);
                    String pass = msg.substring(sep + 1);

                    Serial.println("📝 Received credentials via WebSocket:");
                    Serial.println("   SSID: " + ssid);
                    Serial.println("   PASS: " + pass);

                    wifiManager->pendingSSID = ssid;
                    wifiManager->pendingPASS = pass;
                    wifiManager->pendingStore = credentialStorage;
                    wifiManager->pendingConnect = true;
                }
                else 
                {
                    Serial.println("⚠️ Invalid submit format. Expected submit:ssid,password");
                }
            }
            else if (msg == "reset") {
    Serial.println("🧼 Received WiFi reset request");
    LittleFS.remove("/wifi.json");
    WiFi.disconnect(true);
    client->text("status:"+wifiManager->getStatusHtml());
}
            else {
                Serial.println("⚠️ Unknown WebSocket message received: " + msg);
            }
        }
    });

    ws.enable(true);
    Serial.println("🧩 WebSocket initialized");
}

void WebPortal::notifyAll(const String& message) {
    Serial.println("📣 Notifying all WebSocket clients: " + message.substring(7));
    ws.textAll(message);
}

void WebPortal::loop() {
    if (scanInProgress && WiFi.scanComplete() >= 0) {
        int n = WiFi.scanComplete();
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
}

