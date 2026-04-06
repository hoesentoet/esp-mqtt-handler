/**
 * @file MqttDiscoverable.h
 * @brief Metadata struct that describes a Home Assistant MQTT discovery entity.
 *
 * Attach one of these to any MqttPubVariable or MqttSubVariable and pass
 * both to MqttHandler::addDiscoverable() to enable automatic HA discovery.
 */

#ifndef MQTT_DISCOVERABLE_H
#define MQTT_DISCOVERABLE_H

#include <Arduino.h>

#include "HomeAssistantUnits.h"

/**
 * @brief HA component types supported by MQTT discovery.
 *        Extend as needed — full list at https://www.home-assistant.io/integrations/mqtt/
 */
enum class HaComponent : uint8_t
{
    SENSOR,
    BINARY_SENSOR,
    SWITCH,
    NUMBER,
    SELECT,
    TEXT,
    BUTTON,
    LIGHT,
};

/**
 * @brief All configuration fields for a single HA entity.
 *        Only `component` and `entityId` are mandatory.
 *        Leave optional fields empty ("") to omit them from the payload.
 */
struct MqttDiscoverable
{
    // ── Required ──────────────────────────────────────────────────────────────

    HaComponent component; ///< HA component type (sensor, switch, …)
    String entityId;       ///< Unique slug within this device, e.g. "temperature"

    // ── Common optional fields ─────────────────────────────────────────────

    String name = "";                 ///< Friendly name shown in HA UI
    String deviceClass = "";          ///< e.g. "temperature", "humidity", "motion"
    String stateClass = "";           ///< "measurement" | "total" | "total_increasing"
    HaUnit unitOfMeas = HaUnit::NONE; ///< e.g. "°C", "%", "W"
    String icon = "";                 ///< e.g. "mdi:thermometer"
    String entityCategory = "";       ///< "config" | "diagnostic" | ""

    // ── Switch / Number / Select specific ─────────────────────────────────

    String payloadOn = "true";   ///< Payload that HA sends for ON  (switch)
    String payloadOff = "false"; ///< Payload that HA sends for OFF (switch)
    float numberMin = 0.0f;      ///< Minimum value (number)
    float numberMax = 100.0f;    ///< Maximum value (number)
    float numberStep = 1.0f;     ///< Step size    (number)
};

#endif // MQTT_DISCOVERABLE_H