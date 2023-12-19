#ifndef SERIALCOMMUNICATION_H
#define SERIALCOMMUNICATION_H

#include <Arduino.h>
#include <ArduinoJson.h>

class SerialCommunication
{
	protected:
		StaticJsonDocument<200> ReadJson(Stream& input) const;
		void SendJson(Stream& output, int debounceMillisIR, int debounceMillisVibration, int blockadeThreshold) const;
		bool ValidConfig(StaticJsonDocument<200>& jsonDocument) const;
};

#endif
