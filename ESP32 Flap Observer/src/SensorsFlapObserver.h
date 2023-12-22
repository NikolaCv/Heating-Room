#ifndef SENSORS_FLAP_OBSERVER_H
#define SENSORS_FLAP_OBSERVER_H

#include <SPI.h>
#include <ArduinoJson.h>


class SensorsFlapObserver 
{
	public:
		SensorsFlapObserver(int* irSensorPins, int& numSensorsIR);
		void SetupIRSensors();
		void SetupVibrationSensor(int sensorPin);

		int TriggeredIR(unsigned long currTime);
		bool TriggeredVibration(unsigned long currTime);
		void UpdateCurrentIR();

		int GetVibrationFlapDiff();
		unsigned long GetLastIRTime();
		unsigned long GetLastVibrationTime();

		static void IRAM_ATTR InterruptIRCallback();
		static void IRAM_ATTR InterruptVibrationCallback();

	private:	
		int* irSensorPins;
		int numSensorsIR;

		int previousIR = -1, currentIR = -1;
		unsigned long lastIRTime = 0, lastVibrationTime = 0;

		int vibrationFlapDiff = 0;

		static volatile bool triggeredIR;
		static volatile bool triggeredVibration;
};

#endif