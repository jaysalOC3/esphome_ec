#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "mmwave_component.h"
#include "esphome/components/text_sensor/text_sensor.h" // Include for text_sensor
#include <sstream>
#include <iomanip>

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
            ESP_LOGV(TAG, "Setting up MMWave component...");

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
                uint8_t c = this->read();
                ESP_LOGV(TAG, "Received byte: 0x%02X", c); // Log every received byte
                last_byte_time_ = millis();

                if (data_.size() >= MAX_PACKET_SIZE)
                {
                    ESP_LOGW(TAG, "Buffer overflow, resetting parser");
                    data_.clear();
                    state_ = ParseState::STATE_HEADER_START;
                    return;
                }

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
                        ESP_LOGV(TAG, "Header found");
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
            uint16_t data_length = (data_[4] << 8) | data_[5]; // Corrected Indexing
            ESP_LOGV(TAG, "Data length from packet: %d bytes", data_length);

            uint8_t cmd = data_[2];
            ESP_LOGV(TAG, "Command received: 0x%02X", cmd); // Log the command

            // Now the payload is simply a view into the existing data_ vector
            const uint8_t *payload_start = data_.data() + 6;
            size_t payload_size = data_length;
            std::vector<uint8_t> payload(payload_start, payload_start + payload_size);

            switch (cmd)
            {
            case 0x80:
                process_presence_data(payload); // Now just stores the packet
                break;
            case 0x85:
                process_engineering_data(payload);
                break;
            default:
                ESP_LOGW(TAG, "Unknown command received: 0x%02X", cmd);
                break;
            }
        }

        void MMWaveComponent::process_presence_data(const std::vector<uint8_t> &payload)
        {
            ESP_LOGD(TAG, "Storing presence data packet");
            std::stringstream ss;
            ss << "Raw Packet (Cmd 0x80):";
            for (uint8_t byte : data_) {
                ss << " " << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
            }
            std::string packet_string = ss.str();

            received_packets_.push_front(packet_string);
            if (received_packets_.size() > num_packets_to_store_) {
                received_packets_.pop_back();
            }

            if (packet_text_sensor_ != nullptr) {
                std::stringstream combined_packets;
                for (const auto& packet : received_packets_) {
                    combined_packets << packet << "\n";
                }
                packet_text_sensor_->publish_state(combined_packets.str());
            } else {
                ESP_LOGW(TAG, "Packet text sensor not initialized yet!");
            }
        }

        void MMWaveComponent::process_engineering_data(const std::vector<uint8_t> &payload)
        {
            ESP_LOGD(TAG, "Processing engineering data");
            // Add specific processing for engineering mode data
        }

        void MMWaveComponent::dump_config()
        {
            ESP_LOGCONFIG(TAG, "MMWave Component:");
            ESP_LOGCONFIG(TAG, "  UART configured");
            if (packet_text_sensor_ != nullptr) {
                LOG_TEXT_SENSOR("  ", "Packet Text Sensor", packet_text_sensor_);
            }
        }

    } // namespace mmwave_ns
} // namespace esphome