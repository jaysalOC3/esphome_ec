#include "esphome/core/log.h"
#include "esphome/core/application.h"
#include "mmwave_component.h"
#include "esphome/components/text_sensor/text_sensor.h"
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

            // Schedule a callback to change DeviceState to Init 10 seconds after boot
            this->set_timeout("init_state", 5000, [this]()
                              {
                this->device_state_ = DeviceState::STATE_SENSOR_INIT;
                ESP_LOGI(TAG, "Device state changed to STATE_SENSOR_INIT after 10 seconds"); });
            // Configure UART buffer size if needed
            // this->uart_->set_rx_buffer_size(64);  // Uncomment and adjust if needed

            // if (control_button_ != nullptr)
            // {
            //     control_button_->add_on_press_callback([this]()
            //                                            { ESP_LOGI(TAG, "Button clicked!"); });
            // }
        }

        void MMWaveComponent::loop()
        {
            if (device_state_ != DeviceState::STATE_SENSOR_ERROR || device_state_ != DeviceState::STATE_POWER_ON)
            {
                // Check for packet timeout
                if (state_ != ParseState::STATE_HEADER_START &&
                    (millis() - last_byte_time_) > PACKET_TIMEOUT_MS)
                {
                    ESP_LOGW(TAG, "Packet timeout - resetting parser");
                    data_.clear();
                    state_ = ParseState::STATE_HEADER_START;
                }

                handle_uart_data();

                // Process DeviceState
                switch (device_state_)
                {
                case DeviceState::STATE_SENSOR_INIT:
                    ESP_LOGV(TAG, "Device state: STATE_SENSOR_INIT");
                    device_state_history = DeviceState::STATE_SENSOR_CHG_MODE;
                    device_state_ = DeviceState::STATE_SLEEP;
                    this->begin();
                    break;
                case DeviceState::STATE_SENSOR_CHG_MODE:
                    ESP_LOGV(TAG, "Device state: STATE_SENSOR_CHG_MODE");
                    device_state_history = DeviceState::STATE_SENSOR_SLEEP_MODE;
                    device_state_ = DeviceState::STATE_SLEEP;
                    this->start_work_mode();
                    break;
                case DeviceState::STATE_SENSOR_SLEEP_MODE:
                    ESP_LOGV(TAG, "Device state: STATE_SENSOR_SLEEP_MODE");
                    device_state_history = DeviceState::STATE_SENSOR_ERROR;
                    device_state_ = DeviceState::STATE_SLEEP;
                    this->send_sleep_mode_command();
                    break;
                case DeviceState::STATE_SLEEP:
                    delay(1);
                    break;
                }
            }
        }

        void MMWaveComponent::handle_uart_data()
        {
            static const uint8_t MAX_PACKET_SIZE = 32;
            static const uint8_t HEADER_SIZE = 6; // SY + Config + Command + Length(2)
            uint8_t cfg = 0;
            uint8_t cmd = 0;

            while (this->available())
            {
                uint8_t c = this->read();

                // ESP_LOGVV(TAG, "Received byte: 0x%02X", c); // Log every received byte
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
                    cfg = c;
                    state_ = ParseState::STATE_COMMAND;
                    break;

                case ParseState::STATE_COMMAND:
                    data_.push_back(c);
                    cmd = c;
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
                        ESP_LOGVV(TAG, "Received complete packet of %d bytes", data_.size());

                        process_packet();
                        state_ = ParseState::STATE_HEADER_START;
                        return;
                    }
                    break;
                }
            }
        }

        void MMWaveComponent::process_packet()
        {
            uint16_t data_length = (data_[4] << 8) | data_[5];
            // ESP_LOGVV(TAG, "Data length from packet: %d bytes", data_length);

            uint8_t cmd = data_[2];
            // ESP_LOGVV(TAG, "Command: 0x%02X, Instruction: 0x%02X packets", cmd, data_[3]);

            // Now the payload is simply a view into the existing data_ vector
            const uint8_t *payload_start = data_.data() + 6;
            size_t payload_size = data_length;
            std::vector<uint8_t> payload(payload_start, payload_start + payload_size);

            switch (cmd)
            {
            case 0x01:
                if (data_[3] == 0x02)
                {
                    if (data_[6] == 0x0F)
                    {
                        ESP_LOGVV(TAG, "Sleep Mode successful.", cmd);
                        device_state_ = device_state_history;
                        packet_text_sensor_->publish_state("Sleep Mode successful.");
                    }
                    else
                    {
                        ESP_LOGVV(TAG, "Sleep Mode failed.", cmd);
                        delay(250);
                        device_state_ = DeviceState::STATE_SENSOR_SLEEP_MODE;
                        packet_text_sensor_->publish_state("Sleep Mode failed.");
                    }
                }
                if (data_[3] == 0x83)
                {
                    if (data_[6] == 0x01)
                    {
                        ESP_LOGVV(TAG, "LED On.", cmd);
                        packet_text_sensor_->publish_state("LED On.");
                    }
                    else
                    {
                        ESP_LOGVV(TAG, "Initialization Successful.", cmd);
                        packet_text_sensor_->publish_state("Initialization Successful.");
                    }
                }
                break;
            case 0x02:
                if (data_[3] == 0x83)
                {
                    ESP_LOGVV(TAG, "Work mode switch successful.", cmd);
                    packet_text_sensor_->publish_state("Work mode switch successful.");
                }
                if (data_[3] == 0xA8)
                {
                    if (data_[6] == 0x02)
                    {
                        ESP_LOGVV(TAG, "Work mode switch successful.", cmd);
                        device_state_ = device_state_history;
                        packet_text_sensor_->publish_state("Work mode switch successful.");
                    }
                    else
                    {
                        ESP_LOGVV(TAG, "Work mode switch failed.", cmd);
                        delay(250);
                        device_state_ = DeviceState::STATE_SENSOR_CHG_MODE;
                        packet_text_sensor_->publish_state("Work mode switch failed.");
                    }
                }
                break;
            case 0x80:
                if (data_[3] == 0x03)
                {
                    ESP_LOGVV(TAG, "Command 80 and Instruction 03 data avalible.", cmd);
                    process_movement_data(payload);
                }
                if (data_[3] == 0x05)
                {
                    ESP_LOGVV(TAG, "Command 80 and Instruction 05 data avalible.", cmd);
                    process_position_data(payload);
                }
                break;
            case 0x81:
                ESP_LOGVV(TAG, "Command 81 and Instruction data avalible.", cmd);
                process_position_data(payload);
                break;
            case 0x84:
                if (data_[3] == 0x8D)
                {
                    ESP_LOGD(TAG, "SleepComposite data avalible.", cmd);
                    packet_text_sensor_->publish_state("SleepComposite data avalible.");
                    process_sleep_composite(payload);
                }
                break;
            case 0x85:
                process_engineering_data(payload);
                break;

            default:
                ESP_LOGW(TAG, "Unknown command received: 0x%02X", cmd);
                std::stringstream ss;
                ss << "cfg: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(data_[2]) << " ";
                ss << "cmd: " << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(data_[3]) << " ";
                for (size_t i = 0; i < data_.size(); ++i)
                {
                    ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data_[i]) << " ";
                }
                ESP_LOGVV(TAG, "%s", ss.str().c_str());
                packet_text_sensor_->publish_state(ss.str().c_str());

                break;
            }
        }

        void MMWaveComponent::process_position_data(const std::vector<uint8_t> &payload)
        {
            ESP_LOGV(TAG, "Processing position data packets");

            if (position_text_sensor_ != nullptr)
            {
                if (payload.size() >= 6)
                { // Ensure enough data for X and Y

                    // Assuming X is the first 3 bytes, Y is the next 3 bytes
                    int32_t x = (payload[0] << 16) | (payload[1] << 8) | payload[2];
                    int32_t y = (payload[3] << 16) | (payload[4] << 8) | payload[5];

                    std::stringstream ss;
                    ss << "X: " << x << ", Y: " << y;
                    position_text_sensor_->publish_state(ss.str().c_str());
                }
                else
                {
                    ESP_LOGW(TAG, "Insufficient data for X,Y coordinates");
                }
            }
            else
            {
                ESP_LOGW(TAG, "Position sensor not initialized yet!");
            }
        }

        void MMWaveComponent::process_movement_data(const std::vector<uint8_t> &payload)
        {
            ESP_LOGV(TAG, "Processing movement data packets");

            if (movement_sensor_ != nullptr)
            {
                movement_sensor_->publish_state(data_[6]);
            }
            else
            {
                ESP_LOGW(TAG, "Movement sensor not initialized yet!");
            }
        }

        void MMWaveComponent::process_engineering_data(const std::vector<uint8_t> &payload)
        {
            ESP_LOGV(TAG, "Processing engineering data");
            // Add specific processing for engineering mode data
        }

        void MMWaveComponent::process_sleep_composite(const std::vector<uint8_t> &payload)
        {
            ESP_LOGV(TAG, "Processing Composite Sleep data");
            if (payload.size() >= 8)
            {
                int presence = payload[0];
                int sleepState = payload[1];
                int averageRespiration = payload[2];
                int averageHeartbeat = payload[3];
                int turnoverNumber = payload[4];
                int largeBodyMove = payload[5];
                int minorBodyMove = payload[6];
                int apneaEvents = payload[7];

                ESP_LOGV(TAG, "Publishing sleep composite data: presence=%d, sleepState=%d, averageRespiration=%d, averageHeartbeat=%d, turnoverNumber=%d, largeBodyMove=%d, minorBodyMove=%d, apneaEvents=%d",
                         presence, sleepState, averageRespiration, averageHeartbeat, turnoverNumber, largeBodyMove, minorBodyMove, apneaEvents);

                if (presence_sensor_ != nullptr)
                    presence_sensor_->publish_state(presence == 1);
                if (sleep_state_sensor_ != nullptr)
                    sleep_state_sensor_->publish_state(sleepState);
                if (average_respiration_sensor_ != nullptr)
                    average_respiration_sensor_->publish_state(averageRespiration);
                if (average_heartbeat_sensor_ != nullptr)
                    average_heartbeat_sensor_->publish_state(averageHeartbeat);
                if (turnover_sensor_ != nullptr)
                    turnover_sensor_->publish_state(turnoverNumber);
                if (large_bodymove_sensor_ != nullptr)
                    large_bodymove_sensor_->publish_state(largeBodyMove);
                if (minor_bodymove_sensor_ != nullptr)
                    minor_bodymove_sensor_->publish_state(minorBodyMove);
                if (apnea_events_sensor_ != nullptr)
                    apnea_events_sensor_->publish_state(apneaEvents);
            }
            else
            {
                ESP_LOGW(TAG, "Insufficient data for sleep composite");
            }
        }

        void MMWaveComponent::begin()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x01, 0x83, 0x00, 0x01, 0x0f, 0x40, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            state_ = ParseState::STATE_HEADER_START;
            ESP_LOGV(TAG, "Sent Begin: 0x0f");
            data_.clear();
            state_ = ParseState::STATE_HEADER_START;
            delay(50);
        }

        void MMWaveComponent::send_command()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x02, 0xA8, 0x00, 0x01, 0x0f, 0x0f, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            state_ = ParseState::STATE_HEADER_START;
            ESP_LOGV(TAG, "Sent command: 0x00");
            data_.clear();
            state_ = ParseState::STATE_HEADER_START;
            delay(50);
        }

        void MMWaveComponent::start_work_mode()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x02, 0xA8, 0x00, 0x01, 0x0F, 0x66, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Sent Get Work Mode: 0x80 0x0A");
            data_.clear();
            state_ = ParseState::STATE_HEADER_START;
            delay(50);
        }

        void MMWaveComponent::sensor_restart()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x01, 0x03, 0x00, 0x01, 0x01, 0xB2, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Send Sensor Restart: 0x01 0x02, 0x0f");
            data_.clear();
            state_ = ParseState::STATE_HEADER_START;
            delay(50);
        }

        void MMWaveComponent::get_sleep_composite()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x84, 0x8D, 0x00, 0x01, 0x0F, 0xCD, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Send Sleep Composite: 0x84 0x8D, 0x0f");
            data_.clear();
            state_ = ParseState::STATE_HEADER_START;
            delay(50);
        }

        void MMWaveComponent::send_sleep_mode_command()
        {
            // Start switching work mode
            uint8_t cmdBufSleepConfig[10] = {0x53, 0x59, 0x02, 0xA8, 0x00, 0x01, 0x0F, 0x66, 0x54, 0x43};
            this->write_array(cmdBufSleepConfig, sizeof(cmdBufSleepConfig));
            ESP_LOGI(TAG, "Start switching work mode.");

            // You might need a delay here, depending on the sensor's response time
            delay(100); // Adjust delay as needed

            // Sleep Command
            uint8_t cmdBufSleep[10] = {0x53, 0x59, 0x02, 0xA8, 0x00, 0x01, 0x0F, 0x66, 0x54, 0x43};
            this->write_array(cmdBufSleep, sizeof(cmdBufSleep));
            ESP_LOGI(TAG, "Sent sleep command.");
        }

        void MMWaveComponent::dump_config()
        {
            ESP_LOGCONFIG(TAG, "MMWave Component:");
            ESP_LOGCONFIG(TAG, "  UART configured");
            if (packet_text_sensor_ != nullptr)
            {
                LOG_TEXT_SENSOR("  ", "Packet Text Sensor", packet_text_sensor_);
            }
            if (config_text_sensor_ != nullptr)
            {
                LOG_TEXT_SENSOR("  ", "Packet Text Sensor", config_text_sensor_);
            }
        }

    } // namespace mmwave_ns
} // namespace esphome
