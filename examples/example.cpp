#include <Arduino.h>

#include "MqttHandler.h"

#define MQTT_TOPIC_PUBTEST_INT "pubTestInt"
#define MQTT_TOPIC_PUBTEST_STRING "pubTestString"

#define MQTT_TOPIC_SUBTEST_BOOL "subTestBool"
#define MQTT_TOPIC_SUBTEST_FLOAT "subTestFloat"

bool isWifiConnected;

MqttHandler mqttHandler(IPAddress(192, 168, 0, 12), "user", "password", "testClient01");

// ── MQTT Variable declaration ─────────────────────────────────────────────────
MqttPubVariable<int> pubTestInt(0, MQTT_TOPIC_PUBTEST_INT);
MqttPubVariable<String> pubTestString("Default String", MQTT_TOPIC_PUBTEST_STRING);

MqttSubVariable<bool> subTestBool(false, MQTT_TOPIC_SUBTEST_BOOL);
MqttSubVariable<float> subTestFloat(0.0, MQTT_TOPIC_SUBTEST_FLOAT);
//                │       │          │                └ MQTT-Topic
//                │       │          └ Initial value
//                │       └ Variable name
//                └ Variable data type

int counter = 0;

void setup() {
    Serial.begin(115200);

    // ── Device info for HA discovery ──────────────────────────────────────────
    // All discoverable entities will be grouped under this device in HA.
    mqttHandler.setDeviceInfo(
        /*deviceId=*/"testClient01",
        /*deviceName=*/"Test ESP Client",
        /*model=*/"NodeMCU v2",
        /*manufacturer=*/"DIY",
        /*swVersion=*/"1.0.0");

    Serial.println("Initializing MqttHandler...");
    mqttHandler.begin();

    Serial.println("Connecting to WiFi...");
    // Connect to WiFi!
    isWifiConnected = true;

    Serial.println("Setting up MQTT Sub-Variables onChange callbacks...");
    // MQTT Sub-Variables onChange callback setup:
    subTestBool.onChange([&](MqttSubVariable<bool>& var) {
        Serial.printf("subTestBool  (Topic '%s') changed to %s\n", var.getTopic().c_str(),
                      var.getValue() ? "True" : "False");
    });
    subTestFloat.onChange([&](MqttSubVariable<float>& var) {
        Serial.printf("subTestFloat (Topic '%s') changed to %f\n", var.getTopic().c_str(),
                      var.getValue());
    });

    // ── Register pub/sub variables ────────────────────────────────────────────
    mqttHandler.addPubVariable(pubTestInt);
    mqttHandler.addPubVariable(pubTestString);
    mqttHandler.addSubVariable(subTestBool);
    mqttHandler.addSubVariable(subTestFloat);

    // ── HA Discovery: pubTestInt → sensor ─────────────────────────────────────
    // A plain integer value without a specific device class — shown as a generic
    // numeric sensor. "state_class: measurement" enables the history graph in HA.
    mqttHandler.addDiscoverable(pubTestInt, {
                                                .component = HaComponent::SENSOR,
                                                .entityId = "pub_test_int",
                                                .name = "Test Integer",
                                                .stateClass = "measurement",
                                                .icon = "mdi:counter",
                                            });

    // ── HA Discovery: pubTestString → sensor ──────────────────────────────────
    // String sensors have no unit or device class; HA displays the raw value.
    mqttHandler.addDiscoverable(pubTestString, {
                                                   .component = HaComponent::SENSOR,
                                                   .entityId = "pub_test_string",
                                                   .name = "Test String",
                                                   .icon = "mdi:text",
                                               });

    // ── HA Discovery: subTestBool → binary_sensor ─────────────────────────────
    // The ESP only receives this value, so there is no command_topic.
    // HA will display it as ON/OFF; payloadOn/Off default to "true"/"false"
    // which matches the MqttHandler bool serialisation.
    mqttHandler.addDiscoverable(subTestBool, {
                                                 // uses the SubVariable overload
                                                 .component = HaComponent::BINARY_SENSOR,
                                                 .entityId = "sub_test_bool",
                                                 .name = "Test Bool",
                                                 .icon = "mdi:toggle-switch",
                                                 .payloadOn = "true",
                                                 .payloadOff = "false",
                                             });

    // ── HA Discovery: subTestFloat → number (bidirectional example) ───────────
    // Even though this sketch only reads subTestFloat, we register it as a
    // HA "number" entity so the HA UI can send new values to the ESP.
    // If you only want to display it read-only, use HaComponent::SENSOR instead
    // and call the single-variable overload (no pubVar needed).
    mqttHandler.addDiscoverable(subTestFloat, {
                                                  // uses the SubVariable overload
                                                  .component = HaComponent::SENSOR,
                                                  .entityId = "sub_test_float",
                                                  .name = "Test Float",
                                                  .stateClass = "measurement",
                                                  .icon = "mdi:decimal",
                                              });

    Serial.println("Setup completed!");
    Serial.println();
}

void loop() {
    if (counter > 9) {
        pubTestInt.setValue(pubTestInt.getValue() + 1);
        counter = 0;
    }

    mqttHandler.loop(isWifiConnected);
    counter++;
    delay(100);
}