#include "ESPMainSerialCommunication.h"

EspMainSerialCommunication::EspMainSerialCommunication(MqttCommunication mqtt, const int vibrationResetMqttSeconds, const unsigned int lastVibrationTime,
													   const char* flapVibrationTopic, const char* flapBlockadeTopic, const char* flapPositionTopic,
													   const char* configValuesTopic)
: mqtt(mqtt), vibrationResetMqttSeconds(vibrationResetMqttSeconds), lastVibrationTime(lastVibrationTime) 
{	
	topics["vibration"] = flapVibrationTopic;
    topics["blockade"] = flapBlockadeTopic;
    topics["position"] = flapPositionTopic;
    topics["config"] = configValuesTopic;
}

void EspMainSerialCommunication::ReadFromSerial(Stream& inputSerial, const unsigned int currTime)
{
	if(currTime - lastVibrationTime > vibrationResetMqttSeconds * 1000)
		mqtt.PublishMessage(topics["vibration"], String(0));
	
	while (inputSerial.available() > 0)
	{
		char incomingChar = inputSerial.read();
		inputSerial.println(incomingChar);

		switch (incomingChar)
		{
			case 'V': // Vibration
			{
				mqtt.PublishMessage(topics["vibration"], String(1));
				lastVibrationTime = millis();

				if (blockade)
				{
					blockade = 0;
					mqtt.PublishMessage(topics["blockade"], String(blockade));
				}
				break;
			}

			case 'X': // Blockade
			{
				blockade = 1;
				mqtt.PublishMessage(topics["blockade"], String(blockade));
				break;
			}

			case 'S': // ESP Flap Observer SENT current config values, publish them
			{
				StaticJsonDocument<200> jsonDocument = ReadJson(inputSerial);
				if (ValidConfig(jsonDocument))
				{
					String jsonString;
					serializeJson(jsonDocument, jsonString);
					mqtt.PublishMessage(topics["config"], jsonString);
				}
				break;
			}

			// TODO: S set new values will be sent in mqtt callback, as well as R for reset, as well as G for get values

			default: // Numbers 0-6 for which IR diode is activated, ignore if not in that range
			{
				if (incomingChar >= '0' && incomingChar <= '6')
					mqtt.PublishMessage(topics["position"], String(incomingChar), true);
				break;
			}
		}
	}
}