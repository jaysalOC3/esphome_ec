#include "esphome/core/log.h"
#include "esphome/core/application.h"
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
            ESP_LOGD(TAG, "Setting up MMWave component...");

            // Configure UART buffer size if needed
            // this->uart_->set_rx_buffer_size(64);  // Uncomment and adjust if needed
        }

        void MMWaveComponent::loop()
        {
            handle_uart_data();

            // Check for timeout
            if (state_ != ParseState::STATE_HEADER_START &&
                (millis() - last_byte_time_) > PACKET_TIMEOUT_MS)
            {
                ESP_LOGW(TAG, "Packet timeout - resetting parser");
                data_.clear();
                state_ = ParseState::STATE_HEADER_START;
            }

            delay(10); // Reduced from continuous polling to prevent watchdog issues
        }

        void MMWaveComponent::handle_uart_data()
        {
            static const uint8_t MAX_PACKET_SIZE = 32;
            static const uint8_t HEADER_SIZE = 6; // SY + Config + Command + Length(2)

            while (this->available())
            {
                if (data_.size() >= MAX_PACKET_SIZE)
                {
                    ESP_LOGW(TAG, "Buffer overflow, resetting parser");
                    data_.clear();
                    state_ = ParseState::STATE_HEADER_START;
                    return;
                }

                uint8_t c = this->read();
                last_byte_time_ = millis();

                switch (state_)
                {
                case ParseState::STATE_HEADER_START:
                    data_.clear();
                    if (c == 0x53)
                    { // 'S'
                        data_.push_back(c);
                        state_ = ParseState::STATE_HEADER_END;
                    }
                    break;

                case ParseState::STATE_HEADER_END:
                    if (c == 0x59)
                    { // 'Y'
                        data_.push_back(c);
                        state_ = ParseState::STATE_CONFIG;
                        ESP_LOGD(TAG, "Header found");
                    }
                    else
                    {
                        state_ = ParseState::STATE_HEADER_START;
                    }
                    break;

                case ParseState::STATE_CONFIG:
                    data_.push_back(c);
                    state_ = ParseState::STATE_COMMAND;
                    break;

                case ParseState::STATE_COMMAND:
                    data_.push_back(c);
                    state_ = ParseState::STATE_LENGTH_H;
                    break;

                case ParseState::STATE_LENGTH_H:
                    data_.push_back(c);
                    data_length_ = c << 8;
                    state_ = ParseState::STATE_LENGTH_L;
                    break;

                case ParseState::STATE_LENGTH_L:
                    data_.push_back(c);
                    data_length_ |= c;

                    // Validate packet length
                    if (data_length_ > MAX_PACKET_SIZE - HEADER_SIZE)
                    {
                        ESP_LOGW(TAG, "Invalid length: %d", data_length_);
                        state_ = ParseState::STATE_HEADER_START;
                        return;
                    }

                    // Calculate total expected bytes
                    bytes_remaining_ = data_length_;
                    expected_packet_size_ = HEADER_SIZE + data_length_;

                    ESP_LOGD(TAG, "Expecting %d bytes of data (total packet size: %d)",
                             data_length_, expected_packet_size_);

                    state_ = ParseState::STATE_DATA;
                    break;

                case ParseState::STATE_DATA:
                    data_.push_back(c);

                    // Check if we have received all expected bytes
                    if (data_.size() == expected_packet_size_)
                    {
                        ESP_LOGV(TAG, "Received complete packet of %d bytes", data_.size());
                        process_packet();
                        state_ = ParseState::STATE_HEADER_START;
                    }
                    break;
                }
            }
        }

        void MMWaveComponent::process_packet()
        {
            if (data_.size() < 8)
            { // Need at least 8 bytes to read length
                ESP_LOGW(TAG, "Packet too short to read length bytes");
                return;
            }

            // Read length from bytes 6 and 7
            uint16_t data_length = (data_[6] << 8) | data_[7];
            ESP_LOGD(TAG, "Data length from packet: %d bytes", data_length);

            // Read the specified number of bytes from UART
            std::vector<uint8_t> payload;
            payload.reserve(data_length);

            uint32_t start_time = millis();
            while (payload.size() < data_length)
            {
                if (this->available())
                {
                    uint8_t c = this->read();
                    payload.push_back(c);
                    last_byte_time_ = millis();
                }

                // Check for timeout
                if ((millis() - start_time) > PACKET_TIMEOUT_MS)
                {
                    ESP_LOGW(TAG, "Timeout while reading payload. Expected %d bytes, got %d",
                             data_length, payload.size());
                    return;
                }

                delay(1); // Small delay to prevent watchdog issues
            }

            ESP_LOGD(TAG, "Successfully read %d bytes of payload", payload.size());

            // Process the payload based on command type
            uint8_t cmd = data_[2]; // Command byte
            switch (cmd)
            {
            case 0x80:
                process_presence_data(payload);
                break;
            case 0x85:
                process_engineering_data(payload);
                break;
            default:
                ESP_LOGW(TAG, "Unknown command received: 0x%02X", cmd);
                break;
            }
        }

        void MMWaveComponent::process_presence_data(const std::vector<uint8_t>& payload)
        {
            ESP_LOGD(TAG, "Processing presence data");
            // Add specific processing for presence detection data
        }

        void MMWaveComponent::process_engineering_data(const std::vector<uint8_t>& payload)
        {
            ESP_LOGD(TAG, "Processing engineering data");
            // Add specific processing for engineering mode data
        }

        void MMWaveComponent::dump_config()
        {
            ESP_LOGCONFIG(TAG, "MMWave Component:");
            ESP_LOGCONFIG(TAG, "  UART configured");
        }

    } // namespace mmwave_ns
} // namespace esphome