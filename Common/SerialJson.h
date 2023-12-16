#ifndef SERIALJSON_H
#define SERIALJSON_H

#include <Arduino.h>
#include <ArduinoJson.h>

namespace SerialJson
{
	StaticJsonDocument<200> ReadJson(Stream& input);
	void SendJson(int debounceMillisIR, int debounceMillisVibration, int blockadeThreshold);
	bool ValidConfig(StaticJsonDocument<200>& jsonDocument);
}

#endif
