#include "Controller.h"
#include "Paulmann.h"

#include <functional>

void log(const char* fmt, ...);

Controller::Controller(Paulmann& device, Scheduler& scheduler)
  : device(device)
  , task(100 * TASK_MILLISECOND, 20,
      std::bind(&Controller::cb, this),
      &scheduler, false,
      std::bind(&Controller::reset, this),
      std::bind(&Controller::finalize, this)
    )
{
}

void Controller::cb()
{
  startBrightness += stepBrightness;
  startTemperature += startTemperature;

  device.setBrightness(startBrightness);
  device.setTemperature(startTemperature);
}
bool Controller::reset()
{
  device.setState(true);
  startBrightness = device.getBrightness();
  startTemperature = device.getTemperature();

  stepBrightness = (targetBrightness - startBrightness) / 20;
  stepTemperature = (targetTemperature - startTemperature) / 20;

  log("Reset Control - brightness %i -> %i (%i)", startBrightness, targetBrightness, stepBrightness);
  return true;
}
void Controller::finalize()
{
  if (targetBrightness == 0)
  {
    device.setState(false);
  }
  else
  {
    device.setBrightness(targetBrightness);
    device.setTemperature(targetTemperature);
  }
}

void Controller::setBrightness(uint8_t brightness)
{
  targetBrightness = brightness;
  task.restart();
  task.enableIfNot();
}
void Controller::setTemperature(uint16_t kelvin)
{
  targetTemperature = kelvin;
  task.restart();
  task.enableIfNot();
}
