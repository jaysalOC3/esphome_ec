#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome
{
  namespace mmwave_ns
  {

    class MMWaveComponent : public uart::UARTDevice, public Component
    {
    public:
      uint8_t begin(void);
      uint8_t getData(uint8_t con, uint8_t cmd, uint16_t len, uint8_t *senData, uint8_t *retData); // Declaration of getData()
      void setup() override;
      void loop() override;
      void dump_config() override;
    };

  } // namespace mmwave_component
} // namespace esphome