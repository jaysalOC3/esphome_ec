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

        MMWaveComponent::MMWaveComponent() : device_state_(DeviceState::STATE_SENSOR_INIT)
        {
            data_.reserve(32); // Pre-allocate space for typical packet size
        }

        void MMWaveComponent::setup()
        {
            ESP_LOGV(TAG, "Setting up MMWave component...");

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
                this->begin();
                break;

            case DeviceState::STATE_SENSOR_CHG_MODE:
                ESP_LOGV(TAG, "Device state: STATE_SENSOR_CHG_MODE");
                this->start_work_mode();
                device_state_ = DeviceState::STATE_SENSOR_SLEEP_MODE;
                break;

            case DeviceState::STATE_SENSOR_SLEEP_MODE:
                ESP_LOGV(TAG, "Device state: STATE_SENSOR_SLEEP_MODE");
                this->send_sleep_mode_command();
                device_state_ = DeviceState::STATE_SLEEP;
                break;

            case DeviceState::STATE_SLEEP:
                if (millis() - last_operation_time_ >= 1000)
                {
                    last_operation_time_ = millis();
                    // Handle any actions needed during sleep (if any)
                }
                break;

            case DeviceState::STATE_SENSOR_ERROR:
                if (millis() - last_operation_time_ >= 5000)
                {
                    last_operation_time_ = millis();
                    ESP_LOGE(TAG, "Device state: STATE_SENSOR_ERROR - Attempting to recover");
                    // Attempt to recover (e.g., reset the sensor)
                    this->sensor_restart();
                    device_state_ = DeviceState::STATE_SENSOR_INIT;
                }
                break;
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

            uint8_t cfg = data_[2];
            // ESP_LOGVV(TAG, "Config: 0x%02X, Command: 0x%02X packets", cfg, data_[3]);

            // Now the payload is simply a view into the existing data_ vector
            const uint8_t *payload_start = data_.data() + 6;
            size_t payload_size = data_length;
            std::vector<uint8_t> payload(payload_start, payload_start + payload_size);

            switch (cfg)
            {
            case 0x01:
                if (data_[3] == 0x02)
                {
                    if (data_[6] == 0x0F)
                    {
                        ESP_LOGVV(TAG, "Sleep Mode successful.", cfg);
                        device_state_ = device_state_history;
                        packet_text_sensor_->publish_state("Sleep Mode successful.");
                    }
                    else
                    {
                        ESP_LOGVV(TAG, "Sleep Mode failed.", cfg);
                        delay(250);
                        device_state_ = DeviceState::STATE_SENSOR_SLEEP_MODE;
                        packet_text_sensor_->publish_state("Sleep Mode failed.");
                    }
                }
                if (data_[3] == 0x03)
                {
                    if (data_[6] == 0x01)
                    {
                        ESP_LOGVV(TAG, "Reset successful.", cfg);
                        device_state_ = device_state_history;
                        packet_text_sensor_->publish_state("Reset successful.");
                    }
                }
                if (data_[3] == 0x83)
                {
                    if (data_[6] == 0x01)
                    {
                        if (device_state_ == DeviceState::STATE_SENSOR_INIT)
                        {
                            device_state_ = DeviceState::STATE_SENSOR_CHG_MODE; // Transition if "Begin Done" after init
                            ESP_LOGI(TAG, "Device initialized, transitioning to STATE_SENSOR_CHG_MODE");
                            packet_text_sensor_->publish_state("Device initialized, transitioning to STATE_SENSOR_CHG_MODE");
                        }
                    }
                    else
                    {
                        ESP_LOGVV(TAG, "Begin failed.", cfg);
                        packet_text_sensor_->publish_state("Begin failed.");
                    }
                }
                break;
            case 0x02:
                if (data_[3] == 0x83)
                {
                    if (device_state_ == DeviceState::STATE_SENSOR_CHG_MODE)
                    {
                        device_state_ = DeviceState::STATE_SENSOR_SLEEP_MODE;
                        ESP_LOGI(TAG, "Work mode Changed, transitioning to STATE_SENSOR_SLEEP_MODE");
                        packet_text_sensor_->publish_state("Work mode Changed, transitioning to STATE_SENSOR_SLEEP_MODE");
                    }
                }
                if (data_[3] == 0xA8)
                {
                    if (data_[6] == 0x02)
                    {
                        ESP_LOGVV(TAG, "Work mode switch successful.", cfg);
                        packet_text_sensor_->publish_state("Work mode switch successful.");
                    }
                    else
                    {
                        ESP_LOGVV(TAG, "Work mode switch failed.", cfg);
                        packet_text_sensor_->publish_state("Work mode switch failed.");
                    }
                }
                break;
            case 0x80:
                if (data_[3] == 0x03)
                {
                    ESP_LOGVV(TAG, "Command 80 and Instruction 03 data avalible.", cfg);
                    process_movement_data(payload);
                }
                if (data_[3] == 0x05)
                {
                    ESP_LOGVV(TAG, "Sleep Composite Date Avalible.", cfg);
                    process_sleep_composite(payload);
                }
                if (data_[3] == 0x81)
                {
                    ESP_LOGVV(TAG, "Existing Information.", cfg);
                    if (presence_sensor_ != nullptr)
                    {
                        if (data_[6] == 0x01)
                        {
                            presence_sensor_->publish_state(true);
                        }
                        else
                        {
                            presence_sensor_->publish_state(false);
                        }
                    }
                }
                if (data_[3] == 0x82)
                {
                    ESP_LOGVV(TAG, "Existing Information.", cfg);
                    if (motion_sensor_ != nullptr)
                    {
                        switch (data_[6])
                        {
                        case 0x00:
                            motion_sensor_->publish_state("None");
                            break;
                        case 0x01:
                            motion_sensor_->publish_state("Still");
                            break;
                        case 0x02:
                            motion_sensor_->publish_state("Active");
                            break;
                        default:
                            motion_sensor_->publish_state("Read Error");
                            break;
                        }
                    }
                }
                break;
            case 0x81:
                if (data_[3] == 0x02)
                {
                    int d = data_[6];
                    ESP_LOGD(TAG, "Respiration data avalible.", cfg);
                    packet_text_sensor_->publish_state("Respiration data avalible.");
                    if (average_respiration_sensor_ != nullptr)
                        average_respiration_sensor_->publish_state(d);
                }
                break;
            case 0x84:
                if (data_[3] == 0x8D)
                {
                    ESP_LOGD(TAG, "Position data avalible.", cfg);
                    packet_text_sensor_->publish_state("SleepComposite data avalible.");
                    process_position_data(payload);
                }
                break;
            case 0x85:
                if (data_[3] == 0x02)
                {
                    int d = data_[6];
                    ESP_LOGD(TAG, "Heartbeat data avalible.", cfg);
                    packet_text_sensor_->publish_state("Heartbeat data avalible.");
                    if (average_heartbeat_sensor_ != nullptr)
                        average_heartbeat_sensor_->publish_state(d);
                }
                break;

            default:
                ESP_LOGW(TAG, "Unknown command received: 0x%02X", cfg);
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
                movement_sensor_->publish_state((float)data_[6]);
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
            if (payload.size() >= 4)
            {
                int presence = payload[0];
                int sleepState = payload[1];
                int averageRespiration = payload[2];
                int averageHeartbeat = payload[3];
                // int turnoverNumber = payload[4];
                // int largeBodyMove = payload[5];
                // int minorBodyMove = payload[6];
                // int apneaEvents = payload[7];

                // ESP_LOGD(TAG, "Publishing sleep composite data: presence=%d, sleepState=%d, averageRespiration=%d, averageHeartbeat=%d, turnoverNumber=%d, largeBodyMove=%d, minorBodyMove=%d, apneaEvents=%d",
                //          presence, sleepState, averageRespiration, averageHeartbeat, turnoverNumber, largeBodyMove, minorBodyMove, apneaEvents);

                ESP_LOGD(TAG, "Publishing sleep composite data: presence=%d, sleepState=%d, averageRespiration=%d, averageHeartbeat=%d",
                         presence, sleepState, averageRespiration, averageHeartbeat);
            }
            else
            {
                ESP_LOGW(TAG, "Insufficient data for sleep composite");
            }
        }

        void MMWaveComponent::send_command()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x02, 0xA8, 0x00, 0x01, 0x0f, 0x66, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Sent command: 0x00");
        }

        void MMWaveComponent::send_command_8081()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x80, 0x81, 0x00, 0x01, 0x0f, 0xBD, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Sent command: 0x80 0x81");
        }

        void MMWaveComponent::send_command_8082()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x80, 0x82, 0x00, 0x01, 0x0f, 0xBE, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Sent command: 0x80 0x82");
        }

        void MMWaveComponent::send_command_8083()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x80, 0x83, 0x00, 0x01, 0x0f, 0xBF, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Sent command: 0x80 0x83");
        }

        void MMWaveComponent::get_heartbeat()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x81, 0x82, 0x00, 0x01, 0x0f, 0xBF, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Sent command: 0x80 0x83");
        }

        void MMWaveComponent::get_respiration()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x85, 0x82, 0x00, 0x01, 0x0f, 0xC3, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Sent command: 0x80 0x83");
        }

        void MMWaveComponent::begin()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x01, 0x83, 0x00, 0x01, 0x0f, 0x40, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Sent Begin: 0x0f");
        }

        void MMWaveComponent::start_work_mode()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x02, 0xA8, 0x00, 0x01, 0x0F, 0x66, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Sent Get Work Mode: 0x80 0x0A");
        }

        void MMWaveComponent::sensor_restart()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x01, 0x03, 0x00, 0x01, 0x01, 0xB2, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Send Sensor Restart: 0x01 0x02, 0x0f");
        }

        void MMWaveComponent::sensor_led()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x01, 0x03, 0x00, 0x01, 0x01, 0xB2, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Send Sensor Restart: 0x01 0x02, 0x0f");
        }

        void MMWaveComponent::get_sleep_composite()
        {
            uint8_t cmdBuf[10] = {0x53, 0x59, 0x84, 0x8D, 0x00, 0x01, 0x0F, 0xCD, 0x54, 0x43};
            this->write_array(cmdBuf, sizeof(cmdBuf));
            ESP_LOGV(TAG, "Send Sleep Composite: 0x84 0x8D, 0x0f");
        }

void MMWaveComponent::send_sleep_mode_command() {
    // Switching work mode
    uint8_t cmdBufSleepConfig[10] = {0x53, 0x59, 0x02, 0xA8, 0x00, 0x01, 0x0F, 0x66, 0x54, 0x43};
    for (int i = 0; i < 4; ++i) {
        this->write_array(cmdBufSleepConfig, sizeof(cmdBufSleepConfig));
        ESP_LOGI(TAG, "Sent sleep config command.");
    }

    // Sleep Command
    uint8_t cmdBufSleep[10] = {0x53, 0x59, 0x01, 0x03, 0x00, 0x01, 0x01, 0xB2, 0x54, 0x43};
    for (int i = 0; i < 3; ++i) {
        this->write_array(cmdBufSleep, sizeof(cmdBufSleep));
        ESP_LOGI(TAG, "Sent sleep command.");
    }
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
