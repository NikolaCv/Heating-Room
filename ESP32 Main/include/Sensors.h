#ifndef SENSORS_H
#define SENSORS_H

#include <Adafruit_SHT31.h>
#include <HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "ACS712.h"

class Sensors 
{
	public:
		void SetupCurrentSensor(const int sensorNumber, ACS712_type sensorType, const int sensorPin);
		void SetupDallasTempSensor(const int sensorPin);
		void SetupSHT3xTempSensor();
		void SetupHX711WeightSensor();
		void SetupRelayPin(int relayPin);
		void ToggleRelay();

		float GetCurrent(const int sensorNumber, const int numMeasurements) const;
		float GetDallasTemp(const int numMeasurements);
		float* GetSHT3xTempHumidity(const int numMeasurements) const;
		float GetWeight(const int numMeasurements) const;
		bool GetRelayState() const;

	private:
		//Adafruit_SHT31 sht31 = Adafruit_SHT31();
		//HX711 scale;

		ACS712 currentSensor[2];

		OneWire oneWire;
		DallasTemperature dallasSensor;

		int relayPin;
		bool relayState; // false = LOW, true = HIGH
};

#endif