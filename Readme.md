# Paulman BLE2MQTT

Bridges a Paulman BLE controller to a mqtt stream.
The following attributes are currently supported:

* State [bool] - On/Off
* Brighness [uint8_t] 1 - 100
* Color Temperature in Kelvin [uint16_t] 2700 - 6500

NOTE: The boundaries are not validated/checked. 

## Configuration
Configure your WiFi and MQTT settings via the ```config.h``` file.
Consolidate the example file for a list of required define identifiers.


## MQTT Topics
The topic composes of ```MQTT_BASE_TOPIC``` /  ```DEVICE_NAME```.
Based on this _base topic_, the following topics are used to report the light attributes:

* ```availability``` - online/offline - if the BLE device is reachable
* ```state``` - on/off - if the BLE device is on or off
* ```brightness``` - 1-100 - birghtness value of the light
* ```temperature``` - 2700 - 6500 color temperatue in kelvin

Furhtermore, based on this _base topic_ the device listens on the following topic: ```MQTT_BASE_TOPIC``` /  ```DEVICE_NAME``` / ```set``` / ```#```.
Obviously, this topic is used to configure the light.

## Home Assistant
The following MQTT message can be used to register this device as an [MQTT Light](https://www.home-assistant.io/integrations/light.mqtt/#default-schema):

```jsonc
// Send to topic:
// homeassistant/light/0x0123456789ab/light/config
{
    "~": "ble2mqtt/hallway",
    "name": "Hallway",
    "unique_id": "0123456789ab",
    "availability_topic": "~/availability",
    "state_topic": "~/state",
    "payload_on": "on",
    "payload_off": "off",
    "command_topic": "~/set/state",
    "brightness_state_topic": "~/brightness",
    "brightness_command_topic": "~/set/brightness",
    "brightness_scale": 100,
    "color_temp_state_topic": "~/temperature",
    "color_temp_value_template": "{{ (1000000 / value | float) | round(0) }}",
    "color_temp_command_topic": "~/set/temperature",
    "color_temp_command_template": "{{ (1000000 / value) | round(0) }}",
    "min_mireds": 154,
    "max_mireds": 370
}
```

NOTE: Home-Assistant uses mireds for color temperature.
This json defines templates to map between mireds and kelvin.
Min/Max mireds represent 2700K to 6500K, matching my light.

## TODO
It seems the device does not reconnect to the WiFi if it lost the connection.

Furthermore, the status update is sent hard-coded to be sent every 10 seconds. 
In this context, it is also worth to point out, that the ESP esentially does not sleep. 
I should proably investigate, if it is worthwile and if the TaskScheduler library uses interrupts and can also schedule/handle the mqtt loop.

## Credits
Thanks to Aaron for his blogpost on [Atcnetz](http://atcnetz.blogspot.com/2018/06/mit-dem-raspberry-pi-oder-dem-esp32-die.html).
He did the challanging task of reverse-engineering the Paulmann BLE API and provided some usefull information to get started.

