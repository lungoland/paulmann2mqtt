#define __DEBUG

#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>

// #define _TASK_SLEEP_ON_IDLE_RUN
#define _TASK_STD_FUNCTION
#include <TaskScheduler.h>

#include "Paulmann.h"
#include "Controller.h"
#include "config.h"


WiFiClient net;
MQTTClient mqtt;
Paulmann device;

Scheduler scheduler;

// Does not work, as BLE throws an error in fast calls :/
// Controller ctrl(device, scheduler);


void log(const char* fmt, ...)
{
#if defined(__DEBUG)
  char buffer[64];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  Serial.println(buffer);
#endif
}


template<typename T>
void sendMqttMessage(const std::string& topic, T payload)
{
  mqtt.publish(topic.c_str(), String(payload));
}

Task mqttLoop(10 * TASK_MILLISECOND, TASK_FOREVER, []()
{
  if (!mqtt.connected())
  {
    log("Trying to connect to " MQTT_BROKER);
    if(mqtt.connect(DEVICE_NAME, MQTT_USER, MQTT_PASSWORD))
    {
      log("Subscribing to " MQTT_BASE_TOPIC "/" DEVICE_NAME);
      mqtt.subscribe(MQTT_BASE_TOPIC "/" DEVICE_NAME "/set/#");
    }
    else
    {
      mqttLoop.delay(500);
    }
  }
  mqtt.loop();
}, &scheduler, true);

Task sendStateUpdate(10 * TASK_SECOND, TASK_FOREVER, []()
{
#if defined(__DEBUG)
  log("State update");
#endif

  if (!mqtt.connected())
    return;

  std::string baseTopic = MQTT_BASE_TOPIC "/" DEVICE_NAME "/";
  if (device.connected())
  {
    sendMqttMessage(baseTopic + "availability", "online");
    sendMqttMessage(baseTopic + "state", device.getState() ? "on" : "off");
    sendMqttMessage(baseTopic + "brightness", device.getBrightness());
    sendMqttMessage(baseTopic + "temperature", device.getTemperature());
  }
  else
  {
    sendMqttMessage(baseTopic + "availability", "offline");
  }
}, &scheduler, true);

Task checkBle(1 * TASK_MINUTE, TASK_FOREVER, []()
{
  log("Check BLE connection");

  if (!device.connected())
  {
    log("Trying to connect BLE Device [" DEVICE_MAC "]");
    device.connect(DEVICE_MAC, DEVICE_PASSWORD);
  }
}, &scheduler, true);


void onMqttMessage(String &topic, String &payload)
{
  log("Handle Topic %s: %s", topic.c_str(), payload.c_str());

  if(topic.endsWith("state"))
  {
    auto on = payload.equalsConstantTime("ON") || payload.equalsConstantTime("on");
    device.setState(on);
  }
  if(topic.endsWith("brightness"))
  {
    uint8_t val = payload.toInt();
    device.setBrightness(val);
  }
  if(topic.endsWith("temperature"))
  {
    uint16_t val = payload.toInt();
    device.setTemperature(val);
  }

  sendStateUpdate.forceNextIteration();
}

void setup()
{
#if defined(__DEBUG)
  Serial.begin(115200);
#endif

  device.begin();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  mqtt.begin(MQTT_BROKER, net);
  mqtt.onMessage(onMqttMessage);

  log("Starting scheduler");
  scheduler.startNow();
}
void loop()
{
  while (WiFi.status() != WL_CONNECTED)
    delay(1000);
  scheduler.execute();
}
