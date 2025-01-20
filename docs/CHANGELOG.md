# ChangeLog for MqttHandler

## 1.0.6 - (2025-01-20)

### Bugfixes:
- Remove strict topic from PubVariable and remove callback parameter from SubVariable constructor because of bugs.

## 1.0.5 - (2025-01-20)

### Features
- Add topic strictness to pub and sub variable to be able to use the raw topic string specified instead of automatically adding the client name and in/out paths.

## 1.0.4 - (2025-01-19)

### Bugfixes:
- Fix excluding test files from importing into projects

## 1.0.3 - (2025-01-19)

### Features
- Add minimum publish interval option to MqttPubVariable. This offers the option to define that a PubVariable is only published every 5s for example.

## 1.0.2 - (2025-01-16)

### Bugfixes:
- Fix ESP8266 support by including ESP8266WiFi.h on purpose

## 1.0.1 - (2025-01-15)

### Bugfixes:
- Fix missing defines bug

## 1.0.0 (2025-01-15)
Initial Commit
