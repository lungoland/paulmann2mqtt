#include "Paulmann.h"
#include <Arduino.h>


const BLEUUID Paulmann::SERVICE_UUID = BLEUUID((uint16_t) 0xffb0);

const BLEUUID Paulmann::CHARACTERISTIC_PIN = BLEUUID((uint16_t) 0xffba);
const BLEUUID Paulmann::CHARACTERISTIC_STATE = BLEUUID((uint16_t) 0xffb7);
const BLEUUID Paulmann::CHARACTERISTIC_BRIGHTNESS = BLEUUID((uint16_t) 0xffb8);
const BLEUUID Paulmann::CHARACTERISTIC_TEMPERATURE = BLEUUID((uint16_t) 0xffb6);

Paulmann::Paulmann()
    : bleHandle(BLEDevice::createClient())
{}

void Paulmann::begin()
{
  BLEDevice::init("");
}

bool Paulmann::connect(const std::string& mac, const std::string& pin)
{
    this->pin = pin;
    return bleHandle->connect(BLEAddress(mac));
}
bool Paulmann::connected() const
{
    return bleHandle->isConnected();
}


void Paulmann::setState(bool state)
{
    std::string payload(state ? "\001" : "\000", 1);
    sendCommand(CHARACTERISTIC_STATE, payload);
}
bool Paulmann::getState()
{
    return getValue(CHARACTERISTIC_STATE).c_str()[0];
}

void Paulmann::setBrightness(uint8_t brightness)
{
    std::string payload(1, brightness);
    sendCommand(CHARACTERISTIC_BRIGHTNESS, payload);
}
uint8_t Paulmann::getBrightness()
{
    return getValue(CHARACTERISTIC_BRIGHTNESS).c_str()[0];
}

void Paulmann::setTemperature(uint16_t kelvin)
{
    std::string payload(2, 0x00);
    payload[0] = static_cast<uint8_t>(kelvin & 0x00FF);
    payload[1] = static_cast<uint8_t>((kelvin & 0xFF00) >> 8);
    sendCommand(CHARACTERISTIC_TEMPERATURE, payload);
}
uint16_t Paulmann::getTemperature()
{
    auto str = getValue(CHARACTERISTIC_TEMPERATURE).c_str();
    uint16_t expander = str[1];
    return (expander << 8) + str[0];
}

bool Paulmann::sendCommand(BLEUUID characteristic, const std::string& payload)
{
    bleHandle->setValue(SERVICE_UUID, CHARACTERISTIC_PIN, pin);
    bleHandle->setValue(SERVICE_UUID, characteristic, payload);
    return true;
}
std::string Paulmann::getValue(BLEUUID characteristic)
{
    bleHandle->setValue(SERVICE_UUID, CHARACTERISTIC_PIN, pin);
    return bleHandle->getValue(SERVICE_UUID, characteristic);
}