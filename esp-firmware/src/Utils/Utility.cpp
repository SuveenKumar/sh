#include "Utility.h"
#define RESET_BUTTON_PIN 0

void Utility::wipeAllMemory() {
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

  Serial.println("All memory wiped.");
}

bool checkResetButton() {
    pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
    delay(100);
    if (digitalRead(RESET_BUTTON_PIN) == LOW) {
        Serial.println("Reset button held...");
        return true;
    }
    return false;
}