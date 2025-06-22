#include "WiFiManager.h"

void WiFiManager::begin(CredentialStorage* store) {
    storage = store;
    WiFi.mode(WIFI_AP_STA);
    Serial.println("🔧 WiFiManager initialized in AP+STA mode");
}

void WiFiManager::FallbackToDefaultSSID() {
    if (pendingConnect) {
        Serial.println("⏭️ Skipping fallback: pending connection in progress");
        return;
    }

    if (WiFi.isConnected()) {
        // Already connected, no fallback needed
        return;
    }

    String savedSSID, savedPassword;
    if (!storage->load(savedSSID, savedPassword)) {
        Serial.println("⚠️ No credentials found for fallback");
        return;
    }

    Serial.println("📶 Attempting fallback connection to SSID: " + savedSSID);
    WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(100);
        yield();
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ Fallback connected to: " + savedSSID);
    } else {
        Serial.println("❌ Fallback failed to connect");
    }
}

void WiFiManager::checkPendingConnection() {
    if (!pendingConnect) return;

    Serial.println("⏳ Attempting connection to submitted SSID: " + pendingSSID);
    WiFi.begin(pendingSSID.c_str(), pendingPASS.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(100);
        yield();
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ Connected to: " + pendingSSID);
        if (pendingStore && pendingStore->save(pendingSSID, pendingPASS)) {
            Serial.println("💾 Credentials saved successfully");
        } else {
            Serial.println("⚠️ Connected, but failed to save credentials");
        }
    } else {
        Serial.println("❌ Failed to connect to: " + pendingSSID);
    }

    pendingConnect = false;
}

void WiFiManager::startCaptivePortal() {
    IPAddress apIP = WiFi.softAPIP();
    Serial.println("🌐 Captive portal started at: " + apIP.toString());
    dnsServer.start(53, "*", apIP);
}

void WiFiManager::loop() {
    dnsServer.processNextRequest();
    checkPendingConnection();      // High priority
    if (millis() - lastAttempt >= IDLE_TIMEOUT)
    {
        lastAttempt = millis();
        FallbackToDefaultSSID();   // Try fallback only if idle
    }
}

String WiFiManager::getStatusHtml() {
    if (WiFi.status() == WL_CONNECTED) 
    {
        return "✅ Connected to: " + WiFi.SSID() + "<br>IP: " + WiFi.localIP().toString();
    }
    if(pendingConnect){
        return "⏳ Attempting connection to: " + pendingSSID;
    }
    return "❌ Not connected";
}
