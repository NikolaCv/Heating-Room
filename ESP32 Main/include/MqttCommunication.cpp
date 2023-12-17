#include "MqttCommunication.h"

MqttCommunication::MqttCommunication(WiFiClient wifiClient, const char* mqttServerIP, const int mqttPort,
									const char* mqttClientName, const char* mqttUserName, const char* mqttUserPassword)
: wifiClient(wifiClient), serverIP(mqttServerIP), port(mqttPort),
  clientName(mqttClientName), userName(mqttUserName), userPassword(mqttUserPassword)
{
}

void MqttCommunication::Setup()
{
	client.setServer(serverIP, port);
	client.setKeepAlive(60);

	auto callbackWrapper = [this](char* topic, byte* payload, unsigned int length) {
        this->SubscribeCallback(topic, payload, length);
    };

    client.setCallback(callbackWrapper);
}

void MqttCommunication::Reconnect()
{
	while (!client.connected())
	{
		Serial.println("Attempting MQTT connection...");
		if (client.connect(clientName, userName, userPassword))
		{
			Serial.println("Connected to MQTT broker");
			
			for (const auto& topic : subscribeTopics)
	            SubscribeToTopic(topic);
		}
		else
		{
			Serial.print("Failed, rc=");
			Serial.print(client.state());
			Serial.println(" Retrying in 5 seconds...");
			delay(5000);
		}
	}
}

void MqttCommunication::PublishMessage(String topic, String message, bool serialPrint)
{
	if (client.connected())
	{
		client.publish(topic.c_str(), message.c_str());

		if(serialPrint) Serial.println(topic + "\t" + message);
	}
}

void MqttCommunication::SubscribeCallback(char* topic, byte* payload, unsigned int length)
{
	Serial.print("Message arrived on topic: ");
	Serial.println(topic);
}

void MqttCommunication::AddSubscribeTopic(const char *topic)
{
	subscribeTopics.push_back(topic);
}

void MqttCommunication::SubscribeToTopic(const char *topic)
{
	client.subscribe(topic);
}

void MqttCommunication::Loop()
{
	client.loop();
}
