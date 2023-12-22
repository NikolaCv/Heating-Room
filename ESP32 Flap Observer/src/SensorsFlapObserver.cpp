#include "SensorsFlapObserver.h"

volatile bool SensorsFlapObserver::triggeredIR = false;
volatile bool SensorsFlapObserver::triggeredVibration = false;

SensorsFlapObserver::SensorsFlapObserver(int* irSensorPins, int& numSensorsIR)
: irSensorPins(irSensorPins), numSensorsIR(numSensorsIR)
{
}

void SensorsFlapObserver::SetupIRSensors()
{
	for (int i = 0; i < this->numSensorsIR; ++i)
	{
        pinMode(this->irSensorPins[i], INPUT);
		attachInterrupt(digitalPinToInterrupt(this->irSensorPins[i]), InterruptIRCallback, RISING);
    }
}

void SensorsFlapObserver::SetupVibrationSensor(int sensorPin)
{
	pinMode(sensorPin, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(sensorPin), InterruptVibrationCallback, CHANGE);
}

int SensorsFlapObserver::TriggeredIR(unsigned long currTime)
{
	UpdateCurrentIR();
	if (previousIR == currentIR) return -1;

	vibrationFlapDiff = 0;
	lastIRTime = currTime;
	previousIR = currentIR;

	return currentIR;
}

void SensorsFlapObserver::UpdateCurrentIR()
{
	if (triggeredIR)
	{
		triggeredIR = false;
		for (int i = 0; i < 7; i++)
		{
			if (digitalRead(irSensorPins[i]))
			{
				currentIR = i;
				break;
			}
		}
	}
}

bool SensorsFlapObserver::TriggeredVibration(unsigned long currTime)
{
	if (!triggeredVibration) return false;

	lastVibrationTime = currTime;
	vibrationFlapDiff++;
	triggeredVibration = false;

	return true;
}

int SensorsFlapObserver::GetVibrationFlapDiff()
{
	return vibrationFlapDiff;
}

unsigned long SensorsFlapObserver::GetLastIRTime()
{
	return lastIRTime;
}

unsigned long SensorsFlapObserver::GetLastVibrationTime()
{
	return lastVibrationTime;
}

void IRAM_ATTR SensorsFlapObserver::InterruptIRCallback()
{
	triggeredIR = true;
}

void IRAM_ATTR SensorsFlapObserver::InterruptVibrationCallback()
{
	triggeredVibration = true;
}
