#ifndef ESPMAINSERIALCOMMUNICATION_H
#define ESPMAINSERIALCOMMUNICATION_H

#include <ArduinoJson.h>
#include "SerialCommunication.h"
#include "MqttCommunication.h"
#include "../src/Configuration/DefaultConfig.h"

class EspMainSerialCommunication : public SerialCommunication
{
	public:
		EspMainSerialCommunication(MqttCommunication mqtt, const int vibrationResetMqttSeconds, const unsigned int lastVibrationTime,
									const char* flapVibrationTopic, const char* flapBlockadeTopic, const char* flapPositionTopic,
									const char* configValuesTopic);
		void ReadFromSerial(Stream& inputSerial, const unsigned int currTime);

		void SetVibrationConfigValue(const int newVibrationResetMqttSeconds);
		void ResetVibrationConfigValue();
		int GetVibrationConfigValue() const;

	private:
		MqttCommunication mqtt;

		StaticJsonDocument<200> publishTopics;

		int vibrationResetMqttSeconds;
		unsigned int lastVibrationTime;
		int blockade = 0;
};

#endif
