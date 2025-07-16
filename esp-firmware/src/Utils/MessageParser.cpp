#include "MessageParser.h"

MessageData MessageParser::parse(const String &input)
{
    MessageData data;

    unsigned int start = 0;
    int end = input.indexOf('|');

    // Get type
    if (end == -1)
    {
        data.type = input;
        return data;
    }

    data.type = input.substring(start, end);
    start = end + 1;

    // Get values
    while ((end = input.indexOf('|', start)) != -1)
    {
        data.values.emplace_back(input.substring(start, end));
        start = end + 1;
    }

    if (start < input.length())
    {
        data.values.emplace_back(input.substring(start));
    }

    return data;
}

String MessageParser::build(MessageData messageData)
{
    String result = messageData.type;
    for (const auto &value : messageData.values)
    {
        result += '|';
        result += value;
    }
    return result;
}
