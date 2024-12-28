#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "DFRobot_HumanDetection.h"

namespace esphome {
namespace mmwave_sensor {

class MMWaveSensor : public sensor::Sensor, public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void update() override;
  void loop() override;
  void dump_config() override;
  void printHumanPresence(int presence);
  void printHumanMovement(int movement);

 private:
  DFRobot_HumanDetection hu;
};

}  // namespace mmwave_sensor
}  // namespace esphome
