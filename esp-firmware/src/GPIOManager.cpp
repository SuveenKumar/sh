#include "GPIOManager.h"
#include <Utils/Constants.h>
#include <LittleFS.h>
#include "Utils/CommonUtility.h"
#include "Utils/MessageParser.h"

void GPIOManager::begin(AsyncWebSocket* ws) {
    this->ws = ws;
    SetupGPIOPins();
    if(mountFileSystem()){
       //Load and set Last Pins State from memory
       CommonUtility::LogInfo("Setting Last Pins State");
       updateAllPinsState(CommonUtility::loadStringFromFile("/leds.txt"));
    }
    CommonUtility::LogInfo("🚀 GPIOManager started");
}

void GPIOManager::loop(){
}

void GPIOManager::handleWebSocketMessage(AsyncWebSocketClient* client, AwsEventType eventType, String msg) {
    if (eventType == WS_EVT_DATA) {
            MessageData message = MessageParser::parse(msg);
            if (message.type == "PinsStatus") {
                CommonUtility::LogInfo("📩 WebSocket received message: " + msg);
                MessageData response;
                response.type = "PinsState";
                response.values.emplace_back(CommonUtility::loadStringFromFile("/leds.txt"));
                client->text(MessageParser::build(response));
            }
            if (message.type == "TogglePin") {
                int index = message.values[0][0] - '0';
                int state = message.values[1][0] - '0';
                
                String newStates = CommonUtility::loadStringFromFile("/leds.txt");
                newStates[index] = state;
                updatePinState(index, state);

                if(CommonUtility::saveStringToFile("/leds.txt", newStates))
                {
                    Serial.println("✅ JSON saved to LittleFS");
                }

                MessageData response;
                response.type = "PinsState";
                response.values.emplace_back(newStates);

                String json = MessageParser::build(response);
                CommonUtility::LogInfo("📣 Notifying all WebSocket clients: " + json);
                ws-> textAll(json);
            }
    }
}

bool GPIOManager::mountFileSystem(){
     // Mount LittleFS
    if (!LittleFS.begin()) {
        CommonUtility::LogInfo("❌ LittleFS mount failed!");
        return false;
    }

    CommonUtility::LogInfo("✅ LittleFS mounted");
    // List all files for debugging
    CommonUtility::LogInfo("📂 Listing LittleFS files:");
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
        CommonUtility::LogInfo("  " + dir.fileName());
    }
    return true;
}

void GPIOManager::SetupGPIOPins() {
    for (int i = 0; i < NUM_PINS; i++) {
       pinMode(LED_PINS[i], OUTPUT);
    }
}

void GPIOManager::updateAllPinsState(const String& pinStates) {
    for (size_t i = 0; i < pinStates.length(); i++)
    {
        //Active Low
        digitalWrite(LED_PINS[i], !(pinStates[i] - '0'));
    }
}

void GPIOManager::updatePinState(int index, bool state){
    //Active Low
    CommonUtility::LogInfo("updatePinState: "+String(LED_PINS[index])+ ": " + state);
    digitalWrite(LED_PINS[index], !state);
}


