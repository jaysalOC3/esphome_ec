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

  private:
    void processSensorData(uint8_t *data, uint8_t dataLen);
    uint8_t sumData(uint8_t len, uint8_t *buf);
};


}  // namespace mmwave_component
}  // namespace esphome
