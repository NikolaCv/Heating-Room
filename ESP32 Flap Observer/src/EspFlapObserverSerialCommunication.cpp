#include "EspFlapObserverSerialCommunication.h"

EspFlapObserverSerialCommunication::EspFlapObserverSerialCommunication(SensorsFlapObserver& sensors,
																	   int debounceIRMillis, int debounceVibrationMillis, int blockadeThreshold)
: sensors(sensors),
  debounceIRMillis(debounceIRMillis), debounceVibrationMillis(debounceVibrationMillis), blockadeThreshold(blockadeThreshold)
{
}

void EspFlapObserverSerialCommunication::ReadFromSerial(Stream &inputSerial)
{	
	while (Serial.available() > 0)
	{
		char incomingChar = Serial.read();

		switch (incomingChar)
		{
			case 'R': // Reset config to default
			{
				SetConfigValues(DefaultConfig::debounceIRMillis, DefaultConfig::debounceVibrationMillis, DefaultConfig::blockadeThreshold);
				SendConfigValues(inputSerial);
				break;
			}

			case 'G': // Get config (ESP Main is requesting values, send them)
			{
				SendConfigValues(inputSerial);
				break;
			}

			case 'S': // Set config (ESP Main SENT new values, update them)
			{
				StaticJsonDocument<200> jsonDocument = ReadJsonFromSerial(inputSerial);
				SetConfigValues(jsonDocument["debounceIRMillis"], jsonDocument["debounceVibrationMillis"], jsonDocument["blockadeThreshold"]);
				SendConfigValues(inputSerial);
				break;
			}
		}
	}
}

void EspFlapObserverSerialCommunication::SendConfigValues(Stream& serial)
{
	StaticJsonDocument<200> jsonDocument;
	jsonDocument["debounceIRMillis"] = debounceIRMillis;
	jsonDocument["debounceVibrationMillis"] = debounceVibrationMillis;
	jsonDocument["blockadeThreshold"] = blockadeThreshold;
	
	SendJsonToSerial(serial, 'S', jsonDocument);
}

void EspFlapObserverSerialCommunication::SetConfigValues(int newDebounceIRMillis, int newDebounceVibrationMillis, int newBlockadeThreshold)
{	
	debounceIRMillis = newDebounceIRMillis;
	debounceVibrationMillis = newDebounceVibrationMillis;
	blockadeThreshold = newBlockadeThreshold;
}

void EspFlapObserverSerialCommunication::SendTriggeredIR(const unsigned int currTime, Stream& serial)
{
	if (currTime - sensors.GetLastIRTime() > debounceIRMillis)
	{
		int triggeredIR = sensors.TriggeredIR(currTime);
		if (triggeredIR != -1)
			serial.print(triggeredIR);
	}
}

void EspFlapObserverSerialCommunication::SendTriggeredVibration(const unsigned int currTime, Stream& serial)
{	
	if (currTime - sensors.GetLastVibrationTime() > debounceVibrationMillis && sensors.TriggeredVibration(currTime))
	{
		if (sensors.GetVibrationFlapDiff() > blockadeThreshold)
			serial.print('X'); // Blockade
		else
			serial.print('V'); // Vibration
	}
}
