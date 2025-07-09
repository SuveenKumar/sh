#pragma once
#include "BaseModel.h"
#include <vector>

class ScanResults : public BaseModel {
public:
    std::vector<String> results;
    
private:     
    void fromJson(JsonDocument& doc) override {
        results.clear();
        JsonArray jsonArray = doc["results"];
        for (JsonVariant v : jsonArray) {
            results.push_back(v.as<String>());
        }
    }

    void toJson(JsonDocument& doc) override {
        JsonArray jsonArray = doc["results"].to<JsonArray>();
        for (const auto ssid : results) {
            jsonArray.add(ssid);
        }
    }

    const String getType() const override {
        return "Scan";
    }
};
