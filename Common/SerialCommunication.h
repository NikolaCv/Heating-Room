#ifndef SERIALCOMMUNICATION_H
#define SERIALCOMMUNICATION_H

#include <Arduino.h>
#include <ArduinoJson.h>

class SerialCommunication
{
	public:
		StaticJsonDocument<200> ReadJsonFromSerial(Stream& input) const;
		void SendToSerial(Stream& outputSerial, char code, String message = "");
		void SendJsonToSerial(Stream& outputSerial, char code, StaticJsonDocument<200> jsonDocument);
		
};

#endif
