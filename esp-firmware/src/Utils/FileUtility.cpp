#include "FileUtility.h"
#include <Utils/Constants.h>
#include <LittleFS.h>
#include "Log.h"
#include "Models/PinsState.h"

bool FileUtility::mountFileSystem(){
     // Mount LittleFS
    if (!LittleFS.begin()) {
        Log::Info("❌ LittleFS mount failed!");
        return false;
    }

    Log::Info("✅ LittleFS mounted");
    // List all files for debugging
    Log::Info("📂 Listing LittleFS files:");
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        Log::Info("  " + dir.fileName());
    }
    return true;
}

PinsState FileUtility::loadPinStates() {
    PinsState pinsState;
    if (LittleFS.exists("/leds.json")) {
        File file = LittleFS.open("/leds.json", "r");
        if (file) {
            String jsonStr = file.readString();
            Serial.println("Suveen");
            pinsState.deserialize(jsonStr);
            Serial.println("Suveen");
        }
        else{
            Log::Info("Pin States not Found");
        }
        file.close();
    }
        return pinsState;
}

void FileUtility::savePinStates(PinsState ledStates) {
    File file = LittleFS.open("/leds.json", "w");
    if (!file) {
        Serial.println("❌ Failed to open file for writing");
        return;
    }

  file.print(ledStates.serialize());
  file.close();
  Serial.println("✅ JSON saved to LittleFS");
}

bool FileUtility::loadCredentials(String& ssid, String& password) {
    if (!LittleFS.exists("/wifi.json")) return false;

    File file = LittleFS.open("/wifi.json", "r");
    if (!file) return false;

    DynamicJsonDocument doc(512);
    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err) {
        Log::Info("Cannot Deserialize SSID and Password from Saved File");
        return false;
    }

    ssid = doc["ssid"] | "";
    password = doc["password"] | "";

    if (ssid.isEmpty()) {
        Log::Info("Saved SSID is empty");
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