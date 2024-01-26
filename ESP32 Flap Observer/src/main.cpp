#include <Arduino.h>
#include <ArduinoJson.h>

#include "Configuration/DefaultConfig.h"
#include "SensorsFlapObserver.h"
#include "EspFlapObserverSerialCommunication.h"

#define VIBRATION_PIN 4

int irSensorPins[7] = {25, 33, 32, 35, 34, 39, 26};
int numSensorsIR = 7;

SensorsFlapObserver sensors(irSensorPins, numSensorsIR);
EspFlapObserverSerialCommunication serialComm(sensors,
											  DefaultConfig::debounceIRMillis,
											  DefaultConfig::debounceVibrationMillis,
											  DefaultConfig::blockadeThreshold);

unsigned long currTime = millis();


void setup()
{
	Serial.begin(9600);
	sensors.SetupIRSensors();
	sensors.SetupVibrationSensor(VIBRATION_PIN);	    
}

void loop()
{
	currTime = millis();

	serialComm.ReadFromSerial(Serial);

	serialComm.SendTriggeredIR(currTime, Serial);
	serialComm.SendTriggeredVibration(currTime, Serial);

	delay(5);
}
