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
		void SetupCurrentSensor(int sensorNumber, ACS712_type sensorType, int sensorPin);
		void SetupDallasTempSensor(int sensorPin);
		void SetupSHT3xTempSensor();
		void SetupHX711WeightSensor();
		void SetupRelayPin(int relayPin);
		void ToggleRelay();

		float GetCurrent(int sensorNumber, int numMeasurements) const;
		float GetDallasTemp(int numMeasurements);
		float* GetSHT3xTempHumidity(int numMeasurements) const;
		float GetWeight(int numMeasurements) const;
		bool GetRelayState() const;

	private:
		//Adafruit_SHT31 sht31 = Adafruit_SHT31();
		//HX711 scale;

		ACS712 currentSensor[2];

		OneWire oneWire;
		DallasTemperature DallasSensor;

		int relayPin;
		bool relayState; // false = LOW, true = HIGH
};

#endif