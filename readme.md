# ESP-MQTT-Handler
MQTT wrapper library that adds MQTT variables for a very easy and clean use of the PubSubClient library.

## Usage
Add one of the following lines to your `platformio.ini`

```ini
lib_deps =
  # RECOMMENDED
  # Accept new functionality in a backwards compatible manner and patches
  https://github.com/hoesentoet/esp-mqtt-handler.git @ ^1.0.7

  # Accept only backwards compatible bug fixes
  # (any version with the same major and minor versions, and an equal or greater patch version)
  https://github.com/hoesentoet/esp-mqtt-handler.git @ ~1.0.7

  # The exact version
  https://github.com/hoesentoet/esp-mqtt-handler.git @ 1.0.7
```

## Maintaining this library
1. Clone this repo
2. Make your edits
    - Add needed library dependencies in the `library.json` file
    - For testing: Copy the following example `platformio.ini` file in the root directory:

        ```ini
        [env:az-delivery-devkit-v4]
        platform = espressif32
        board = az-delivery-devkit-v4
        framework = arduino
        build_unflags = -std=gnu++11
        build_flags = -std=gnu++17

        lib_deps = knolleary/PubSubClient @ ^2.8

        monitor_speed = 115200
        monitor_filters = esp8266_exception_decoder, default
        ```

3. Commit everything (consider splitting it in multiple commits)
4. Modify the version in the `library.json` and in this `readme.md` file
5. Add changelog to the `docs/CHANGELOG.md` and commit everything
6. Add release in GitHub with tag name e.g. `v1.0.0` and release name `1.0.0`
