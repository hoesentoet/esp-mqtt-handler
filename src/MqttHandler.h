#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <functional>
#include <unordered_map>
#include <ArduinoJson.h>

#include "MqttDiscoverable.h"
#include "HomeAssistantUnits.h"

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "Unsupported platform"
#endif

struct StringHash
{
    std::size_t operator()(const String &s) const
    {
        std::size_t hash = 0;
        for (uint i = 0; i < s.length(); i++)
        {
            hash = hash * 31 + s[i];
        }
        return hash;
    }
};

template <typename T>
class MqttSubVariable
{
public:
    MqttSubVariable(T value, String topic, bool strictTopic = false)
        : _value(value), _topic(topic), _strictTopic(strictTopic), _onChange(nullptr) {}

    MqttSubVariable(T value, const char *topic, bool strictTopic = false)
        : _value(value), _topic(String(topic)), _strictTopic(strictTopic), _onChange(nullptr) {}

    T getValue(void)
    {
        return _value;
    }

    String getTopic(void)
    {
        return _topic;
    }
    bool isTopicStrict(void)
    {
        return _strictTopic;
    }

    void onChange(std::function<void(MqttSubVariable &)> cb)
    {
        _onChange = cb;
    }

    friend class MqttHandler;

private:
    T _value;
    String _topic;
    bool _strictTopic;
    std::function<void(MqttSubVariable &)> _onChange;

    void _setValue(T value)
    {
        _value = value;

        if (_onChange)
        {
            _onChange(*this);
        }
    }
};

template <typename T>
class MqttPubVariable
{
public:
    MqttPubVariable(T value, String topic, ulong minPubIntervalMs = 0)
        : _value(value), _topic(topic), _minPubIntervalMs(minPubIntervalMs) {}

    MqttPubVariable(T value, const char *topic, ulong minPubIntervalMs = 0)
        : _value(value), _topic(String(topic)), _minPubIntervalMs(minPubIntervalMs) {}

    void setValue(T value)
    {
        if (_value != value || !_initPubDone || _pubInQueue)
        {
            _value = value;
            if (_onChangeCb)
            {
                if (_minPubIntervalMs == 0 || millis() - _lastPubTime > _minPubIntervalMs)
                {
                    _onChangeCb(*this);
                    _lastPubTime = millis();
                    _pubInQueue = false;
                }
                else
                {
                    _pubInQueue = true;
                }
            }
            _initPubDone = true;
        }
        else
        {
            _value = value;
        }
    }

    T getValue(void)
    {
        return _value;
    }

    String getTopic(void)
    {
        return _topic;
    }
    bool isTopicStrict(void)
    {
        return _strictTopic;
    }

    friend class MqttHandler;

private:
    T _value;
    String _topic;
    bool _strictTopic;
    ulong _minPubIntervalMs;
    std::function<void(MqttPubVariable &)> _onChangeCb;
    bool _initPubDone = false;
    bool _pubInQueue = false;
    ulong _lastPubTime = 0;

    void _onChange(std::function<void(MqttPubVariable &)> cb)
    {
        _onChangeCb = cb;
    }
};

class MqttHandler
{
public:
    MqttHandler(IPAddress brokerIP, String username, String password, String clientName, bool logging = false)
    {
        _mqttServer = brokerIP;
        _mqttUser = username;
        _mqttPassword = password;
        _mqttClientName = clientName;
        _logging = logging;
    }

    template <typename T>
    void addSubVariable(MqttSubVariable<T> &variable)
    {
        const String topic = variable.getTopic();
        bool strictTopic = variable.isTopicStrict();
        this->_subscribe<T>(topic, strictTopic, [&variable, this, topic](T newValue)
                            {
                variable._setValue(newValue);
                if (this->_logging) {
                    Serial.println(String("Updated variable '") + variable.getTopic() + String("' to ") + String(variable.getValue()));
                } });
    }

    template <typename T>
    void addPubVariable(MqttPubVariable<T> &variable)
    {
        variable._onChange([this](MqttPubVariable<T> &var)
                           { this->_publish(var.getTopic(), var.isTopicStrict(), var.getValue()); });
    }

    /**
     * @brief Set the shared device metadata block that is embedded in every
     *        discovery config payload (links entities together in the HA UI).
     *
     * @param deviceId           Unique hardware identifier, e.g. chip ID
     * @param deviceName         Human-readable name shown in HA
     * @param deviceModel        Optional model string
     * @param deviceManufacturer Optional manufacturer string
     * @param swVersion          Optional firmware version string
     */
    void setDeviceInfo(const String &deviceId,
                       const String &deviceName,
                       const String &deviceModel = "",
                       const String &deviceManufacturer = "",
                       const String &swVersion = "");

    /// Overload for read-only Sub-Variables (no state_topic, only command_topic).
    template <typename T>
    void addDiscoverable(MqttSubVariable<T> &subVar, const MqttDiscoverable &meta)
    {
        // For a pure subscriber the "state" visible in HA is what the ESP receives,
        // so we expose the sub-topic as state_topic (HA reads back what it sent).
        String topic = _resolveTopic(subVar.getTopic(), subVar.isTopicStrict());

        _discoverables.push_back([this, topic, meta]()
                                 { _publishDiscoveryConfig(topic, /*cmdTopic=*/"", meta); });
    }

