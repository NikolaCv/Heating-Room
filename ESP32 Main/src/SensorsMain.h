#ifndef SENSORS_MAIN_H
#define SENSORS_MAIN_H

#include <Adafruit_SHT31.h>
#include <HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "ACS712.h"

class SensorsMain 
{
	public:
		void SetupRelayPin(int relayPin);
		void SetupCurrentSensor(int sensorNumber, ACS712_type sensorType, int sensorPin);
		void SetupDallasTempSensor(const int sensorPin);
		void SetupSHT3xTempSensor();
		void SetupHX711WeightSensor();
		
		void ToggleRelay();
		void UpdateRelayState();
		void SetRelayOutput();

		bool GetRelayState() const;
		float GetCurrent(const int sensorNumber, const int numMeasurements) const;
		float GetDallasTemp(const int numMeasurements);
		float* GetSHT3xTempHumidity(const int numMeasurements) const;
		float GetWeight(const int numMeasurements) const;
	private:
		//Adafruit_SHT31 sht31 = Adafruit_SHT31();
		//HX711 scale;

		ACS712 currentSensor[2];

		OneWire oneWire;
		DallasTemperature dallasSensor;

		int relayPin;
		bool relayState = false; // false = LOW, true = HIGH
};

#endif