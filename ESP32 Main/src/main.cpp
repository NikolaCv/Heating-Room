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
#define LOAD_CELL_SCK 34
#define RELAY_PIN 17
#define DS18B20_PIN 33
#define ACS712_1_PIN 32
#define ACS712_2_PIN 39
/*
// mSD card reader pins
#define SD_CS_PIN 5
#define SD_MOSI_PIN 23
#define SD_MISO_PIN 19
#define SD_SCK_PIN 18
*/


/* TODO

topics:
Prilikom reseta/update treba da se prikazu vrednosti u HA (3 prikaza mozda, default, current, set new)
request => main update, main prosledi klapni => klapna update i vrati => main publish na heating_room/config

TODO
u svakom get sensor, i u svakom setup sensor treba proveriti da li je senzor prikljucen
(osim za ACS712, i mozda za Dallas)

*/

void ReconnectWiFi();
void SetupSensors(SensorsMain& sensors);
void PublishSensorData(MqttCommunication& mqtt, SensorsMain& sensors);

//Adafruit_SHT31 sht31 = Adafruit_SHT31();
//HX711 scale;

SensorsMain sensors;
WiFiClient espClient;
PubSubClient mqttClient(espClient);

MqttCommunication mqtt(mqttClient, Secrets::mqttServerIP, Secrets::mqttPort,
					   "ESP32 Heating Room", Secrets::mqttUserName, Secrets::mqttUserPassword,
					   DefaultConfig::samplingRateSeconds, sensors, Serial,
					   "heating_room/relay/control", // sub
					   "heating_room/config/reset", // sub
					   "heating_room/config/update"); // sub

EspMainSerialCommunication serialComm(DefaultConfig::vibrationResetMqttSeconds, millis(),
									  "heating_room/flap/vibration", // pub
									  "heating_room/flap/blockade", // pub
									  "heating_room/flap/position", // pub
									  "heating_room/config"); // pub

unsigned long currTime = millis();

void setup()
{	
	Serial.begin(9600);
	ReconnectWiFi();

	mqtt.Setup(serialComm);
	SetupSensors(sensors);

	Serial.print('G'); // Get values from ESP Flap Observer, which triggers config values to be published
}

void loop()
{
	currTime = millis();
	Serial.println(currTime);
	ReconnectWiFi();

	mqtt.Reconnect();
	mqtt.Loop();
	mqtt.PublishDataPeriodically(PublishSensorData, sensors);

	serialComm.ReadFromSerial(Serial, mqtt, currTime);
}

void SetupSensors(SensorsMain& sensors)
{
	sensors.SetupCurrentSensor(0, ACS712_30A, ACS712_1_PIN);
	sensors.SetupCurrentSensor(1, ACS712_30A, ACS712_2_PIN);
	sensors.SetupDallasTempSensor(DS18B20_PIN);
	sensors.SetupRelayPin(RELAY_PIN);
}

void PublishSensorData(MqttCommunication& mqtt, SensorsMain& sensors)
{
	mqtt.PublishMessage("heating_room/burner", String(sensors.GetCurrent(0, 10), 2));
	mqtt.PublishMessage("heating_room/pump", String(sensors.GetCurrent(1, 10), 2));
	
	// blocking function, potentially split into 2 functions (requestTemperatures(), getTempCByIndex())
	// first requestTemp, then after 1 second getTemp (then delay every other sensor, so all publish at the same time?)
	mqtt.PublishMessage("heating_room/furnace/temp", String(sensors.GetDallasTemp(1), 2));
}

// Rewrite for offline data collecting
void ReconnectWiFi() 
{
	if(WiFi.status() != WL_CONNECTED)
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