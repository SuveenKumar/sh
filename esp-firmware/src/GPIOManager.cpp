#include "GPIOManager.h"
#include <Utils/Constants.h>
#include <LittleFS.h>

void GPIOManager::begin(AsyncWebSocket* ws) {
    this->ws = ws;
    subscribeToMessages();
    Serial.println("🚀 WebPortal started");
}

void GPIOManager::subscribeToMessages() {
    ws->onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                      AwsEventType type, void *arg, uint8_t *data, size_t len) {

        if (type == WS_EVT_DATA) {
            String msg;
            for (size_t i = 0; i < len; i++) msg += (char)data[i];
            Serial.println("📩 WebSocket received message: " + msg);

            if (msg == "led_status") {
                DynamicJsonDocument doc(128);
                JsonArray arr = doc.to<JsonArray>();
                bool* ledStates = FileUtility::loadPinStates();
                for (int i = 0; i < 4; i++) arr.add(ledStates[i]);
                String json;
                serializeJson(doc, json);
                client->text("leds:" + json);
            }
            if (msg.startsWith("led:")) {
                int first = msg.indexOf(":", 4);
                if (first > 0) {
                    int index = msg.substring(4, first).toInt();
                    bool state = msg.substring(first + 1) == "1";
                    if (index >= 0 && index < 4) {
                        bool* ledStates = FileUtility::loadPinStates();
                        ledStates[index] = state;
                        //Active Low !state
                        digitalWrite(LED_PINS[index], !state);
                        FileUtility::savePinStates(ledStates);
                        DynamicJsonDocument doc(128);
                        JsonArray arr = doc.to<JsonArray>();
                        for (int i = 0; i < 4; i++) arr.add(ledStates[i]);
                        String json;
                        serializeJson(doc, json);
                        Serial.println("📣 Notifying all WebSocket clients: " + json);
                        ws-> textAll("leds:" + json);
                    }
                }
            }
        }
    });

    ws->enable(true);
    Serial.println("🧩 WebSocket initialized");
}