    /**
     * @brief Register a PubVariable for HA discovery.
     *        The variable's topic becomes the state_topic in the config payload.
     *        Call this before begin().
     */
    template <typename T>
    void addDiscoverable(MqttPubVariable<T> &variable, const MqttDiscoverable &meta)
    {
        // Resolve the actual topic the same way _publish() does
        String stateTopic = meta.component == HaComponent::SWITCH
                                ? _resolveTopic(variable.getTopic(), variable.isTopicStrict())
                                : _resolveTopic(variable.getTopic(), variable.isTopicStrict());

        _discoverables.push_back([this, stateTopic, meta]()
                                 { _publishDiscoveryConfig(stateTopic, /*cmdTopic=*/"", meta); });
    }

    /**
     * @brief Register a paired Pub+Sub variable for bidirectional HA entities
     *        (e.g. switch, number, select) that need both a state_topic and a
     *        command_topic.
     *
     * @param pubVar  Variable the ESP publishes state to  → state_topic
     * @param subVar  Variable the ESP receives commands on → command_topic
     * @param meta    HA entity metadata
     */
    template <typename TPub, typename TSub>
    void addDiscoverable(MqttPubVariable<TPub> &pubVar,
                         MqttSubVariable<TSub> &subVar,
                         const MqttDiscoverable &meta)
    {
        String stateTopic = _resolveTopic(pubVar.getTopic(), pubVar.isTopicStrict());
        String cmdTopic = _resolveTopic(subVar.getTopic(), subVar.isTopicStrict());

        _discoverables.push_back([this, stateTopic, cmdTopic, meta]()
                                 { _publishDiscoveryConfig(stateTopic, cmdTopic, meta); });
    }

    void loop(bool wifiConnected);
    void begin(void);

private:
    String _mqttUser, _mqttPassword, _mqttClientName;
    bool _logging;

    IPAddress _mqttServer;
    WiFiClient _wifiClient;
    PubSubClient _client;

    std::unordered_map<String, std::function<void(const String &)>, StringHash> _subscriptions;
    std::unordered_map<String, String, StringHash> _toPublish;

    template <typename T>
    void _subscribe(const String &topic, bool strictTopic, std::function<void(T)> callback)
    {
        String topicStr = strictTopic ? topic : String(_mqttClientName) + String("/in/") + topic;
        _subscriptions[topicStr] = [callback, this](const String &value)
        {
            T typedValue = this->_fromString<T>(value);
            callback(typedValue);
        };
        _sub(topicStr);
    }

    template <typename T>
    void _publish(const String &topic, bool strictTopic, const T &value)
    {
        String valueStr = _toString(value);

        String pubTopic = strictTopic ? topic : String(_mqttClientName) + String("/out/") + topic;
        if (_client.connected())
        {
            _client.publish(pubTopic.c_str(), valueStr.c_str());
        }
        _toPublish[pubTopic] = valueStr;
    }

    void _sub(String topic);
    void _subCallback(char *topic, uint8_t *payload, unsigned int length);
    void _connect(void);

    // ── Discovery state ───────────────────────────────────────────────────────

    String _deviceId, _deviceName, _deviceModel, _deviceManufacturer, _swVersion;
    std::vector<std::function<void()>> _discoverables; ///< Lambdas, each publishes one config

    /** @brief Resolve a topic to its full MQTT path (prefix + in/out or strict). */
    String _resolveTopic(const String &topic, bool strictTopic) const;

    /** @brief Build and publish a single HA discovery config message. */
    void _publishDiscoveryConfig(const String &stateTopic,
                                 const String &cmdTopic,
                                 const MqttDiscoverable &meta);

    /** @brief Map HaComponent enum to its HA string representation. */
    static const char *_componentStr(HaComponent c);

    // ──────────────────────────────────────────────────────────────────────────

    template <typename T>
    T _fromString(const String &value) const
    {
        if constexpr (std::is_same_v<T, int>)
        {
            return value.toInt();
        }
        else if constexpr (std::is_same_v<T, unsigned int>)
        {
            return static_cast<unsigned int>(value.toInt());
        }
        else if constexpr (std::is_same_v<T, long>)
        {
            return value.toInt();
        }
        else if constexpr (std::is_same_v<T, unsigned long>)
        {
            return static_cast<unsigned long>(value.toInt());
        }
        else if constexpr (std::is_same_v<T, float>)
        {
            return value.toFloat();
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            return value.toFloat();
        }
        else if constexpr (std::is_same_v<T, bool>)
        {
            return value.equalsIgnoreCase("true") || value.toInt() != 0;
        }
        else
        {
            return static_cast<T>(value);
        }
    }

    template <typename T>
    String _toString(const T &value) const
    {
        if constexpr (std::is_same_v<T, String>)
        {
            return value;
        }
        else if constexpr (std::is_arithmetic_v<T>)
        {
            return String(value);
        }
        else
        {
            return String(value);
        }
    }
};

#endif // MQTTHANDLER_H