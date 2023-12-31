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
//volatile bool triggeredIR = false;

unsigned long currTime = millis();
/*
void IRAM_ATTR InterruptCallback()
{
	triggeredIR = true;
}*/

void setup()
{
	Serial.begin(9600);
	sensors.SetupIRSensors();
	sensors.SetupVibrationSensor(VIBRATION_PIN);	    
	
	/*for (int i = 0; i < numSensorsIR; ++i)
	{
        pinMode(irSensorPins[i], INPUT);
		attachInterrupt(digitalPinToInterrupt(irSensorPins[i]), InterruptCallback, RISING);
    }*/
}

void loop()
{
	currTime = millis();
	/*if(triggeredIR)
	{
		Serial.println(currTime);
		triggeredIR = false;
	}*/
	serialComm.ReadFromSerial(Serial);

	serialComm.SendTriggeredIR(currTime, Serial);
	serialComm.SendTriggeredVibration(currTime, Serial);

	//delay(10);
}
