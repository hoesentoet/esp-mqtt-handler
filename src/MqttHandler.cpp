#include "MqttHandler.h"

WiFiClient __wifiClient;
PubSubClient __client(__wifiClient);

void MqttHandler::begin() {
    MqttHandler* instance = this;

    _client = __client;

    _client.setServer(_mqttServer, 1883);
    _client.setBufferSize(1024);

    auto cb_lambda = [instance](char* topic, byte* payload, unsigned int length) {
        instance->_subCallback(topic, payload, length);
    };

    _client.setCallback(cb_lambda);
}

void MqttHandler::loop(bool wifiConnected) {
    if (wifiConnected && !_client.connected()) _connect();

    if (_client.connected()) _client.loop();
}

void MqttHandler::_connect() {
    // Loop until we're reconnected
    while (!_client.connected()) {
        Serial.print(String("Attempting MQTT connection to ") + _mqttServer.toString() +
                     String("... "));
        // Attempt to connect
        if (_client.connect(_mqttClientName.c_str(), _mqttUser.c_str(), _mqttPassword.c_str())) {
            // Once connected, publish an announcement...
            Serial.println("Successfully (re)connected to MQTT");

            // Subscribe to all needed topics:
            for (const auto& pair : _subscriptions) {
                _sub(pair.first);
            }

            // Publish LWT "online" marker
            _client.publish((_mqttClientName + "/status").c_str(), "online", /*retain=*/true);

            // Re-publish all HA discovery configs
            for (const auto& publishFn : _discoverables) {
                publishFn();
            }
            Serial.println("Home Assistant Auto-Discovery package sent!");

            // Publish all unpublished things:
            Serial.println("Publishing everything piled up while disconnected...");
            for (const auto& pair : _toPublish) {
                _client.publish(pair.first.c_str(), pair.second.c_str());
            }
            _toPublish.clear();
            Serial.println("All done! MQTT Connection (re-)established and good to go!");
        } else {
            Serial.println(String("failed, rc=") + _client.state() +
                           String(" try again in 5 seconds"));
            // Wait 5 seconds before retrying
            delay(5000);
        }
        yield();
    }
}

void MqttHandler::_sub(String topicStr) {
    const char* topic = topicStr.c_str();
    if (*topic == '\0') return;

    Serial.print(String("Subscribing to '") + String(topic) + String("'..."));
    if (_client.connected()) {
        _client.subscribe(topic);
        Serial.println("\tOK");
    } else if (!_client.connected()) {
        Serial.println("\tFailed! Client not connected (yet)!");
    }
}

void MqttHandler::_subCallback(char* topic, uint8_t* payload, unsigned int length) {
    char payloadChar[length + 1];
    memcpy(payloadChar, payload, length);
    payloadChar[length] = '\0';  // Null-Terminate

    String topicStr = String(topic);
    if (_subscriptions.find(topicStr) != _subscriptions.end()) {
        _subscriptions[topicStr](String(payloadChar));
    }
}

void MqttHandler::setDeviceInfo(const String& deviceId, const String& deviceName,
                                const String& deviceModel, const String& deviceManufacturer,
                                const String& swVersion) {
    _deviceId = deviceId;
    _deviceName = deviceName;
    _deviceModel = deviceModel;
    _deviceManufacturer = deviceManufacturer;
    _swVersion = swVersion;
}

// ─────────────────────────────────────────────────────────────────────────────

String MqttHandler::_resolveTopic(const String& topic, bool strictTopic) const {
    return strictTopic ? topic : (_mqttClientName + "/out/" + topic);
}

// ─────────────────────────────────────────────────────────────────────────────

const char* MqttHandler::_componentStr(HaComponent c) {
    switch (c) {
        case HaComponent::SENSOR:
            return "sensor";
        case HaComponent::BINARY_SENSOR:
            return "binary_sensor";
        case HaComponent::SWITCH:
            return "switch";
        case HaComponent::NUMBER:
            return "number";
        case HaComponent::SELECT:
            return "select";
        case HaComponent::TEXT:
            return "text";
        case HaComponent::BUTTON:
            return "button";
        case HaComponent::LIGHT:
            return "light";
        default:
            return "sensor";
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void MqttHandler::_publishDiscoveryConfig(const String& stateTopic, const String& cmdTopic,
                                          const MqttDiscoverable& meta) {
    const char* component = _componentStr(meta.component);

    // homeassistant/<component>/<device_id>/<entity_id>/config
    String discoveryTopic =
        String("homeassistant/") + component + "/" + _deviceId + "/" + meta.entityId + "/config";

    JsonDocument doc;

    // ── Identity ──────────────────────────────────────────────────────────────
    doc["unique_id"] = _deviceId + "_" + meta.entityId;
    doc["name"] = meta.name.isEmpty() ? meta.entityId : meta.name;

    // ── Topics ────────────────────────────────────────────────────────────────
    doc["state_topic"] = stateTopic;
    if (!cmdTopic.isEmpty()) {
        doc["command_topic"] = cmdTopic;
    }

    // ── Availability (LWT) ────────────────────────────────────────────────────
    if (!_deviceId.isEmpty()) {
        doc["availability_topic"] = _mqttClientName + "/status";
    }

    // ── Optional metadata fields ──────────────────────────────────────────────
    if (!meta.deviceClass.isEmpty()) doc["device_class"] = meta.deviceClass;
    if (!meta.stateClass.isEmpty()) doc["state_class"] = meta.stateClass;
    if (meta.unitOfMeas != HaUnit::NONE)
        doc["unit_of_measurement"] = haUnitToString(meta.unitOfMeas);
    if (!meta.icon.isEmpty()) doc["icon"] = meta.icon;
    if (!meta.entityCategory.isEmpty()) doc["entity_category"] = meta.entityCategory;

    // ── Component-specific fields ─────────────────────────────────────────────
    switch (meta.component) {
        case HaComponent::SWITCH:
            doc["payload_on"] = meta.payloadOn;
            doc["payload_off"] = meta.payloadOff;
            doc["retain"] = true;
            break;
        case HaComponent::NUMBER:
            doc["min"] = meta.numberMin;
            doc["max"] = meta.numberMax;
            doc["step"] = meta.numberStep;
            break;
        case HaComponent::BINARY_SENSOR:
            doc["payload_on"] = meta.payloadOn;
            doc["payload_off"] = meta.payloadOff;
            break;
        default:
            break;
    }

    // ── Device block (groups entities in HA UI) ───────────────────────────────
    if (!_deviceId.isEmpty()) {
        JsonObject device = doc["device"].to<JsonObject>();
        device["identifiers"][0] = _deviceId;
        device["name"] = _deviceName;
        if (!_deviceModel.isEmpty()) device["model"] = _deviceModel;
        if (!_deviceManufacturer.isEmpty()) device["manufacturer"] = _deviceManufacturer;
        if (!_swVersion.isEmpty()) device["sw_version"] = _swVersion;
    }

    // ── Serialise & publish ───────────────────────────────────────────────────
    char payload[768];
    serializeJson(doc, payload, sizeof(payload));

    Serial.println(payload);
    Serial.printf("Sending discovery (client %s)",
                  _client.connected() ? "connected" : "not connected");
    bool ok = _client.publish(discoveryTopic.c_str(), payload, /*retain=*/true);

    if (_logging) {
        Serial.printf("[Discovery] publish -> %s : %s\n", discoveryTopic.c_str(),
                      ok ? "OK" : "FAILED (buffer too small?)");
    }
}

template <>
String MqttHandler::_fromString<String>(const String& value) const {
    return value;
}