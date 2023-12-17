#include "Sensors.h"


void Sensors::SetupCurrentSensor(int sensorNumber, ACS712_type sensorType, int sensorPin)
{
	currentSensor[sensorNumber] = ACS712(sensorType, sensorPin);
	currentSensor[sensorNumber].calibrateAC();
}

void Sensors::SetupDallasTempSensor(int sensorPin)
{
	oneWire = OneWire(sensorPin);
	DallasSensor = DallasTemperature(&oneWire);
	DallasSensor.begin();
}

void Sensors::SetupSHT3xTempSensor()
{
	// TODO
}

void Sensors::SetupHX711WeightSensor()
{
	// TODO
}
void Sensors::SetupRelayPin(int relayPin)
{
	this->relayPin = relayPin;
	pinMode(relayPin, OUTPUT);
	digitalWrite(relayPin, LOW);
	relayState = false;
}

float Sensors::GetCurrent(int sensorNumber, int numMeasurements) const
{
	float I = 0;

	for (int i = 0; i < numMeasurements; ++i)
		I += currentSensor[sensorNumber].getCurrentAC();

	return I / numMeasurements;
}

float Sensors::GetDallasTemp(int numMeasurements)
{
	DallasSensor.requestTemperatures();
	float temp = 0;

	for (int i = 0; i < numMeasurements; ++i)
		temp += DallasSensor.getTempCByIndex(0);

	return temp / numMeasurements;
}

float* Sensors::GetSHT3xTempHumidity(int numMeasurements) const
{

}

float Sensors::GetWeight(int numMeasurements) const
{

}

bool Sensors::GetRelayState() const
{
	return relayState;
}