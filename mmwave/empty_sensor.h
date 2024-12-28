#pragma once

#include "esphome/components/sensor/sensor.h"
#include "esphome/core/component.h"

namespace esphome {
namespace mmwave_sensor {

class mmwaveSensor : public sensor::Sensor, public PollingComponent {
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
};

}  // namespace mmwave_sensor
}  // namespace esphome