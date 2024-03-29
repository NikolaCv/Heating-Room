#include "EspMainSerialCommunication.h"
#include "MqttCommunication.h"

EspMainSerialCommunication::EspMainSerialCommunication(const int vibrationResetMqttSeconds, const unsigned int lastVibrationTime,
													   const char *flapVibrationTopic, const char *flapBlockadeTopic, const char *flapPositionTopic,
													   const char *configValuesTopic)
	: vibrationResetMqttSeconds(vibrationResetMqttSeconds), lastVibrationTime(lastVibrationTime),
	  flapVibrationTopic(flapVibrationTopic), flapBlockadeTopic(flapBlockadeTopic),
	  flapPositionTopic(flapPositionTopic), configValuesTopic(configValuesTopic)
{
}

void EspMainSerialCommunication::ReadFromSerial(Stream &inputSerial, MqttCommunication &mqtt, SensorsMain &sensors, const unsigned int currTime)
{
	if (currTime - lastVibrationTime > vibrationResetMqttSeconds * 1000)
		mqtt.PublishMessage(flapVibrationTopic, String(0));

	while (inputSerial.available() > 0)
	{
		char incomingChar = inputSerial.read();

		switch (incomingChar)
		{
		case 'V': // Vibration
		{
			mqtt.PublishMessage(flapVibrationTopic, String(1));
			lastVibrationTime = millis();

			if (blockade)
			{
				blockade = 0;
				mqtt.PublishMessage(flapBlockadeTopic, String(blockade));
			}
			break;
		}

		case 'X': // Blockade
		{
			if (!blockade)
				blockade = 1;
			mqtt.PublishMessage(flapBlockadeTopic, String(blockade));
			break;
		}

		case 'S': // ESP Flap Observer SENT current config values, publish them
		{
			StaticJsonDocument<200> jsonDocument = ReadJsonFromSerial(inputSerial);
			jsonDocument["samplingRateSeconds"] = mqtt.GetSamplingRateSeconds();
			jsonDocument["vibrationResetMqttSeconds"] = GetVibrationConfigValue();

			StaticJsonDocument<200> sensorConfig = sensors.GetConfig();

			// Add keys from sensorConfig to jsonDocument
			for (JsonPair obj : sensorConfig.as<JsonObject>())
				jsonDocument[obj.key()] = obj.value();

			String jsonString;
			serializeJson(jsonDocument, jsonString);
			mqtt.PublishMessage(configValuesTopic, jsonString);
			break;
		}

		default: // Numbers 0-6 for which IR diode is activated, ignore if not in that range
		{
			if (incomingChar >= '0' && incomingChar <= '6')
				mqtt.PublishMessage(flapPositionTopic, String(incomingChar));
			break;
		}
		}
	}
}

void EspMainSerialCommunication::ResetVibrationConfigValue()
{
	vibrationResetMqttSeconds = DefaultConfig::vibrationResetMqttSeconds;
}

void EspMainSerialCommunication::SetVibrationConfigValue(const int newVibrationResetMqttSeconds)
{
	vibrationResetMqttSeconds = newVibrationResetMqttSeconds;
}

int EspMainSerialCommunication::GetVibrationConfigValue() const
{
	return vibrationResetMqttSeconds;
}
