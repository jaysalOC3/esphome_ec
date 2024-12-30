// mmwave_component.h
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome {
namespace mmwave_ns {

class MMWaveComponent : public uart::UARTDevice, public Component {
public:
    // Constructor that matches ESPHome's patterns
    MMWaveComponent() = default;
    
    void setup() override;
    void loop() override;
    void dump_config() override;
    uint8_t begin();
    uint8_t getData(uint8_t con, uint8_t cmd, uint16_t len, uint8_t *senData, uint8_t *retData);
    uint8_t sumData(uint8_t len, uint8_t *buf);

    // Add setter for UART parent
    void set_uart_parent(uart::UARTComponent *parent) { 
        this->set_uart_parent_(parent);
    }

private:
    static const size_t BUFFER_SIZE = 10;
    static const uint8_t CMD_WHITE = 0;
    static const uint8_t CMD_HEAD = 1;
    static const uint32_t TIME_OUT = 1000;
};

} // namespace mmwave_ns
} // namespace esphome