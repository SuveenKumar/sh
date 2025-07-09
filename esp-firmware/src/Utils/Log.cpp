#include "Log.h"

void Log::Info(const String& info) {
    Serial.println(String(millis()) + ": " + info);
}
