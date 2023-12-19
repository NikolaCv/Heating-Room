#ifndef MQTTCOMMUNICATION_H
#define MQTTCOMMUNICATION_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>
#include "Sensors.h"
#include "ESPMainSerialCommunication.h"

class MqttCommunication
{
	public:
		MqttCommunication(WiFiClient wifiClient, const char* mqttServerIP, const int mqttPort,
						  const char* mqttClientName, const char* mqttUserName, const char* mqttUserPassword,
						  const int samplingRateSeconds, Sensors& sensors);
		void Setup(EspMainSerialCommunication serialComm);
		void Reconnect();

		void PublishMessage(String topic, String message, bool serialPrint = false);
		void SubscribeCallback(char* topic, byte* payload, unsigned int length);

		void AddSubscribeTopic(const char* topic);
		void SubscribeToTopic(const char* topic);

		void Loop();

		void SetupInterruptTimer();
		static void IRAM_ATTR InterruptTimerCallback();
		void PublistDataPeriodically(void (&PublishSensorDataFunction)(MqttCommunication mqtt, Sensors sensors));

	private:
		WiFiClient wifiClient;
		PubSubClient client;
		EspMainSerialCommunication serialComm;

		const char *serverIP, *userName, *userPassword, *clientName;
		const int port;

		std::vector<const char*> subscribeTopics;
		
		hw_timer_t* interruptTimer = nullptr;

		static int samplingRateSeconds;
		static bool publishData;

		Sensors sensors;
};

#endif