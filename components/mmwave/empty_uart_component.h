#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace dfrobot_sensor_component {

class EmptyUARTComponent : public uart::UARTDevice, public Component {
  public:
    void setup() override;
    void loop() override;
    void dump_config() override;
};


}  // namespace dfrobot_sensor_component
}  // namespace esphome