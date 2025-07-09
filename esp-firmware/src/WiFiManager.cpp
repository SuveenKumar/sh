#include "WiFiManager.h"
#include "Utils/FileUtility.h"
#include <LittleFS.h>
#include "Utils/Constants.h"
#include "Models/WiFiConfig.h"
#include "Models/Status.h"

#include "Utils/CommonUtility.h"
#include "Models/ScanResults.h"
#include "Utils/Log.h"

void WiFiManager::begin(AsyncWebSocket* ws) {
    this->ws = ws;

    // Start Access Point
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP("ESP-Config");
    
    Log::Info("📡 AP Started: IP = " + WiFi.softAPIP().toString());
    FallbackToDefaultSSID();
    Log::Info("🚀 WiFiManager started");
}

void WiFiManager::loop() {
    checkPendingConnection();
    scanNetworks();
    if (millis() - lastAttempt >= IDLE_TIMEOUT) {
        lastAttempt = millis();
        FallbackToDefaultSSID();
    }
}

void WiFiManager::handleWebSocketMessage(AsyncWebSocketClient *client, AwsEventType eventType, String msg) {
        if (eventType == WS_EVT_CONNECT) {
            Log::Info("🔌 WebSocket client connected: "+ client->remoteIP().toString());
            Status status;            
            status.status = getStatusHtml();
            status.connected = status.status != "❌ Not connected";
            ws-> textAll(status.serialize());
        } 
        if (eventType == WS_EVT_DATA) {
            String type = CommonUtility::getMessageType(msg);
            if (type == "Scan") {
                Log::Info("📩 WebSocket received message: " + msg);
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
                Log::Info("📩 WebSocket received message: " + msg);
                WiFiConfig config;
                config.deserialize(msg);
                Log::Info("SSID: " + config.ssid);
                pendingSSID = config.ssid;
                pendingPASS = config.password;
                pendingConnect = true;
            } 
            if (type == "Reset") {
                Log::Info("📩 WebSocket received message: " + msg);
                LittleFS.remove("/wifi.json");
                WiFi.disconnect(true);
                Status status;
                status.connected = false;
                status.status = getStatusHtml();
                ws-> textAll(status.serialize());
            } 
        }
}

void WiFiManager::FallbackToDefaultSSID() {
    if (pendingConnect) {
        Log::Info("⏭️ Skipping fallback: pending connection in progress");
        return;
    }

    if (WiFi.isConnected()) return;

    String savedSSID, savedPassword;
    if (!FileUtility::loadCredentials(savedSSID, savedPassword)) {
        //Log::Info("⚠️ No credentials found for fallback");
        return;
    }

    Log::Info("📶 Attempting fallback connection to SSID: " + savedSSID + savedPassword);
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(100);
        yield();
        Serial.print(".");
    }
    Serial.println();
    if (WiFi.status() == WL_CONNECTED) {
        Log::Info("✅ Fallback connected to: " + savedSSID);
        Status status;            
        status.status = getStatusHtml();
        status.connected = true;
        ws-> textAll(status.serialize());    
    } 
    else {
        Log::Info("❌ Fallback failed to connect");
        Status status;
        status.connected = false;
        status.status = getStatusHtml();
        ws-> textAll(status.serialize());    
    }
}

void WiFiManager::checkPendingConnection() {
    if (!pendingConnect) return;

    Log::Info("⏳ Attempting connection to submitted SSID: " + pendingSSID + " "+ pendingPASS);

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
        Log::Info("✅ Connected to: " + pendingSSID);
        if (FileUtility::saveCredentials(pendingSSID, pendingPASS)) {
            Log::Info("💾 Credentials saved successfully");
        } else {
            Log::Info("⚠️ Connected, but failed to save credentials");
        }
        Status status;            
        status.status = getStatusHtml();
        status.connected = true;
        ws-> textAll(status.serialize());
    } else {
        Log::Info("❌ Failed to connect to: " + pendingSSID);
        Status status;            
        status.status = getStatusHtml();
        status.connected = false;
        ws-> textAll(status.serialize());
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