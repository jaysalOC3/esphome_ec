#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include <vector>

namespace esphome {
namespace mmwave_ns {

class MMWaveComponent : public Component, public uart::UARTDevice {
public:
    MMWaveComponent();
    void setup() override;
    void loop() override;
    void dump_config() override;

protected:
    enum ParseState {
        STATE_HEADER_START,
        STATE_HEADER_END,
        STATE_LENGTH_H,
        STATE_LENGTH_L,
        STATE_DATA,
        STATE_CHECKSUM,
        STATE_TAIL_1,
        STATE_TAIL_2,
    };
    
    ParseState state_{STATE_HEADER_START};
    std::vector<uint8_t> data_;
    uint16_t data_length_{0};

private:
    void process_packet();
    void process_presence_data();
    void process_engineering_data();
    uint8_t sumData(uint8_t len, uint8_t *buf);
};

} // namespace mmwave_ns
} // namespace esphome