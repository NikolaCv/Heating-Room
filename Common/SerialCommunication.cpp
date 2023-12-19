#include "SerialCommunication.h"

StaticJsonDocument<200> SerialCommunication::ReadJson(Stream& input) const
{
    String jsonString;
    while (input.available() > 0)
	{
        char incomingChar = input.read();
        jsonString += incomingChar;
    }

    StaticJsonDocument<200> jsonDocument;
    DeserializationError error = deserializeJson(jsonDocument, jsonString);

    if (error)
	{
        input.print(F("Parsing failed: "));
        input.println(error.c_str());
        return StaticJsonDocument<200>();
    }

    return jsonDocument;
}

void SerialCommunication::SendJson(Stream& output, int debounceMillisIR, int debounceMillisVibration, int blockadeThreshold) const
{
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["debounceMillisIR"] = debounceMillisIR;
  jsonDocument["debounceMillisVibration"] = debounceMillisVibration;
  jsonDocument["blockadeThreshold"] = blockadeThreshold;

  String jsonString;
  serializeJson(jsonDocument, jsonString);

  output.print(jsonString);
}

bool SerialCommunication::ValidConfig(StaticJsonDocument<200>& jsonDocument) const
{
	if (jsonDocument.isNull())
	{
		//Serial.println("Failed to parse JSON.");
		return false;
	}

	if (!jsonDocument.containsKey("debounceMillisIR") ||
		!jsonDocument.containsKey("debounceMillisVibration") ||
		!jsonDocument.containsKey("blockadeThreshold"))
	{
		//Serial.println("JSON structure is missing some keys.");
		return false;
	}
		
	return true;
}
