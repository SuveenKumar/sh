#pragma once
#include <Arduino.h>
#include <vector>

struct MessageData
{
    String type;
    std::vector<String> values;
};

class MessageParser
{
public:
    // Parses "Type|val1|val2|...|valN"
    static MessageData parse(const String &input);

    // Builds "Type|val1|val2|...|valN"
    static String build(MessageData messageData);
};
