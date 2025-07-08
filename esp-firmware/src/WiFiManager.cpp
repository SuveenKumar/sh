#include "WiFiManager.h"
#include "Utils/FileUtility.h"
#include <LittleFS.h>
#include "Utils/Constants.h"
#include "Models/WiFiConfig.h"
#include "Models/Status.h"

#include "Utils/CommonUtility.h"
#include "Models/ScanResults.h"

void WiFiManager::begin(AsyncWebSocket* ws) {
    this->ws = ws;
    subscribeToMessages();

    // Start Access Point
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("ESP-Config");
    
    Serial.println("📡 AP Started: IP = " + WiFi.softAPIP().toString());
    FallbackToDefaultSSID();
}

void WiFiManager::subscribeToMessages() {
    ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {

        if (type == WS_EVT_CONNECT) {
            Serial.printf("🔌 WebSocket client connected: %s\n", client->remoteIP().toString().c_str());
            
            Status status;            
            status.status = getStatusHtml();
            status.connected = status.status != "❌ Not connected";
            ws-> textAll(status.serialize());
        } 
        if (type == WS_EVT_DATA) {
            String msg;
            for (size_t i = 0; i < len; i++) msg += (char)data[i];
            Serial.println("📩 WebSocket received message: " + msg);

            String type = CommonUtility::getMessageType(msg);
            if (type == "Scan") {
                if (!scanInProgress) {
                    WiFi.scanNetworks(true);
                    scanInProgress = true;
                    scanClientId = client->id();
                    Status status;
                    status.connected = true;
                    status.status = getStatusHtml();
                    client->text(status.serialize());
                }
            } 
            if (type == "Submit") {
                WiFiConfig config;
                config.deserialize(msg);
                Serial.println("SSID: " + config.ssid);
                pendingSSID = config.ssid;
                pendingPASS = config.password;
                pendingConnect = true;
            } 
            if (type == "Reset") {
                LittleFS.remove("/wifi.json");
                WiFi.disconnect(true);
                //client->text("status:" + wifiManager->getStatusHtml());
                Status status;
                status.connected = false;
                status.status = getStatusHtml();
                ws-> textAll(status.serialize());
            } 
        }
    });
}

void WiFiManager::FallbackToDefaultSSID() {
    if (pendingConnect) {
        Serial.println("⏭️ Skipping fallback: pending connection in progress");
        return;
    }

    if (WiFi.isConnected()) return;

    String savedSSID, savedPassword;
    if (!FileUtility::loadCredentials(savedSSID, savedPassword)) {
        Serial.println("⚠️ No credentials found for fallback");
        return;
    }

    Serial.println("📶 Attempting fallback connection to SSID: " + savedSSID + savedPassword);
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(100);
        yield();
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ Fallback connected to: " + savedSSID);
        Status status;            
        status.status = getStatusHtml();
        status.connected = true;
        ws-> textAll(status.serialize());    
    } 
    else {
        Serial.println("❌ Fallback failed to connect");
        Status status;
        status.connected = false;
        status.status = getStatusHtml();
        ws-> textAll(status.serialize());    
    }
}

void WiFiManager::checkPendingConnection() {
    if (!pendingConnect) return;

    Serial.println("⏳ Attempting connection to submitted SSID: " + pendingSSID + " "+ pendingPASS);

    Status status;            
    status.status = getStatusHtml();
    status.connected = false;
    ws-> textAll(status.serialize());

    WiFi.begin(pendingSSID.c_str(), pendingPASS.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(100);
        yield();
        Serial.print(".");
    }
    Serial.println();
    pendingConnect = false;

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ Connected to: " + pendingSSID);
        if (FileUtility::saveCredentials(pendingSSID, pendingPASS)) {
            Serial.println("💾 Credentials saved successfully");
        } else {
            Serial.println("⚠️ Connected, but failed to save credentials");
        }
        Status status;            
        status.status = getStatusHtml();
        status.connected = true;
        ws-> textAll(status.serialize());
    } else {
        Serial.println("❌ Failed to connect to: " + pendingSSID);
        Status status;            
        status.status = getStatusHtml();
        status.connected = false;
        ws-> textAll(status.serialize());
    }

}

void WiFiManager::loop() {
    checkPendingConnection();
    scanNetworks();
    if (millis() - lastAttempt >= IDLE_TIMEOUT) {
        lastAttempt = millis();
        FallbackToDefaultSSID();
    }
}

String WiFiManager::getStatusHtml() {
    if (pendingConnect) {
        return "⏳ Attempting connection to: " + pendingSSID;
    }
    if (WiFi.status() == WL_CONNECTED) {
        return "✅ Connected to: " + WiFi.SSID() + "  " + WiFi.localIP().toString();
    }
    return "❌ Not connected";
}

void WiFiManager::scanNetworks() {
    int result = WiFi.scanComplete();
    if (scanInProgress && result >= 0) {
        ScanResults scanResults;
        for (int i = 0; i < result; ++i) {
            scanResults.results.push_back(WiFi.SSID(i));
        }
        WiFi.scanDelete();

        AsyncWebSocketClient* target = ws->client(scanClientId);
        if (target && target->canSend()) {
            target->text(scanResults.serialize());
        }

        scanInProgress = false;
        scanClientId = -1;

        // Optionally scan again
        WiFi.scanNetworks(true);
    } 
    else if (scanInProgress && result == -2) {
        // Scan never started or was cleared; reset and try again
        scanInProgress = false;
        scanClientId = -1;
        WiFi.scanNetworks(true);
    }
}