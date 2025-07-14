#include <Utils/Constants.h>
#include "CommonUtility.h"

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

  CommonUtility::LogInfo("All memory wiped.");
}

bool CommonUtility::checkResetButton() {
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    delay(100);
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        CommonUtility::LogInfo("Reset button held...");
        return true;
    }
    return false;
}

bool CommonUtility::saveStringToFile(const String& path, const String& content) {
    File file = LittleFS.open(path, "w");
    if (!file) {
        CommonUtility::LogInfo("❌ Failed to open file for writing");
        return false;
    }
    size_t written = file.print(content);
    file.close();

    return written == content.length();
}

String CommonUtility::loadStringFromFile(const String& path) {
    if (!LittleFS.exists(path))
    {
        CommonUtility::LogInfo("❌ Path doesnot exist");
        return "0000";
    } 
    File file = LittleFS.open(path, "r");
    if (!file) {
        CommonUtility::LogInfo("❌ Failed to open file");
        return "0000";
    }
    String content = file.readString();
    file.close();

    return content;
}

void CommonUtility::LogInfo(const String& info) {
    Serial.println(String(millis())+ ", Free heap: " + String(ESP.getFreeHeap()) + ": " + info);
}