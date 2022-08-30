#include <Arduino.h>
#include <WiFi.h>
#include <MQTT.h>

// #define _TASK_SLEEP_ON_IDLE_RUN
#include <TaskScheduler.h>

#include "Paulmann.h"
#include "config.h"


WiFiClient net;
MQTTClient mqtt;
Paulmann device;

Scheduler scheduler;


template<typename T>
void sendMqttMessage(const std::string& topic, T payload)
{
  mqtt.publish(topic.c_str(), String(payload));
}

Task mqttConnect(0, 1, []()
{
  if (mqtt.connected())
    return;

#if defined(__DEBUG)
  Serial.println("Trying to connect to " MQTT_BROKER);
#endif

  if(mqtt.connect(DEVICE_NAME, MQTT_USER, MQTT_PASSWORD))
  {
#if defined(__DEBUG)
    Serial.println("Subscribing to " MQTT_BASE_TOPIC "/" DEVICE_NAME);
#endif
    mqtt.subscribe(MQTT_BASE_TOPIC "/" DEVICE_NAME "/set/#");
  }
  else
  {
    mqttConnect.restartDelayed(100);
  }
}, &scheduler);
// Task mqttLoop(100 * TASK_MILLISECOND, TASK_FOREVER, []() { mqtt.loop(); }, &scheduler, true);

Task mqttStatus(10 * TASK_SECOND, TASK_FOREVER, []()
{
#if defined(__DEBUG)
  Serial.println("State update");
#endif

  if (!mqtt.connected())
  {
    mqttConnect.restart();
    return;
  }

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
#if defined(__DEBUG)
  Serial.println("Check device connection");
#endif

  if (!device.connected())
  {
#if defined(__DEBUG)
    Serial.println("Trying to connect BLE Device [" DEVICE_MAC "]");
#endif
    device.connect(DEVICE_MAC, DEVICE_PASSWORD);
  }
}, &scheduler, true);

void onMqttMessage(String &topic, String &payload)
{
#if defined(__DEBUG)
    Serial.printf("Handle Topic %s: %s", topic.c_str(), payload.c_str());
#endif

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

  mqttStatus.forceNextIteration();
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

#if defined(__DEBUG)
  Serial.println("Starting scheduler");
#endif
  scheduler.startNow();
}
void loop()
{
  while (WiFi.status() != WL_CONNECTED)
    delay(1000);
  scheduler.execute();
  mqtt.loop();
}
