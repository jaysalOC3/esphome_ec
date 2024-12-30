#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace mmwave_ns {

class MMWaveComponent : public uart::UARTDevice, public Component {
 public:
  // Constants for commands and control bytes
  static const uint8_t CMD_WHITE = 0x00;  // Initial state of the command parser
  static const uint8_t CMD_HEAD = 0x01;   // State after receiving the header byte (0x53)
  // ... add more states as needed ...

  static const uint32_t TIME_OUT = 5000;  // Timeout for communication in milliseconds

  uint8_t begin(void);
  uint8_t getData(uint8_t con, uint8_t cmd, uint16_t len, uint8_t *senData,
                  uint8_t *retData);  // Declaration of getData()
  void setup() override;
  void loop() override;
  void dump_config() override;

 private:
  uint8_t sumData(uint8_t len, uint8_t *data);
};

}  // namespace mmwave_component
}  // namespace esphome