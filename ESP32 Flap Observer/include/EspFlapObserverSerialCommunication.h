#ifndef ESPFLAPOBSERVERSERIALCOMMUNICATION_H
#define ESPFLAPOBSERVERSERIALCOMMUNICATION_H

#include <ArduinoJson.h>
#include "SerialCommunication.h"
#include "../src/Configuration/DefaultConfig.h"

class EspFlapObserverSerialCommunication : public SerialCommunication
{
	public:
		EspFlapObserverSerialCommunication(MqttCommunication mqtt, const int vibrationResetMqttSeconds, const unsigned int lastVibrationTime,
									const char* flapVibrationTopic, const char* flapBlockadeTopic, const char* flapPositionTopic,
									const char* configValuesTopic);
		void ReadFromSerial(Stream& inputSerial, const unsigned int currTime);

		void SetVibrationConfigValue(const int newVibrationResetMqttSeconds);
		void ResetVibrationConfigValue();
		int GetVibrationConfigValue() const;

	private:
};

#endif
