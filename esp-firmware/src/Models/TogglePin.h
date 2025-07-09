#pragma once
#include "BaseModel.h"

class TogglePin : public BaseModel {
public:
    int index;
    bool state;
    
private:
    void fromJson(JsonDocument& doc) override {
        index = doc["index"];
        state = doc["state"];
    }

    void toJson(JsonDocument& doc) override {
        doc["index"] = index;
        doc["state"] = state;
    }

    const String getType() const override {
        return "TogglePin";
    }
};
