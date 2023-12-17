#ifndef MQTTCOMMUNICATION_H
#define MQTTCOMMUNICATION_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>

class MqttCommunication
{
	public:
		MqttCommunication(WiFiClient wifiClient, const char* mqttServerIP, const int mqttPort,
						  const char* mqttClientName, const char* mqttUserName, const char* mqttUserPassword);
		void Setup();
		void Reconnect();
		void PublishMessage(String topic, String message, bool serialPrint = false);
		void SubscribeCallback(char* topic, byte* payload, unsigned int length);
		void AddSubscribeTopic(const char* topic);
		void SubscribeToTopic(const char* topic);
		void Loop();

	private:
		WiFiClient wifiClient;
		PubSubClient client;

		const char *serverIP, *userName, *userPassword, *clientName;
		const int port;

		std::vector<const char*> subscribeTopics;
};

#endif