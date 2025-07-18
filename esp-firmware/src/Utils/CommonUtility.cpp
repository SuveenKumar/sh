#include <Utils/Constants.h>
#include "CommonUtility.h"
#include <espnow.h>

void CommonUtility::wipeAllMemory()
{
    // Format FS
    LittleFS.begin();
    LittleFS.format();

    // Clear EEPROM
    EEPROM.begin(512);
    for (int i = 0; i < 512; i++)
        EEPROM.write(i, 0xFF);
    EEPROM.commit();
    EEPROM.end();

    // Forget WiFi
    WiFi.disconnect(true); // Also clears SDK flash WiFi creds

    CommonUtility::LogInfo("All memory wiped.");
}

bool CommonUtility::checkResetButton()
{
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    delay(100);
    if (digitalRead(RESET_BUTTON_PIN) == LOW)
    {
        CommonUtility::LogInfo("Reset button held...");
        return true;
    }
    return false;
}

bool CommonUtility::saveStringToFile(const String &path, const String &content)
{
    File file = LittleFS.open(path, "w");
    if (!file)
    {
        CommonUtility::LogInfo("❌ Failed to open file for writing");
        return false;
    }
    size_t written = file.print(content);
    file.close();

    return written == content.length();
}

String CommonUtility::loadStringFromFile(const String &path)
{
    if (!LittleFS.exists(path))
    {
        //CommonUtility::LogInfo("❌ Path doesnot exist");
        return "";
    }
    File file = LittleFS.open(path, "r");
    if (!file)
    {
        CommonUtility::LogInfo("❌ Failed to open file");
        return "";
    }
    String content = file.readString();
    file.close();

    return content;
}

void CommonUtility::LogInfo(const String &info)
{
    Serial.println(String(millis()) + ", Free heap: " + String(ESP.getFreeHeap()) + ": " + info);
}

bool CommonUtility::sendEspNowMessage(const uint8_t* peerMacAddress, const String& message) {
  int result = esp_now_send((uint8_t*)peerMacAddress, (uint8_t*)message.c_str(), message.length());
  if (result == 0) {
    CommonUtility::LogInfo("✅ ESP-NOW message sent successfully to: " + message);
    return true;
  } else {
    CommonUtility::LogInfo("❌ Failed to send ESP-NOW message.");
    return false;
  }
}

bool CommonUtility::broadcastEspNowMessage(const String& message) {
  int result = esp_now_send(BROADCAST_MAC_ADDRESS, (uint8_t*)message.c_str(), message.length());
  if (result == 0) {
    CommonUtility::LogInfo("✅ ESP-NOW message Broadcasted successfully: "+ message);
    return true;
  } else {
    CommonUtility::LogInfo("❌ Failed to send ESP-NOW message.");
    return false;
  }
}