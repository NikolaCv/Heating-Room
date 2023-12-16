#include "SerialJson.h"

StaticJsonDocument<200> SerialJson::ReadJson(Stream& input)
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

void SerialJson::SendJson(int debounceMillisIR, int debounceMillisVibration, int blockadeThreshold)
{
  StaticJsonDocument<200> jsonDocument;
  jsonDocument["debounceMillisIR"] = debounceMillisIR;
  jsonDocument["debounceMillisVibration"] = debounceMillisVibration;
  jsonDocument["blockadeThreshold"] = blockadeThreshold;

  String jsonString;
  serializeJson(jsonDocument, jsonString);

  Serial.println(jsonString);
}


bool SerialJson::ValidConfig(StaticJsonDocument<200>& jsonDocument)
{
	if (jsonDocument.isNull())
	{
		Serial.println("Failed to parse JSON.");
		return false;
	}

	if (!jsonDocument.containsKey("debounceMillisIR") ||
		!jsonDocument.containsKey("debounceMillisVibration") ||
		!jsonDocument.containsKey("blockadeThreshold"))
	{
		Serial.println("JSON structure is missing some keys.");
		return false;
	}
		
	return true;
}
