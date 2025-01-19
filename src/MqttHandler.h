#ifndef MQTTHANDLER_H
#define MQTTHANDLER_H

#include <Arduino.h>
#include <PubSubClient.h>
#include <functional>
#include <unordered_map>

#ifdef ESP32
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#else
#error "Unsupported platform"
#endif

struct StringHash {
    std::size_t operator()(const String& s) const {
        std::size_t hash = 0;
        for (uint i = 0; i < s.length(); i++) {
            hash = hash * 31 + s[i];
        }
        return hash;
    }
};


template <typename T>
class MqttSubVariable
{
    public:
        MqttSubVariable(T value, String topic, std::function<void(MqttSubVariable&)> onChangeCb = nullptr)
            : _value(value), _topic(topic), _onChange(onChangeCb) {}
        
        MqttSubVariable(T value, const char *topic, std::function<void(MqttSubVariable&)> onChangeCb = nullptr)
            : _value(value), _topic(String(topic)), _onChange(onChangeCb) {}

        T getValue(void)
        {
            return _value;
        }

        String getTopic(void)
        {
            return _topic;
        }
        
        void onChange(std::function<void(MqttSubVariable&)> cb)
        {
            _onChange = cb;
        }

        friend class MqttHandler;


    private:
        T _value;
        String _topic;
        std::function<void(MqttSubVariable&)> _onChange;

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
            : _value(value), _topic(topic), _minPubIntervalMs(minPubIntervalMs)  {}

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
                    } else {
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

        friend class MqttHandler;
        
    private:
        T _value;
        String _topic;
        ulong _minPubIntervalMs;
        std::function<void(MqttPubVariable&)> _onChangeCb;
        bool _initPubDone = false;
        bool _pubInQueue = false;
        ulong _lastPubTime = 0;
        
        void _onChange(std::function<void(MqttPubVariable&)> cb)
        {
            _onChangeCb = cb;
        }
};

class MqttHandler
{
    public:
        MqttHandler(IPAddress brokerIP, String username, String password, String clientName) {
            _mqttServer = brokerIP;
            _mqttUser = username;
            _mqttPassword = password;
            _mqttClientName = clientName;
        }

        template<typename T>
        void addSubVariable(MqttSubVariable<T> &variable)
        {
            const String topic = variable.getTopic();
            this->_subscribe<T>(topic, [&variable, this, topic](T newValue) {
                variable._setValue(newValue);
                Serial.println(String("Updated variable '") + variable.getTopic() + String("' to ") + String(variable.getValue()));
            });
        }

        template<typename T>
        void addPubVariable(MqttPubVariable<T> &variable)
        {
            variable._onChange([this](MqttPubVariable<T>& var) {
                this->_publish(var.getTopic(), var.getValue());
            });
        }

        void loop(bool wifiConnected);
        void begin(void);

    private:
        String _mqttUser, _mqttPassword, _mqttClientName;

        IPAddress _mqttServer;
        WiFiClient _wifiClient;
        PubSubClient _client;

        std::unordered_map<String, std::function<void(const String&)>, StringHash> _subscriptions;
        std::unordered_map<String, String, StringHash> _toPublish;

        template <typename T>
        void _subscribe(const String &topic, std::function<void(T)> callback)
        {
            String topicStr = String(_mqttClientName) + String("/in/") + topic;
            _subscriptions[topicStr] = [callback, this](const String& value)
            {
                T typedValue = this->_fromString<T>(value);
                callback(typedValue);
            };
            _sub(topicStr);
        }

        template <typename T>
        void _publish(const String &topic, const T& value)
        {
            String valueStr = _toString(value);

            String pubTopic = String(_mqttClientName) + String("/out/") + topic;
            if (_client.connected())
            {
                _client.publish(pubTopic.c_str(), valueStr.c_str());
            }
            _toPublish[pubTopic] = valueStr;
        }

        void _sub(String topic);
        void _subCallback(char *topic, uint8_t *payload, unsigned int length);
        void _connect(void);

        template<typename T>
        T _fromString(const String& value) const {
            if constexpr (std::is_same_v<T, int>) {
                return value.toInt();
            } else if constexpr (std::is_same_v<T, unsigned int>) {
                return static_cast<unsigned int>(value.toInt());
            } else if constexpr (std::is_same_v<T, long>) {
                return value.toInt();
            } else if constexpr (std::is_same_v<T, unsigned long>) {
                return static_cast<unsigned long>(value.toInt());
            } else if constexpr (std::is_same_v<T, float>) {
                return value.toFloat();
            } else if constexpr (std::is_same_v<T, double>) {
                return value.toFloat();
            } else if constexpr (std::is_same_v<T, bool>) {
                return value.equalsIgnoreCase("true") || value.toInt() != 0;
            } else {
                return static_cast<T>(value);
            }
        }

        template<typename T>
        String _toString(const T& value) const {
            if constexpr (std::is_same_v<T, String>) {
                return value;
            } else if constexpr (std::is_arithmetic_v<T>) {
                return String(value);
            } else {
                return String(value);
            }
        }
};

#endif // MQTTHANDLER_H