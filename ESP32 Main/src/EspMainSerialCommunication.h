#ifndef ESP_MAIN_SERIAL_COMMUNICATION_H
#define ESP_MAIN_SERIAL_COMMUNICATION_H

#include <ArduinoJson.h>
#include "SerialCommunication.h"
#include "../src/Configuration/DefaultConfig.h"

class MqttCommunication;
class SensorsMain;

class EspMainSerialCommunication : public SerialCommunication
{
	public:
		EspMainSerialCommunication() = default;
		EspMainSerialCommunication(const int vibrationResetMqttSeconds, const unsigned int lastVibrationTime,
									const char* flapVibrationTopic, const char* flapBlockadeTopic, const char* flapPositionTopic,
									const char* configValuesTopic);
		void ReadFromSerial(Stream& inputSerial, MqttCommunication& mqtt, SensorsMain& sensors, unsigned int currTime);

		void SetVibrationConfigValue(const int newVibrationResetMqttSeconds);
		void ResetVibrationConfigValue();
		int GetVibrationConfigValue() const;

	private:
		StaticJsonDocument<200> publishTopics;

		int vibrationResetMqttSeconds;
		unsigned int lastVibrationTime;
		
		int blockade = 0;
};

#endif