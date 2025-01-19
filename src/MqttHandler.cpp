#include "MqttHandler.h"

WiFiClient __wifiClient;
PubSubClient __client(__wifiClient);

void MqttHandler::begin()
{
    MqttHandler* instance = this;

    _client = __client;

    _client.setServer(_mqttServer, 1883);

    auto cb_lambda = [instance] (char *topic, byte *payload, unsigned int length) {
        instance->_subCallback(topic, payload, length);
    };

    _client.setCallback(cb_lambda);
}

void MqttHandler::loop(bool wifiConnected)
{
    if (wifiConnected && !_client.connected())
        _connect();

    if (_client.connected())
        _client.loop();
}

void MqttHandler::_connect()
{
    // Loop until we're reconnected
    while (!_client.connected())
    {
        Serial.print(String("Attempting MQTT connection to ") + _mqttServer.toString() + String("... "));
        // Attempt to connect
        if (_client.connect(_mqttClientName.c_str(), _mqttUser.c_str(), _mqttPassword.c_str()))
        {
            // Once connected, publish an announcement...
            Serial.println("Successfully (re)connected to MQTT");

            // Subscribe to all needed topics:
            for (const auto& pair : _subscriptions) {
                _sub(pair.first);
            }

            // Publish all unpublished things:
            for (const auto& pair : _toPublish) {
                _client.publish(pair.first.c_str(), pair.second.c_str());
            }
        }
        else
        {
            Serial.println(String("failed, rc=") + _client.state() + String(" try again in 5 seconds"));
            // Wait 5 seconds before retrying
            delay(5000);
        }
        yield();
    }
}

void MqttHandler::_sub(String topicStr)
{
    const char * topic = topicStr.c_str();
    if (*topic == '\0')
        return;

    Serial.print(String("Subscribing to '") + String(topic) + String("'..."));
    if (_client.connected())
    {
        _client.subscribe(topic);
        Serial.println("\tOK");
    }
    else if (!_client.connected())
    {
        Serial.println("\tFailed! Client not connected (yet)!");
    }
}

void MqttHandler::_subCallback(char *topic, uint8_t *payload, unsigned int length)
{
    char payloadChar[length + 1];
    memcpy(payloadChar, payload, length);
    payloadChar[length] = '\0'; // Null-Terminate

    String topicStr = String(topic);
    if (_subscriptions.find(topicStr) != _subscriptions.end())
    {
        _subscriptions[topicStr](String(payloadChar));
    }
}

template<>
String MqttHandler::_fromString<String>(const String& value) const {
    return value;
}