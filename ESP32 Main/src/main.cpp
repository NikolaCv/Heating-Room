#include <Wire.h>
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
#include "../../Common/SerialJson.h"

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
preko wifi moze da mu se da novi config
dugme za reset na default (hard kodovan default)
dugme za SEND new config
relay switch

topics:
heating_room/relay/control
heating_room/relay/state

heating_room/config/reset
heating_room/config/update
heating_room/config

Prilikom reseta treba da se prikazu vrednosti u HA
tj. kada se uradi reset klapna salje svoje vrednosti, pa onda main salje sve na 

TODO
sub na mqtt topice
kad stigne config update i prosledi na flap observer

za relay kad stigne promeni stanje
state update i publish nakon toga

TODO
proveriti redom koji senzori su povezani a koji ne (bool json?)
ako npr. sht nije povezan ignorisi ga da ne bude errora

*/

void IRAM_ATTR PublishSensorData();
void SetupInterruptTimer();

void ReconnectWiFi();
void SetupMqttClient();
void ReconnectMqtt();
void PublishMessage(String topic, String message, bool serialPrint = false);

void ReadFromFlapObserver();
void SendToFlapObserver();

float GetCurrent(ACS712 sensor, int numMeasurements);
float GetDallasTemp(DallasTemperature DallasSensor, int numMeasurements);
/*float* GetSHT3xTempHumidity(int sensorPin, int numMeasurements);
float GetWeight(int sensorPin, int numMeasurements);*/

hw_timer_t* mqttTimer = NULL;

//Adafruit_SHT31 sht31 = Adafruit_SHT31();
//HX711 scale;

WiFiClient espClient;
PubSubClient mqttClient(espClient);

ACS712 currentSensor1(ACS712_30A, ACS712_1_PIN);
ACS712 currentSensor2(ACS712_30A, ACS712_2_PIN);

OneWire oneWire(DS18B20_PIN);
DallasTemperature DallasSensor(&oneWire);

int samplingRateSeconds = DefaultConfig::samplingRateSeconds;

int vibrationResetMqttSeconds = DefaultConfig::vibrationResetMqttSeconds;
int lastVibrationTime = 0;

int blockade = 0;

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

    SetupMqttClient();
	ReconnectMqtt();

    SetupInterruptTimer();

	currentSensor1.calibrateAC();
	currentSensor2.calibrateAC();

	DallasSensor.begin();

	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, LOW);
}

void loop()
{
	currTime = millis();

	ReconnectMqtt();
	ReconnectWiFi();
	
	mqttClient.loop();

	ReadFromFlapObserver();
}

void IRAM_ATTR PublishSensorData()
{
	PublishMessage("heating_room/burner", String(GetCurrent(currentSensor1, 20), 2));
	PublishMessage("heating_room/pump", String(GetCurrent(currentSensor2, 20), 2));

	PublishMessage("heating_room/furnace/temp", String(GetDallasTemp(DallasSensor, 3), 2));
}

void ReadFromFlapObserver()
{	
	if(currTime - lastVibrationTime > vibrationResetMqttSeconds * 1000)
		PublishMessage("heating_room/flap/vibration", String(0));
	
	while (Serial.available() > 0)
	{
		char incomingChar = Serial.read();
		Serial.println(incomingChar);

		switch (incomingChar)
		{
			case 'V': // Vibration
			{
				PublishMessage("heating_room/flap/vibration", String(1));
				lastVibrationTime = millis();

				if (blockade)
				{
					blockade = 0;
					PublishMessage("heating_room/flap/blockade", String(blockade));
				}
				break;
			}

			case 'X': // Blockade
			{
				blockade = 1;
				PublishMessage("heating_room/flap/blockade", String(blockade));
				break;
			}

			case 'S': // ESP Flap Observer SENT current config values, publish them
			{
				StaticJsonDocument<200> jsonDocument = SerialJson::ReadJson(Serial);
				if (SerialJson::ValidConfig(jsonDocument))
					// FIXME staviti u vrednosti koje treba i publish
				break;
			}

			// TODO: S set new values will be sent in mqtt callback, as well as R for reset, as well as G for get values

			default: // Numbers 0-6 for which IR diode is activated, ignore if not in that range
			{
				if (incomingChar >= '0' && incomingChar <= '6')
					PublishMessage("heating_room/flap/position", String(incomingChar), true);
				break;
			}
		}
	}
}

void SetupInterruptTimer()
{
	mqttTimer = timerBegin(0, 80, true);
	timerAttachInterrupt(mqttTimer, &PublishSensorData, true);
	timerAlarmWrite(mqttTimer, samplingRateSeconds * 1000000, true);
	timerAlarmEnable(mqttTimer);
}

void ReconnectWiFi() 
// FIXME
// ako ce i offline da skuplja podatke ne treba da se u infinite while vrti
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

void SetupMqttClient()
{
    mqttClient.setServer(Secrets::mqttServerIP, Secrets::mqttPort);
    mqttClient.setKeepAlive(60);
}

void ReconnectMqtt()
// FIXME
// ako ce i offline da skuplja podatke ne treba da se u infinite while vrti
{
	while (!mqttClient.connected())
	{
		Serial.println("Attempting MQTT connection...");
		if (mqttClient.connect("ESP32 Heating Room", Secrets::mqttUserName, Secrets::mqttUserPassword))
		{
			Serial.println("Connected to MQTT broker");
		}
		else
		{
			Serial.print("Failed, rc=");
			Serial.print(mqttClient.state());
			Serial.println(" Retrying in 5 seconds...");
			delay(5000);
		}
	}
}

void PublishMessage(String topic, String message, bool serialPrint)
{
	if (mqttClient.connected())
	{
		mqttClient.publish(topic.c_str(), message.c_str());

		if(serialPrint) Serial.println(topic + "\t" + message);
	}
}

float GetCurrent(ACS712 sensor, int numMeasurements)
{
	float I = 0;

	for (int i = 0; i < numMeasurements; ++i)
		I += sensor.getCurrentAC();

	return I / numMeasurements;
}

float GetDallasTemp(DallasTemperature DallasSensor, int numMeasurements)
{
	DallasSensor.requestTemperatures();
	float temp = 0;

	for (int i = 0; i < numMeasurements; ++i)
		temp += DallasSensor.getTempCByIndex(0);

	return temp / numMeasurements;
}

/*float* GetSHT3xTempHumidity(int sensorPin, int numMeasurements)
{
	
}

float GetWeight(int sensorPin, int numMeasurements)
{

}*/
		
