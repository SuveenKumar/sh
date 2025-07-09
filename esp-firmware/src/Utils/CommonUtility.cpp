#include "CommonUtility.h"
#include <Utils/Constants.h>
#include <ArduinoJson.h>

void CommonUtility::wipeAllMemory() {
  // Format FS
  LittleFS.begin();
  LittleFS.format();

  // Clear EEPROM
  EEPROM.begin(512);
  for (int i = 0; i < 512; i++) EEPROM.write(i, 0xFF);
  EEPROM.commit();
  EEPROM.end();

  // Forget WiFi
  WiFi.disconnect(true); // Also clears SDK flash WiFi creds

  Log::Info("All memory wiped.");
}

bool CommonUtility::checkResetButton() {
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    delay(100);
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        Log::Info("Reset button held...");
        return true;
    }
    return false;
}

String CommonUtility::getMessageType(const String msg) {
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, msg);
    if (!error) {
       return doc["type"] | "";
    } 
    Log::Info("⚠️ JSON Parse Error: " + String(error.c_str()));
    return "";
}