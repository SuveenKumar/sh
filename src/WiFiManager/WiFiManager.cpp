#include "WiFiManager.h"
void WiFiManager::begin(CredentialStorage* store, std::function<void(const String&)> statusCallback) {
    storage = store;
    notifyCallback = statusCallback;
    WiFi.mode(WIFI_AP_STA);
    Serial.println("🔧 WiFiManager initialized in AP+STA mode");
}

void WiFiManager::FallbackToDefaultSSID() {
    if (pendingConnect) {
        Serial.println("⏭️ Skipping fallback: pending connection in progress");
        return;
    }

    if (WiFi.isConnected()) return;

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
        if (notifyCallback) notifyCallback("status:" + getStatusHtml());
    } else {
        Serial.println("❌ Fallback failed to connect");
        if (notifyCallback) notifyCallback("status:"+getStatusHtml());
    }
    pendingConnect = false;
}

void WiFiManager::checkPendingConnection() {
    if (!pendingConnect) return;
    Serial.println("⏳ Attempting connection to submitted SSID: " + pendingSSID);
        if (notifyCallback) notifyCallback("status:" + getStatusHtml());

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
        if (pendingStore && pendingStore->save(pendingSSID, pendingPASS)) {
            Serial.println("💾 Credentials saved successfully");
        } else {
            Serial.println("⚠️ Connected, but failed to save credentials");
        }
        if (notifyCallback) notifyCallback("status:" + getStatusHtml());
    } else {
        Serial.println("❌ Failed to connect to: " + pendingSSID);
        if (notifyCallback) notifyCallback("status:"+ getStatusHtml());
    }

}

void WiFiManager::loop() {
    checkPendingConnection();
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