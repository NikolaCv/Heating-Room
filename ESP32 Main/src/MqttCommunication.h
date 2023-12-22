#ifndef MQTT_COMMUNICATION_H
#define MQTT_COMMUNICATION_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <vector>
#include "SensorsMain.h"
#include "EspMainSerialCommunication.h"
#include "Configuration/DefaultConfig.h"

class MqttCommunication
{
	public:
		MqttCommunication() = default;
		MqttCommunication(PubSubClient& mqttClient, const char* mqttServerIP, const int mqttPort,
						  const char* mqttClientName, const char* mqttUserName, const char* mqttUserPassword,
						  const int samplingRateSeconds, SensorsMain& sensors, Stream& serial,
						  const char* relayControlTopic, const char* configResetTopic, const char* configUpdateTopic);

		void Setup(EspMainSerialCommunication& serialComm);
		void Reconnect();

		void PublishMessage(String topic, String message);
		void SubscribeCallback(char* topic, byte* payload, unsigned int length);

		void Loop();

		void SetupInterruptTimer();
		static void IRAM_ATTR InterruptTimerCallback();
		void PublishDataPeriodically(void (&PublishSensorDataFunction)(MqttCommunication& mqtt, SensorsMain& sensors), SensorsMain& sensors);

		int GetSamplingRateSeconds() const;

	private:
		PubSubClient client;
		EspMainSerialCommunication serialComm;
		Stream& serial;

		const char *serverIP, *userName, *userPassword, *clientName;
		const int port;

		StaticJsonDocument<200> subscribeTopics;
		
		hw_timer_t* interruptTimer = nullptr;

		static int samplingRateSeconds;
		static volatile bool publishData;

		SensorsMain sensors;
};

#endif