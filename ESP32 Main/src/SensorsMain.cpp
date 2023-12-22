#include "SensorsMain.h"

void SensorsMain::SetupCurrentSensor(int sensorNumber, ACS712_type sensorType, int sensorPin)
{
	currentSensor[sensorNumber] = ACS712(sensorType, sensorPin);
	currentSensor[sensorNumber].calibrateAC();
}

void SensorsMain::SetupDallasTempSensor(const int sensorPin)
{
	oneWire = OneWire(sensorPin);
	dallasSensor = DallasTemperature(&oneWire);
	dallasSensor.begin();
}

void SensorsMain::SetupSHT3xTempSensor()
{
	// TODO
}

void SensorsMain::SetupHX711WeightSensor()
{
	// TODO
}
void SensorsMain::SetupRelayPin(const int relayPin)
{
	this->relayPin = relayPin;
	pinMode(relayPin, OUTPUT);
	SetRelayOutput();
	UpdateRelayState();
}

void SensorsMain::ToggleRelay()
{
	relayState = !relayState;
	SetRelayOutput();
	UpdateRelayState(); // Maybe the change didn't go through, check it
}

void SensorsMain::UpdateRelayState()
{
	if (digitalRead(relayPin) == HIGH)
		relayState = true;
	else
		relayState = false;
}

void SensorsMain::SetRelayOutput()
{
	if(relayState)
		digitalWrite(relayPin, HIGH);
	else
		digitalWrite(relayPin, LOW);
}

float SensorsMain::GetCurrent(const int sensorNumber, const int numMeasurements) const
{
	float I = 0;

	for (int i = 0; i < numMeasurements; ++i)
		I += currentSensor[sensorNumber].getCurrentAC();

	return I / numMeasurements;
}

float SensorsMain::GetDallasTemp(const int numMeasurements)
{
	dallasSensor.requestTemperatures();
	float temp = 0;

	for (int i = 0; i < numMeasurements; ++i)
		temp += dallasSensor.getTempCByIndex(0);

	return temp / numMeasurements;
}

float* SensorsMain::GetSHT3xTempHumidity(const int numMeasurements) const
{
	return nullptr;
}

float SensorsMain::GetWeight(const int numMeasurements) const
{
	return 0;
}

bool SensorsMain::GetRelayState() const
{
	return relayState;
}
