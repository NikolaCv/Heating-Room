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

#include "Sensors.h"
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
heating_room/relay/control => sub
heating_room/relay/state   => pub

heating_room/config/reset  => sub
heating_room/config/update => sub
heating_room/config		   => pub

Prilikom reseta/update treba da se prikazu vrednosti u HA (3 prikaza mozda, default, current, set new)
request => main update, main prosledi klapni => klapna update i vrati => main publish na heating_room/config

TODO
u svakom get sensor, i u svakom setup sensor treba proveriti da li je senzor prikljucen
(osim za ACS712, i mozda za Dallas)

*/

void SetupInterruptTimer();
void IRAM_ATTR InterruptTimerCallback();
void PublishSensorData();

void ReconnectWiFi();
void SetupMqtt(MqttCommunication mqtt);
void SetupSensors(Sensors sensors);

void ReadFromFlapObserver();
void SendToFlapObserver();

hw_timer_t* mqttTimer = NULL;

//Adafruit_SHT31 sht31 = Adafruit_SHT31();
//HX711 scale;

Sensors sensors;
WiFiClient espClient;
MqttCommunication mqtt(espClient, Secrets::mqttServerIP, Secrets::mqttPort,
					   "ESP32 Heating Room", Secrets::mqttUserName, Secrets::mqttUserPassword,
					   DefaultConfig::samplingRateSeconds, sensors);
EspMainSerialCommunication serialComm(mqtt, DefaultConfig::vibrationResetMqttSeconds, millis(),
									  "heating_room/flap/vibration",
									  "heating_room/flap/blockade",
									  "heating_room/flap/position",
									  "heating_room/config");


namespace NewConfig // FIXME trenutno placeholder, kada se dobiju novi podaci samo se stave u vec postojece variable
{
	int samplingRateSeconds;
	int vibrationResetMqttSeconds;
	int blockadeThreshold;
	int debounceMillisVibration;
	int debounceMillisIR;
}

unsigned long currTime = millis();

void setup()
{	
	Serial.begin(9600);

	ReconnectWiFi();

	SetupMqtt(mqtt, serialComm);
	SetupSensors(sensors);
	SetupInterruptTimer();
}

void loop()
{
	currTime = millis();

	ReconnectWiFi();

	mqtt.Reconnect();
	mqtt.Loop();
	mqtt.PublistDataPeriodically(PublishSensorData);

	serialComm.ReadFromSerial();
}

void SetupMqtt(MqttCommunication mqtt, EspMainSerialCommunication serialComm)
{
	mqtt.Setup(serialComm);
	mqtt.AddSubscribeTopic("heating_room/relay/control");
	mqtt.AddSubscribeTopic("heating_room/config/reset");
	mqtt.AddSubscribeTopic("heating_room/config/update");
	mqtt.Reconnect();
}

void SetupSensors(Sensors sensors)
{
	sensors.SetupCurrentSensor(0, ACS712_30A, ACS712_1_PIN);
	sensors.SetupCurrentSensor(1, ACS712_30A, ACS712_2_PIN);
	sensors.SetupDallasTempSensor(DS18B20_PIN);
	sensors.SetupRelayPin(RELAY_PIN);
}

void PublishSensorData(MqttCommunication mqtt, Sensors sensors)
{		
	mqtt.PublishMessage("heating_room/burner", String(sensors.GetCurrent(0, 20), 2));
	mqtt.PublishMessage("heating_room/pump", String(sensors.GetCurrent(1, 20), 2));

	mqtt.PublishMessage("heating_room/furnace/temp", String(sensors.GetDallasTemp(5), 2));
}

// Rewrite for offline data collecting
void ReconnectWiFi() 
{
	if(WiFi.status() == WL_CONNECTED) return;

	WiFi.begin(Secrets::wifiSSID, Secrets::wifiPassword);

	while (WiFi.status() != WL_CONNECTED)
	{
		Serial.println("Connecting to WiFi...");
		delay(1000);
	}

	Serial.println("Connected to WiFi");
}