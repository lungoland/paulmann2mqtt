#pragma once
#define _TASK_STD_FUNCTION
#include "TaskSchedulerDeclarations.h"

class Paulmann;

class Controller
{
public:
  Controller(Paulmann& device, Scheduler& scheduler);

  void setBrightness(uint8_t brightness);
  void setTemperature(uint16_t kelvin);

private:
  void cb();
  bool reset();
  void finalize();

  Paulmann& device;
  Task task;

  uint8_t  targetBrightness;
  uint16_t targetTemperature;

  uint8_t  startBrightness;
  uint16_t startTemperature;

  int8_t  stepBrightness;
  int16_t stepTemperature;
};
