#include "CredentialStorage.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

bool CredentialStorage::begin() {
    return LittleFS.begin();
}

bool CredentialStorage::save(const String& ssid, const String& password) {
    File file = LittleFS.open("/wifi.json", "w");
    if (!file) return false;

    DynamicJsonDocument doc(512);
    JsonObject obj = doc.to<JsonObject>();
    obj["ssid"] = ssid;
    obj["password"] = password;

    if (serializeJson(doc, file) == 0) {
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool CredentialStorage::load(String& ssid, String& password) {
    if (!LittleFS.exists("/wifi.json")) return false;

    File file = LittleFS.open("/wifi.json", "r");
    if (!file) return false;

    DynamicJsonDocument doc(512);
    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err) {
        Serial.println("Cannot Deserialize SSID and Password from Saved File");
        return false;
    }

    ssid = doc["ssid"] | "";
    password = doc["password"] | "";

    if (ssid.isEmpty()) {
        Serial.println("Saved SSID is empty");
        return false;
    }
    return true;
}
