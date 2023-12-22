#ifndef ESPFLAPOBSERVERSERIALCOMMUNICATION_H
#define ESPFLAPOBSERVERSERIALCOMMUNICATION_H

#include <ArduinoJson.h>
#include "SerialCommunication.h"
#include "../src/Configuration/DefaultConfig.h"
#include "SensorsFlapObserver.h"

class EspFlapObserverSerialCommunication : public SerialCommunication
{
	public:
		EspFlapObserverSerialCommunication(SensorsFlapObserver& sensors,
										   int debounceMillisIR, int debounceMillisVibration, int blockadeThreshold);

		void ReadFromSerial(Stream& inputSerial);

		void SendConfigValues(Stream& serial);
		void SetConfigValues(int newDebounceMillisIR, int newDebounceMillisVibration, int newBlockadeThreshold);

		void SendTriggeredIR(const unsigned int currTime, Stream& serial);
		void SendTriggeredVibration(const unsigned int currTime, Stream& serial);

	private:
		SensorsFlapObserver sensors;

		int debounceIRMillis;
		int debounceVibrationMillis;
		int blockadeThreshold;
};

#endif
