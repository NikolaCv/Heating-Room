#include <Arduino.h>
#include <ArduinoJson.h>

#include "Configuration/DefaultConfig.h"
#include "../../Common/SerialJson.h"

#define VIBRATION_PIN 4

void IRAM_ATTR InterruptIR();
void IRAM_ATTR InterruptVibration();
void SendTriggeredIR();
void SendTriggeredVibration();
void ReadFromMainESP32();
void SendConfigValues();
void SetConfigValues(int newDebounceMillisIR, int newDebounceMillisVibration, int newBlockadeThreshold);

const int irSensorPins[] = {25, 33, 32, 35, 34, 39, 26};
const int numSensorsIR = 7;
volatile int triggeredIR = -1;
int previousIR = -1;
int debounceMillisIR = DefaultConfig::debounceMillisIR;
unsigned long lastTimeIR = millis();

volatile bool triggeredVibration = false;
int debounceMillisVibration = DefaultConfig::debounceMillisVibration;
unsigned long lastTimeVibration = millis();

int vibrationFlapDiff = 0;
int blockadeThreshold = DefaultConfig::blockadeThreshold;

unsigned long currTime = millis();

void setup()
{
	Serial.begin(9600);
	for (int i = 0; i < numSensorsIR; i++)
	{
		pinMode(irSensorPins[i], INPUT);
		attachInterrupt(digitalPinToInterrupt(irSensorPins[i]), InterruptIR, RISING);
	}

	pinMode(VIBRATION_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(VIBRATION_PIN), InterruptVibration, CHANGE);
}

void loop()
{
	currTime = millis();

	ReadFromMainESP32();

	SendTriggeredIR();
	SendTriggeredVibration();

	//delay(10);
}

void ReadFromMainESP32()
{	
	while (Serial.available() > 0)
	{
		char incomingChar = Serial.read();
		Serial.println(incomingChar);

		switch (incomingChar)
		{
			case 'R': // Reset config to default
			{
				SetConfigValues(DefaultConfig::debounceMillisIR, DefaultConfig::debounceMillisVibration, DefaultConfig::blockadeThreshold);
				SendConfigValues('S');
				break;
			}

			case 'G': // Get config (ESP Main is requesting values, send them)
			{
				SendConfigValues('S');
				break;
			}

			case 'S': // Set config (ESP Main SENT new values, update them)
			{
				StaticJsonDocument<200> jsonDocument = SerialJson::ReadJson(Serial);
				if (SerialJson::ValidConfig(jsonDocument))
					SetConfigValues(jsonDocument["debounceMillisIR"], jsonDocument["debounceMillisVibration"], jsonDocument["blockadeThreshold"]);
				SendConfigValues('S');
				break;
			}
		}
	}
}

void SendConfigValues(char code)
{
	Serial.print(code);
	SerialJson::SendJson(debounceMillisIR, debounceMillisVibration, blockadeThreshold);
}

void SetConfigValues(int newDebounceMillisIR, int newDebounceMillisVibration, int newBlockadeThreshold)
{
	debounceMillisIR = newDebounceMillisIR;
	debounceMillisVibration = newDebounceMillisVibration;
	blockadeThreshold = newBlockadeThreshold;
}

void SendTriggeredIR()
{
	if (currTime - lastTimeIR > debounceMillisIR && previousIR != triggeredIR)
	{
		Serial.print(triggeredIR);

		vibrationFlapDiff = 0;
		lastTimeIR = currTime;
		previousIR = triggeredIR;
	}
}

void SendTriggeredVibration()
{
	if (currTime - lastTimeVibration > debounceMillisVibration && triggeredVibration)
	{
		if (vibrationFlapDiff > blockadeThreshold)
			Serial.print('X'); // Blockade
		else
			Serial.print('V'); // Vibration

		vibrationFlapDiff++;
		lastTimeVibration = currTime;
		triggeredVibration = false;
	}
}

void IRAM_ATTR InterruptIR()
{
	for (int i = 0; i < numSensorsIR; i++)
	{
		if (digitalRead(irSensorPins[i]))
		{
			triggeredIR = i;
			break;
		}
	}
}

void IRAM_ATTR InterruptVibration()
{
	triggeredVibration = true;
}
