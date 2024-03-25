#include <Adafruit_SHT31.h>
#include <HX711.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "ACS712.h"
#include "Configuration/Secrets.h"
#include "Configuration/DefaultConfig.h"

#include "SensorsMain.h"
#include "MqttCommunication.h"
#include "ESPMainSerialCommunication.h"

#define SHT3X_ADDRESS 0x44
#define LOAD_CELL_DOUT 35
#define LOAD_CELL_SCK 32
#define RELAY_PIN 17
#define DS18B20_PIN 33
#define ACS712_1_PIN 34
#define ACS712_2_PIN 39

float currentOffset1 = 0;
float currentOffset2 = 0;

/*
// mSD card reader pins
#define SD_CS_PIN 5
#define SD_MOSI_PIN 23
#define SD_MISO_PIN 19
#define SD_SCK_PIN 18
*/

void ReconnectWiFi();
void SetupSensors(SensorsMain &sensors);
void PublishSensorData(MqttCommunication &mqtt, SensorsMain &sensors);

SensorsMain sensors(DefaultConfig::loadCellCalibrationFactor, DefaultConfig::loadCellOffset, DefaultConfig::emptyTankWeight,
					DefaultConfig::furnaceOnThreshold, DefaultConfig::furnaceOffThreshold);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

EspMainSerialCommunication serialComm(DefaultConfig::vibrationResetMqttSeconds, millis(),
									  "heating_room/flap/vibration", // pub
									  "heating_room/flap/blockade",	 // pub
									  "heating_room/flap/position",	 // pub
									  "heating_room/config");		 // pub

MqttCommunication mqtt(mqttClient, Secrets::mqttServerIP, Secrets::mqttPort,
					   "ESP32 Heating Room", Secrets::mqttUserName, Secrets::mqttUserPassword,
					   DefaultConfig::samplingRateSeconds, sensors, Serial,
					   serialComm,
					   "heating_room/relay/state",	  // pub
					   "heating_room/relay/control",  // sub
					   "heating_room/config/reset",	  // sub
					   "heating_room/config/update"); // sub

unsigned long currTime = millis();

void setup()
{
	delay(3000);
	Serial.begin(9600);
	ReconnectWiFi();

	mqtt.Setup(serialComm);
	SetupSensors(sensors);

	Serial.print('G'); // Get values from ESP Flap Observer, which triggers config values to be published
}

void loop()
{
	currTime = millis();

	ReconnectWiFi();

	mqtt.Reconnect();
	mqtt.Loop();
	mqtt.PublishDataPeriodically(PublishSensorData, sensors);

	serialComm.ReadFromSerial(Serial, mqtt, sensors, currTime);
	GetAnalogCalibrationValues(50);
	GetCurrentCalibrationValues(50);
	// Serial.println(sensors.GetWeightGrams(1)); // For Calibration

	delay(25);
}

void SetupSensors(SensorsMain &sensors)
{
	sensors.SetupCurrentSensor(0, ACS712_30A, ACS712_1_PIN, currentOffset1);
	sensors.SetupCurrentSensor(1, ACS712_30A, ACS712_2_PIN, currentOffset2);
	sensors.SetupDallasTempSensor(DS18B20_PIN);
	sensors.SetupRelay(RELAY_PIN);
	sensors.SetupSHT3xTempSensor(SHT3X_ADDRESS);
	sensors.SetupHX711WeightSensor(LOAD_CELL_DOUT, LOAD_CELL_SCK);
}

void PublishSensorData(MqttCommunication &mqtt, SensorsMain &sensors)
{
	std::pair<float, float> result = sensors.GetCurrent(0, 12);
	mqtt.PublishMessage("heating_room/burner/filtered", String(result.first, 3));
	mqtt.PublishMessage("heating_room/burner/raw", String(result.second, 3));

	result = sensors.GetCurrent(1, 12);
	mqtt.PublishMessage("heating_room/pump/filtered", String(result.first, 3));
	mqtt.PublishMessage("heating_room/pump/raw", String(result.second, 3));

	mqtt.PublishMessage("heating_room/burner_minus_pump/filtered", String(sensors.GetFilteredValuesDelta(), 3));

	if (sensors.FurnaceStateHasChanged())
	{
		mqtt.PublishMessage("heating_room/furnace/state", String(1 - sensors.GetFurnaceState()));
		mqtt.PublishMessage("heating_room/furnace/state", String(sensors.GetFurnaceState()));
	}
	// Dallas blocking function, potentially split into 2 functions (requestTemperatures(), getTempCByIndex())
	float temp = sensors.GetDallasTemp(5);
	if (temp > 0)
		mqtt.PublishMessage("heating_room/furnace/temp", String(temp, 2));

	result = sensors.GetSHT3xTempHumidity(1);
	if (result.first != -77)
	{
		mqtt.PublishMessage("heating_room/outside/temp", String(result.first, 2));
		mqtt.PublishMessage("heating_room/outside/humidity", String(result.second, 2));
	}

	float weight = sensors.GetPelletWeightGrams(8) / 1000.0;
	if (weight > -5)
		mqtt.PublishMessage("heating_room/tank", String(weight, 3));

	float consumption = sensors.CalculateConsumption() / 1000.0;
	if (consumption)
		mqtt.PublishMessage("heating_room/consumption", String(consumption, 3));
}

// Rewrite for offline data collecting if needed
void ReconnectWiFi()
{
	if (WiFi.status() != WL_CONNECTED)
	{
		WiFi.begin(Secrets::wifiSSID, Secrets::wifiPassword);

		while (WiFi.status() != WL_CONNECTED)
		{
			Serial.println("Connecting to WiFi...");
			delay(1000);
		}

		Serial.println("Connected to WiFi");
	}
}

void GetAnalogCalibrationValues(int n)
{
	while (1)
	{
		Serial.println("senzor1 start");
		for (int i = 0; i < n; ++i)
			Serial.println(analogRead(ACS712_1_PIN));

		Serial.println("senzor1 end");
		Serial.println("senzor2 start");
		for (int i = 0; i < n; ++i)
			Serial.println(analogRead(ACS712_1_PIN));
		Serial.println("senzor2 end");
	}
}

void GetCurrentCalibrationValues(int n)
{
	while (1)
	{
		Serial.println("senzor1 start");
		for (int i = 0; i < n; ++i)
			Serial.println(sensors.GetCurrent(0, 12).second);

		Serial.println("senzor1 end");
		Serial.println("senzor2 start");
		for (int i = 0; i < n; ++i)
			Serial.println(sensors.GetCurrent(1, 12).second);
		Serial.println("senzor2 end");
	}
}