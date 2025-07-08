#include "FileUtility.h"
#include <Utils/Constants.h>
#include <LittleFS.h>

bool FileUtility::mountFileSystem(){
     // Mount LittleFS
    if (!LittleFS.begin()) {
        Serial.println("❌ LittleFS mount failed!");
        return false;
    }

    Serial.println("✅ LittleFS mounted");
    // List all files for debugging
    Serial.println("📂 Listing LittleFS files:");
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        Serial.println("  " + dir.fileName());
    }
    return true;
}

bool* FileUtility::loadPinStates() {
    bool* ledStates = nullptr;
    if (LittleFS.exists("/leds.json")) {
        File file = LittleFS.open("/leds.json", "r");
        if (file) {
            DynamicJsonDocument doc(256);
            DeserializationError err = deserializeJson(doc, file);
            if (!err) {
                for (int i = 0; i < 4; i++) {
                    if(doc.containsKey("led" + String(i))){
                       ledStates[i] = doc["led" + String(i)] | false;
                       Serial.print(ledStates[i]+", ");
                    }

                }
                Serial.println();
            }
            else{
                Serial.print("Pin States not saved");

            }
            file.close();
        }
    }
    return ledStates;
}

void FileUtility::savePinStates(bool ledStates[4]) {
    DynamicJsonDocument doc(256);
    for (int i = 0; i < 4; i++) {
        doc["led" + String(i)] = ledStates[i];
    }
    File file = LittleFS.open("/leds.json", "w");
    serializeJson(doc, file);
    file.close();
}

bool FileUtility::loadCredentials(String& ssid, String& password) {
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

bool FileUtility::saveCredentials(const String& ssid, const String& password) {
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