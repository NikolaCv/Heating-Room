#ifndef SENSORS_MAIN_H
#define SENSORS_MAIN_H

#include <Adafruit_SHT31.h>
#include <HX711.h>
#include <OneWire.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <vector>
#include <numeric>
#include <algorithm>

#include "ACS712.h"
#include "Configuration/DefaultConfig.h"

class SensorsMain 
{
	public:
		SensorsMain(float loadCellCalibrationFactor, float loadCellOffset, float emptyTankWeight, float furnaceOnThreshold, float furnaceOffThreshold);
		void SetupRelay(int relayPin);
		void SetupCurrentSensor(int sensorNumber, ACS712_type sensorType, int sensorPin, float currentOffset);
		void SetupDallasTempSensor(const int sensorPin);
		void SetupSHT3xTempSensor(const int sensorAddress);
		void SetupHX711WeightSensor(const int doutPin, const int sckPin);

		float CalculateConsumption();
		
		void ToggleRelayState();
		void UpdateRelayState();

		void SetRelayOutput();
		void SetFurnaceState();
		bool FurnaceStateHasChanged();

		bool GetRelayState() const;
		int GetFurnaceState() const;
		std::pair<float, float> GetCurrent(const int sensorNumber, const int numMeasurements);
		float GetFilteredValuesDelta();
		float GetDallasTemp(const int numMeasurements);
		std::pair<float, float> GetSHT3xTempHumidity(const int numMeasurements);
		float GetWeightGrams(const int numMeasurements);
		float GetPelletWeightGrams(const int numMeasurements);
		float GetLoadCellCalibrationFactor() const;

		void SetConfig(float newLoadCellCalibrationFactor, float newLoadCellOffset, float newEmptyTankWeight, float newFurnaceOnThreshold, float newFurnaceOffThreshold);
		void ResetConfig();
		StaticJsonDocument<200> GetConfig() const;

	private:
		Adafruit_SHT31 sht3x;
		int sht3xAddress;
		HX711 loadCell;
  		float loadCellCalibrationFactor, loadCellOffset;
		float emptyTankWeight;
		float weightBeforeFurnaceProcess, weightAfterFurnaceProcess;

		ACS712 currentSensor[2];
		std::vector<std::vector<float>> currentValues;
		float filteredValues[2];
		float filteredValuesDelta;
		int windowSize = 10;

		bool furnaceState;
		bool furnaceStateHasChanged;
		float furnaceOnThreshold, furnaceOffThreshold;

		OneWire oneWire;
		DallasTemperature dallasSensor;

		int relayPin;
		bool relayState; // false = LOW, true = HIGH
};

#endif