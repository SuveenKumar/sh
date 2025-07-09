#pragma once
#include "BaseModel.h"
#include <vector>

class PinsState : public BaseModel {
public:
    std::vector<uint8_t> pinsState{0, 0, 0, 0};
    
private:     
    void fromJson(JsonDocument& doc) override {
        pinsState.clear();
        JsonArray jsonArray = doc["pinsState"];
        for (JsonVariant v : jsonArray) {
            pinsState.push_back(v.as<int>());
        }
    }

    void toJson(JsonDocument& doc) override {
        JsonArray jsonArray = doc["pinsState"].to<JsonArray>();
        for (const auto ssid : pinsState) {
            jsonArray.add(ssid);
        }
    }

    const String getType() const override {
        return "PinsState";
    }
};
