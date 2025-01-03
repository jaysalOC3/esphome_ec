#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace mmwave_ns {

class MMWaveComponent : public uart::UARTDevice, public Component {
  public:
    void setup() override;
    void loop() override;
    void dump_config() override;
};


}  // namespace mmwave_component
}  // namespace esphome