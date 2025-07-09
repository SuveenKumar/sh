#include "GPIOManager.h"
#include <Utils/Constants.h>
#include <LittleFS.h>
#include "Models/PinsState.h"
#include "Utils/CommonUtility.h"
#include "Models/TogglePin.h"
#include "Utils/Log.h"
#include "Utils/GPIOUtility.h"

void GPIOManager::begin(AsyncWebSocket* ws) {
    this->ws = ws;
    GPIOUtility::SetupGPIOPins();
    if(FileUtility::mountFileSystem()){
       //Load and set Last Pins State from memory
       Log::Info("Setting Last Pins State");
       GPIOUtility::updateAllPinsState(FileUtility::loadPinStates());
    }
    Log::Info("🚀 GPIOManager started");
}

void GPIOManager::loop(){
}

void GPIOManager::handleWebSocketMessage(AsyncWebSocketClient* client, AwsEventType eventType, String msg) {
    if (eventType == WS_EVT_DATA) {
            String type = CommonUtility::getMessageType(msg);
                        Log::Info("TYPE: "+type);
            if (type == "PinsStatus") {
                Log::Info("📩 WebSocket received message: " + msg);
                PinsState state; 
                state = FileUtility::loadPinStates();
                client->text(state.serialize());
            }
            if (type == "TogglePin") {
                Log::Info("📩 WebSocket received message: " + msg);
                TogglePin pinState;
                pinState.deserialize(msg);
                PinsState ledStates = FileUtility::loadPinStates();
                ledStates.pinsState[pinState.index] = pinState.state;
                for(int i=0;i<ledStates.pinsState.size();i++){
                    Serial.println(String(i) +" : "+pinState.state);
                }
                Log::Info("TogglePin: " + String(pinState.index) +" "+ String(pinState.state));
                
                GPIOUtility::updatePinState(pinState.index, pinState.state);
                FileUtility::savePinStates(ledStates);

                String json = ledStates.serialize();
                Log::Info("📣 Notifying all WebSocket clients: " + json);
                ws-> textAll(json);
            }
    }
}

