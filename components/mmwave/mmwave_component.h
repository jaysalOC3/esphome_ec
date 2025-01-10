#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/number/number.h"
#include <vector>
#include <deque> // For storing multiple packets

namespace esphome
{
    namespace mmwave_ns
    {
        class MMWaveNumber : public number::Number
        {
        public:
            void control(float value) override
            {
                // This function is called when the number value is changed from Home Assistant
                // For most sensor values, we don't need to do anything here since they're read-only
                this->publish_state(value);
            }
        };

        class MMWaveComponent : public Component, public uart::UARTDevice
        {
        public:
            MMWaveComponent();
            void setup() override;
            void loop() override;
            void dump_config() override;

            void begin();
            void send_command();
            void start_work_mode();
            void sensor_restart();
            void send_sleep_mode_command();
            void get_sleep_composite();

            void set_packet_text_sensor(text_sensor::TextSensor *packet_sensor) { packet_text_sensor_ = packet_sensor; }
            void set_config_text_sensor(text_sensor::TextSensor *config_sensor) { config_text_sensor_ = config_sensor; }
            void set_position_sensor(text_sensor::TextSensor *position_sensor) { position_text_sensor_ = position_sensor; }
            void set_movement_sensor(text_sensor::TextSensor *movement_sensor) { movement_sensor_ = movement_sensor; }

            void set_presence_sensor(MMWaveNumber *presence_sensor) { presence_sensor_ = presence_sensor; }
            void set_sleep_state_sensor(MMWaveNumber *sleep_state_sensor) { sleep_state_sensor_ = sleep_state_sensor; }
            void set_average_respiration_sensor(MMWaveNumber *average_respiration_sensor) { average_respiration_sensor_ = average_respiration_sensor; }
            void set_average_heartbeat_sensor(MMWaveNumber *average_heartbeat_sensor) { average_heartbeat_sensor_ = average_heartbeat_sensor; }
            void set_turnover_sensor(MMWaveNumber *turnover_sensor) { turnover_sensor_ = turnover_sensor; }
            void set_large_bodymove_sensor(MMWaveNumber *large_bodymove_sensor) { large_bodymove_sensor_ = large_bodymove_sensor; }
            void set_minor_bodymove_sensor(MMWaveNumber *minor_bodymove_sensor) { minor_bodymove_sensor_ = minor_bodymove_sensor; }
            void set_apnea_events_sensor(MMWaveNumber *apnea_events_sensor) { apnea_events_sensor_ = apnea_events_sensor; }

            void set_num_packets_to_store(int num_packets) { num_packets_to_store_ = num_packets; }

        private:
            void process_packet();
            void process_presence_data(const std::vector<uint8_t> &payload); // Will now just store the packet
            void process_movement_data(const std::vector<uint8_t> &payload); // Will now just store the packet
            void process_position_data(const std::vector<uint8_t> &payload); // Will now just store the packet
            void process_engineering_data(const std::vector<uint8_t> &payload);
            void process_sleep_composite(const std::vector<uint8_t> &payload);
            void handle_uart_data();
            uint8_t sumData(uint8_t len, uint8_t *buf);
            static const uint32_t PACKET_TIMEOUT_MS = 25; // Timeout in milliseconds
            uint32_t last_byte_time_{0};
            uint16_t total_expected_bytes_{0};
            uint16_t bytes_remaining_{0};
            uint16_t expected_packet_size_{0};

            enum DeviceState
            {
                STATE_POWER_ON,
                STATE_SLEEP,
                STATE_SENSOR_INIT,
                STATE_SENSOR_CHG_MODE,
                STATE_SENSOR_SLEEP_MODE,
                STATE_SENSOR_ERROR,
            };

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
            DeviceState device_state_{STATE_POWER_ON};
            DeviceState device_state_history{STATE_SLEEP};

            std::vector<uint8_t> data_;
            uint16_t data_length_{0};

            text_sensor::TextSensor *packet_text_sensor_{nullptr};
            text_sensor::TextSensor *config_text_sensor_{nullptr};
            text_sensor::TextSensor *position_text_sensor_{nullptr};
            text_sensor::TextSensor *movement_sensor_{nullptr};

            MMWaveNumber *presence_sensor_{nullptr};
            MMWaveNumber *sleep_state_sensor_{nullptr};
            MMWaveNumber *average_respiration_sensor_{nullptr};
            MMWaveNumber *average_heartbeat_sensor_{nullptr};
            MMWaveNumber *turnover_sensor_{nullptr};
            MMWaveNumber *large_bodymove_sensor_{nullptr};
            MMWaveNumber *minor_bodymove_sensor_{nullptr};
            MMWaveNumber *apnea_events_sensor_{nullptr};

            std::deque<std::string> received_packets_;
            int num_packets_to_store_{5}; // Default to storing 5 packets
        };
    } // namespace mmwave_ns
} // namespace esphome