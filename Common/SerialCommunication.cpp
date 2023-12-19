#include "SerialCommunication.h"

StaticJsonDocument<200> SerialCommunication::ReadJsonFromSerial(Stream& input) const
{
    String jsonString;
    while (input.available() > 0)
	{
        char incomingChar = input.read();
        jsonString += incomingChar;
    }

    StaticJsonDocument<200> jsonDocument;
    deserializeJson(jsonDocument, jsonString);

    return jsonDocument;
}

void SerialCommunication::SendToSerial(Stream& output, char code, String message)
{
	output.print(code);
	output.print(message);
}

void SerialCommunication::SendJsonToSerial(Stream &outputSerial, char code, StaticJsonDocument<200> jsonDocument)
{
	String serialMessage;
	serializeJson(jsonDocument, serialMessage);
	SendToSerial(outputSerial, code, serialMessage);
}