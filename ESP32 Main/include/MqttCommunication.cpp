#include "MqttCommunication.h"

MqttCommunication::MqttCommunication(WiFiClient wifiClient, const char* mqttServerIP, const int mqttPort,
									const char* mqttClientName, const char* mqttUserName, const char* mqttUserPassword,
									const int samplingRateSeconds, Sensors& sensors)
: wifiClient(wifiClient), serverIP(mqttServerIP), port(mqttPort),
  clientName(mqttClientName), userName(mqttUserName), userPassword(mqttUserPassword),
  sensors(sensors)
{
	this->samplingRateSeconds = samplingRateSeconds;
	publishData = false;
}

void MqttCommunication::Setup(EspMainSerialCommunication serialComm)
{
	this->serialComm = serialComm;

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

void MqttCommunication::SetupInterruptTimer()
{
	interruptTimer = timerBegin(0, 80, true);
	timerAttachInterrupt(interruptTimer, &InterruptTimerCallback, true);
	timerAlarmWrite(interruptTimer, samplingRateSeconds * 1000000, true);
	timerAlarmEnable(interruptTimer);
}

void IRAM_ATTR MqttCommunication::InterruptTimerCallback()
{
	publishData = true;
}

void MqttCommunication::PublistDataPeriodically(void (&PublishSensorDataFunction)(MqttCommunication mqtt, Sensors sensors))
{
	if (publishData)
	{
		PublishSensorDataFunction(*this, sensors);
		publishData = false;
	}	
}
