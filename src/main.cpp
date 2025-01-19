#include <Arduino.h>
#include "MqttHandler.h"

#define MQTT_TOPIC_PUBTEST_INT "pubTestInt"
#define MQTT_TOPIC_PUBTEST_STRING "pubTestString"

#define MQTT_TOPIC_SUBTEST_BOOL "subTestBool"
#define MQTT_TOPIC_SUBTEST_float "subTestFloat"

bool isWifiConnected = true;

MqttHandler mqttHandler(IPAddress(192, 168, 0, 12), "user", "password", "testClient01");

// MQTT Variable declaration
MqttPubVariable<int> pubTestInt(0, MQTT_TOPIC_PUBTEST_INT);
MqttPubVariable<String> pubTestString("Default String", MQTT_TOPIC_PUBTEST_STRING);

MqttSubVariable<bool> subTestBool(false, MQTT_TOPIC_SUBTEST_BOOL);
MqttSubVariable<float> subTestFloat(0.0, MQTT_TOPIC_SUBTEST_float);
//                │       │          │                └ MQTT-Topic
//                │       │          └ Initial value
//                │       └ Variable name
//                └ Variable data type

void setup() {
    Serial.begin(115200);

    Serial.println("Initializing MqttHandler...");
    mqttHandler.begin();

    Serial.println("Connecting to WiFi...");
    // Connect to WiFi!
    isWifiConnected = true;

    Serial.println("Setting up MQTT Sub-Variables onChange callbacks...");
    // MQTT Sub-Variables onChange callback setup:
    subTestBool.onChange([&](MqttSubVariable<bool> &var){
        Serial.printf("subTestBool (Topic '%s') has changed to %s", var.getTopic(), var.getValue() ? "True" : "False");
    });
    subTestFloat.onChange([&](MqttSubVariable<float> &var){
        Serial.printf("subTestFloat (Topic '%s') has changed to %f", var.getTopic(), var.getValue());
    });

    mqttHandler.addPubVariable(pubTestInt);
    mqttHandler.addPubVariable(pubTestString);
    mqttHandler.addSubVariable(subTestBool);
    mqttHandler.addSubVariable(subTestFloat);

    Serial.println("Setup completed!");
    Serial.println();
}

void loop() {
    mqttHandler.loop(isWifiConnected);
    sleep(0.1);
}