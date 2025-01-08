#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/text_sensor/text_sensor.h" // Include for text_sensor
#include <vector>
#include <deque> // For storing multiple packets

namespace esphome
{
    namespace mmwave_ns
    {

        class MMWaveComponent : public Component, public uart::UARTDevice
        {
        public:
            MMWaveComponent();
            void setup() override;
            void loop() override;
            void dump_config() override;

            void set_packet_text_sensor(text_sensor::TextSensor *packet_sensor) { packet_text_sensor_ = packet_sensor; }
            void set_num_packets_to_store(int num_packets) { num_packets_to_store_ = num_packets; }

        private:
            void process_packet();
            void process_presence_data(const std::vector<uint8_t>& payload); // Will now just store the packet
            void process_movement_data(const std::vector<uint8_t>& payload); // Will now just store the packet
            void process_engineering_data(const std::vector<uint8_t>& payload);
            void handle_uart_data();
            uint8_t sumData(uint8_t len, uint8_t *buf);
            static const uint32_t PACKET_TIMEOUT_MS = 100; // Timeout in milliseconds
            uint32_t last_byte_time_{0};
            uint16_t total_expected_bytes_{0};
            uint16_t bytes_remaining_{0};
            uint16_t expected_packet_size_{0};

            enum ParseState
            {
                STATE_HEADER_START,
                STATE_HEADER_END,
                STATE_CONFIG,
                STATE_COMMAND,
                STATE_LENGTH_H,
                STATE_LENGTH_L,
                STATE_DATA,
                STATE_CHECKSUM,
                STATE_FOOTER_START,
                STATE_FOOTER_END,
            };

            ParseState state_{STATE_HEADER_START};
            std::vector<uint8_t> data_;
            uint16_t data_length_{0};

            text_sensor::TextSensor *packet_text_sensor_{nullptr}; // Add text_sensor member
            text_sensor::TextSensor *movement_sensor_{nullptr};
            std::deque<std::string> received_packets_;
            int num_packets_to_store_{5}; // Default to storing 5 packets
        };
    } // namespace mmwave_ns
} // namespace esphome