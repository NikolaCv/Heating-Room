#include "Sensors.h"

void Sensors::SetupCurrentSensor(const int sensorNumber, ACS712_type sensorType, const int sensorPin)
{
	currentSensor[sensorNumber] = ACS712(sensorType, sensorPin);
	currentSensor[sensorNumber].calibrateAC();
}

void Sensors::SetupDallasTempSensor(const int sensorPin)
{
	oneWire = OneWire(sensorPin);
	dallasSensor = DallasTemperature(&oneWire);
	dallasSensor.begin();
}

void Sensors::SetupSHT3xTempSensor()
{
	// TODO
}

void Sensors::SetupHX711WeightSensor()
{
	// TODO
}
void Sensors::SetupRelayPin(const int relayPin)
{
	this->relayPin = relayPin;
	pinMode(relayPin, OUTPUT);
	SetRelayOutput();
	UpdateRelayState();
}

void Sensors::ToggleRelay()
{
	relayState = !relayState;
	SetRelayOutput();
	UpdateRelayState(); // Maybe the change didn't go through, check it
}

void Sensors::UpdateRelayState()
{
	if (digitalRead(relayPin) == HIGH)
		relayState = true;
	else
		relayState = false;
}

void Sensors::SetRelayOutput()
{
	if(relayState)
		digitalWrite(relayPin, HIGH);
	else
		digitalWrite(relayPin, LOW);
}

float Sensors::GetCurrent(const int sensorNumber, const int numMeasurements) const
{
	float I = 0;

	for (int i = 0; i < numMeasurements; ++i)
		I += currentSensor[sensorNumber].getCurrentAC();

	return I / numMeasurements;
}

float Sensors::GetDallasTemp(const int numMeasurements)
{
	dallasSensor.requestTemperatures();
	float temp = 0;

	for (int i = 0; i < numMeasurements; ++i)
		temp += dallasSensor.getTempCByIndex(0);

	return temp / numMeasurements;
}

float* Sensors::GetSHT3xTempHumidity(const int numMeasurements) const
{

}

float Sensors::GetWeight(const int numMeasurements) const
{

}

bool Sensors::GetRelayState() const
{
	return relayState;
}