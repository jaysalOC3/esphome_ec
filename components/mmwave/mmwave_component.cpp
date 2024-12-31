#include "esphome/core/log.h"
#include "mmwave_component.h"

namespace esphome
{
    namespace mmwave_ns
    {

        static const char *TAG = "mmwave";

        MMWaveComponent::MMWaveComponent()
        {
            data_.reserve(32); // Pre-allocate space for typical packet size
        }

        void MMWaveComponent::setup()
        {
            ESP_LOGD(TAG, "Setting up MMWave component... Setup()");
        }
        void MMWaveComponent::loop()
        {
            if (this->available())
            {
                ESP_LOGD(TAG, "Data available on UART");

                while (this->available())
                {
                    uint8_t c = this->read();
                    ESP_LOGD(TAG, "State: %d, Byte: 0x%02X", (int)state_, c);

                    switch (state_)
                    {
                    case ParseState::STATE_HEADER_START:
                        data_.clear(); // Clear previous data
                        if (c == 0x53)
                        {
                            data_.push_back(c);
                            state_ = ParseState::STATE_HEADER_END;
                            ESP_LOGD(TAG, "Header start found");
                        }
                        break;
                    case ParseState::STATE_HEADER_END:
                        if (c == 0x59)
                        {
                            data_.push_back(c);
                            state_ = ParseState::STATE_CONFIG;
                            ESP_LOGD(TAG, "Header Validated");
                        }
                        else
                        {
                            state_ = ParseState::STATE_HEADER_START;
                        }
                        break;
                    case ParseState::STATE_CONFIG:
                        // data_.push_back(c);
                        state_ = ParseState::STATE_COMMAND;
                        ESP_LOGD(TAG, "Config found: 0x%02X", c);
                        break;
                    case ParseState::STATE_COMMAND:
                        // data_.push_back(c);
                        state_ = ParseState::STATE_LENGTH_H;
                        ESP_LOGD(TAG, "Command found: 0x%02X", c);
                        break;
                    case ParseState::STATE_LENGTH_H:
                        // data_.push_back(c);
                        state_ = ParseState::STATE_LENGTH_L;
                        ESP_LOGD(TAG, "Data Length H: 0x%02X", c);
                        break;
                    case ParseState::STATE_LENGTH_L:
                        // data_.push_back(c);
                        state_ = ParseState::STATE_HEADER_START;
                        ESP_LOGD(TAG, "Data Length L: 0x%02X", c);
                        ESP_LOGD(TAG, "The next %d packets are data.", c);
                        
                        for (int c_decimal = c; c_decimal < 3; c_decimal++)
                        {
                            c = this->read();
                            ESP_LOGD(TAG, "Packet: 0x%02X", c);
                        }
                        break;
                    }
                }
            }
        }

        void MMWaveComponent::process_packet()
        {
            if (data_.size() < 6)
                return; // Minimum packet size check

            uint8_t cmd = data_[2]; // Command byte after header
            ESP_LOGI(TAG, "Received command: 0x%02X", cmd);

            // Print complete packet for debugging
            std::string packet_hex;
            char hex[4];
            for (uint8_t byte : data_)
            {
                sprintf(hex, "%02X ", byte);
                packet_hex += hex;
            }
            ESP_LOGI(TAG, "Complete packet: %s", packet_hex.c_str());

            switch (cmd)
            {
            case 0x80:
                process_presence_data();
                break;
            case 0x85:
                process_engineering_data();
                break;
            default:
                ESP_LOGW(TAG, "Unknown command received");
                break;
            }
        }

        void MMWaveComponent::process_presence_data()
        {
            ESP_LOGI(TAG, "Processing presence data");
            // Add specific processing for presence detection data
        }

        void MMWaveComponent::process_engineering_data()
        {
            ESP_LOGI(TAG, "Processing engineering data");
            // Add specific processing for engineering mode data
        }

        void MMWaveComponent::dump_config()
        {
            ESP_LOGCONFIG(TAG, "MMWave Component:");
            ESP_LOGCONFIG(TAG, "  UART configured");
        }

        uint8_t MMWaveComponent::sumData(uint8_t len, uint8_t *buf)
        {
            uint16_t data = 0;
            uint8_t *_buf = buf;
            for (uint8_t i = 0; i < len; i++)
            {
                data += _buf[i];
            }
            return data & 0xff;
        }

    } // namespace mmwave_component
} // namespace esphome