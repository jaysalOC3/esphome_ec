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
                    case STATE_HEADER_START:
                        data_.clear(); // Clear previous data
                        if (c == 0x53)
                        {
                            data_.push_back(c);
                            state_ = STATE_HEADER_END;
                            ESP_LOGD(TAG, "Header Start found");
                        }
                        break;

                    case STATE_HEADER_END:
                        if (c == 0x59)
                        {
                            data_.push_back(c);
                            state_ = STATE_LENGTH_H;
                            ESP_LOGD(TAG, "Header End found");
                        }
                        else
                        {
                            state_ = STATE_HEADER_START;
                            ESP_LOGW(TAG, "Invalid header end byte");
                        }
                        break;

                    case STATE_LENGTH_H:
                        data_.push_back(c);
                        data_length_ = c << 8;
                        state_ = STATE_LENGTH_L;
                        break;

                    case STATE_LENGTH_L:
                        data_.push_back(c);
                        data_length_ |= c;
                        state_ = STATE_DATA;
                        ESP_LOGD(TAG, "Length: %d", data_length_);
                        break;

                    case STATE_DATA:
                        data_.push_back(c);
                        if (data_.size() >= data_length_ + 4)
                        { // +4 for header and length bytes
                            state_ = STATE_CHECKSUM;
                        }
                        break;

                    case STATE_CHECKSUM:
                        data_.push_back(c);
                        {
                            uint8_t calc_checksum = 0;
                            for (size_t i = 0; i < data_.size() - 1; i++)
                            {
                                calc_checksum += data_[i];
                            }
                            if (calc_checksum == c)
                            {
                                state_ = STATE_TAIL_1;
                                ESP_LOGD(TAG, "Checksum valid");
                            }
                            else
                            {
                                ESP_LOGE(TAG, "Checksum mismatch: expected 0x%02X, got 0x%02X",
                                         calc_checksum, c);
                                state_ = STATE_HEADER_START;
                            }
                        }
                        break;

                    case STATE_TAIL_1:
                        data_.push_back(c);
                        if (c == 0x54)
                        {
                            state_ = STATE_TAIL_2;
                        }
                        else
                        {
                            ESP_LOGE(TAG, "Invalid tail byte 1");
                            state_ = STATE_HEADER_START;
                        }
                        break;

                    case STATE_TAIL_2:
                        if (c == 0x43)
                        {
                            data_.push_back(c);
                            process_packet();
                        }
                        else
                        {
                            ESP_LOGE(TAG, "Invalid tail byte 2");
                        }
                        state_ = STATE_HEADER_START;
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