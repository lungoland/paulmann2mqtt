#pragma once
#include <string>
#include <memory>

#include <BLEDevice.h>


class Paulmann
{
    static const BLEUUID SERVICE_UUID;

    static const BLEUUID CHARACTERISTIC_PIN;
    static const BLEUUID CHARACTERISTIC_STATE;
    static const BLEUUID CHARACTERISTIC_BRIGHTNESS;
    static const BLEUUID CHARACTERISTIC_TEMPERATURE;

public:
    Paulmann();
    static void begin();

    bool connect(const std::string& mac, const std::string& pin);
    bool connected() const;


    // 0,1
    void setState(bool state);
    bool getState();

    // 1-100
    void setBrightness(uint8_t brightness);
    uint8_t getBrightness();

    // 2700 - 6500
    void setTemperature(uint16_t kelvin);
    uint16_t getTemperature();

private:
    bool sendCommand(BLEUUID characteristic, const std::string& payload);
    std::string getValue(BLEUUID characteristic);

    std::unique_ptr<BLEClient> bleHandle;
    std::string pin;
};
