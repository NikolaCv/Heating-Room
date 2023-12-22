#include "MqttCommunication.h"

int MqttCommunication::samplingRateSeconds = 0;
volatile bool MqttCommunication::publishData = true;

MqttCommunication::MqttCommunication(PubSubClient& mqttClient, const char* mqttServerIP, const int mqttPort,
									const char* mqttClientName, const char* mqttUserName, const char* mqttUserPassword,
									const int samplingRateSeconds, SensorsMain& sensors, Stream& serial,
						  			const char* relayControlTopic, const char* configResetTopic, const char* configUpdateTopic)
: client(mqttClient), serverIP(mqttServerIP), port(mqttPort),
  clientName(mqttClientName), userName(mqttUserName), userPassword(mqttUserPassword),
  sensors(sensors), serial(serial)
{
	this->samplingRateSeconds = samplingRateSeconds;
	publishData = false;
	
	subscribeTopics["Relay Control"] = relayControlTopic;
	subscribeTopics["Config Reset"] = configResetTopic;
	subscribeTopics["Config Update"] = configUpdateTopic;
}

void MqttCommunication::Setup(EspMainSerialCommunication& serialComm)
{
	this->serialComm = serialComm;

	client.setServer(serverIP, port);
	client.setKeepAlive(60);
	auto callbackWrapper = [this](char* topic, byte* payload, unsigned int length) {
		this->SubscribeCallback(topic, payload, length);
	};

	client.setCallback(callbackWrapper);

	Reconnect();
	SetupInterruptTimer();
}

void MqttCommunication::Reconnect()
{
	while (!client.connected())
	{
		serial.println("Attempting MQTT connection...");
		if (client.connect(clientName, userName, userPassword))
		{
			serial.println("Connected to MQTT broker");
			
			for (const auto& topic : subscribeTopics.as<JsonObject>())
				client.subscribe(topic.value().as<const char*>());
		}
		else
		{
			serial.print("Failed, rc=");
			serial.print(client.state());
			serial.println(" Retrying in 5 seconds...");
			delay(5000);
		}
	}
}

void MqttCommunication::PublishMessage(String topic, String message)
{
	if (client.connected())
		client.publish(topic.c_str(), message.c_str());
}

void MqttCommunication::SubscribeCallback(char* topic, byte* payload, unsigned int length)
{
	String message;
	for (unsigned int i = 0; i < length; i++)
		message += (char)payload[i];

	if (strcmp(topic, subscribeTopics["Relay Control"]) == 0)
	{
		// TODO
	}
	else if (strcmp(topic, subscribeTopics["Config Reset"]) == 0)
	{
		samplingRateSeconds = DefaultConfig::samplingRateSeconds;
		serialComm.SendToSerial(serial, 'R');
		serialComm.ResetVibrationConfigValue();
		
		timerAlarmWrite(interruptTimer, samplingRateSeconds * 1000000, true);
	}
	else if (strcmp(topic, subscribeTopics["Config Update"]) == 0)
	{
		StaticJsonDocument<200> jsonDocument;
		deserializeJson(jsonDocument, message);

		samplingRateSeconds = jsonDocument["samplingRateSeconds"];
		serialComm.SetVibrationConfigValue(jsonDocument["vibrationResetMqttSeconds"]);

		timerAlarmWrite(interruptTimer, samplingRateSeconds * 1000000, true);

		jsonDocument.remove("samplingRateSeconds");
		jsonDocument.remove("vibrationResetMqttSeconds");

		serialComm.SendJsonToSerial(serial, 'S', jsonDocument);		
	}
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

void MqttCommunication::PublishDataPeriodically(void (&PublishSensorDataFunction)(MqttCommunication& mqtt, SensorsMain& sensors), SensorsMain& sensors)
{	
	if (publishData)
	{
		PublishSensorDataFunction(*this, sensors);
		publishData = false;
	}
}

int MqttCommunication::GetSamplingRateSeconds() const
{
    return samplingRateSeconds;
}
