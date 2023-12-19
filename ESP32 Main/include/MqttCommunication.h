#ifndef MQTTCOMMUNICATION_H
#define MQTTCOMMUNICATION_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>
#include "Sensors.h"
#include "ESPMainSerialCommunication.h"
#include "../src/Configuration/DefaultConfig.h"

class MqttCommunication
{
	public:
		MqttCommunication(WiFiClient wifiClient, const char* mqttServerIP, const int mqttPort,
						  const char* mqttClientName, const char* mqttUserName, const char* mqttUserPassword,
						  const int samplingRateSeconds, Sensors& sensors, Stream& serial,
						  const char* relayControlTopic, const char* configResetTopic, const char* configUpdateTopic);
		void Setup(EspMainSerialCommunication serialComm);
		void Reconnect(Stream& output);

		void PublishMessage(String topic, String message);
		void SubscribeCallback(char* topic, byte* payload, unsigned int length);

		void Loop();

		void SetupInterruptTimer();
		static void IRAM_ATTR InterruptTimerCallback();
		void PublistDataPeriodically(void (&PublishSensorDataFunction)(MqttCommunication mqtt, Sensors sensors));

		int GetSamplingRateSeconds() const;

	private:
		WiFiClient wifiClient;
		PubSubClient client;
		EspMainSerialCommunication serialComm;
		Stream& serial;

		const char *serverIP, *userName, *userPassword, *clientName;
		const int port;

		StaticJsonDocument<200> subscribeTopics;
		
		hw_timer_t* interruptTimer = nullptr;

		static int samplingRateSeconds;
		static bool publishData;

		Sensors sensors;
};

#endif