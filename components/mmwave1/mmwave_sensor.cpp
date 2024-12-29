#include "mmwave_sensor.h" // Include the header!
#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include <cstring> // Make sure to include this for memcpy


namespace esphome {
namespace mmwave_sensor {

static const char *const TAG = "mmwave_sensor.sensor";

void MMWaveSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MMWave Sensor...");

  if (!this->begin()) {
    ESP_LOGE(TAG, "Sensor initialization failed");
    this->mark_failed();
    return;
  }

  if (!this->config_work_mode(SLEEP_MODE)) {
    ESP_LOGE(TAG, "Failed to set sleep mode");
    this->mark_failed();
    return;
  }

  this->config_led_light(HP_LED, 1);
  this->sensor_reset();

  ESP_LOGD(TAG, "Sensor initialized successfully");
}

void MMWaveSensor::loop() {
  if (this->available() >= 6) {
    this->process_response();
  }
}

void MMWaveSensor::update() {
<<<<<<< HEAD
  if (this->current_request_type_ == RequestType::NONE) {
    this->current_request_type_ = this->next_request_type_;
    this->next_request_type_ = RequestType::NONE;
  }
  this->request_data(static_cast<uint8_t>(this->current_request_type_));
=======
  // Add the implementation for the update method
>>>>>>> 5444e4c (Added DFRobot_HumanDetection)
}

void MMWaveSensor::request_data(uint8_t type) {
    uint8_t ret_data[10];
    switch (static_cast<RequestType>(type)) { // Cast to RequestType in switch
        case RequestType::HUMAN_PRESENCE:
            this->send_command(0x02, 0x05, 0, nullptr, ret_data);
            break;
        case RequestType::HUMAN_MOVEMENT:
            this->send_command(0x02, 0x06, 0, nullptr, ret_data);
            break;
        case RequestType::HUMAN_RANGE:
            this->send_command(0x02, 0x07, 0, nullptr, ret_data);
            break;
        case RequestType::BREATH_RATE:
            this->send_command(0x02, 0x09, 0, nullptr, ret_data);
            break;
        case RequestType::HEART_RATE:
            this->send_command(0x02, 0x08, 0, nullptr, ret_data);
            break;
        default:
            ESP_LOGW(TAG, "Invalid request type: %d", type);
            return;
    }
    this->last_request_time_ = millis();
}

void MMWaveSensor::process_response() {
    using namespace esphome::mmwave_sensor;
    uint8_t buf[256];
    uint8_t bytes_read = this->read_array(buf, this->available());
    if (bytes_read < 6) return;

    //Basic Checksum Validation
    uint8_t received_checksum = buf[bytes_read - 1];
    uint8_t calculated_checksum = calculate_checksum(bytes_read - 1, buf);
    if (received_checksum != calculated_checksum) {
        ESP_LOGW(TAG, "Checksum mismatch, discarding data");
        return;
    }

    // Process data based on command
    uint8_t cmd = buf[2];

    switch (cmd) {
        case 0x05: //Presence
            if (this->current_request_type_ == RequestType::HUMAN_PRESENCE) {
                this->presence_sensor_->publish_state((buf[3] << 8) | buf[4]);
                this->current_request_type_ = RequestType::NONE;
            }
            break;
        case 0x06: //Movement
            if (this->current_request_type_ == RequestType::HUMAN_MOVEMENT) {
                this->movement_sensor_->publish_state((buf[3] << 8) | buf[4]);
                this->current_request_type_ = RequestType::NONE;
            }
            break;
        case 0x07: //Range
            if (this->current_request_type_ == RequestType::HUMAN_RANGE) {
                this->movement_range_sensor_->publish_state((buf[3] << 8) | buf[4]);
                this->current_request_type_ = RequestType::NONE;
            }
            break;
        case 0x08: //Heart Rate
            if (this->current_request_type_ == RequestType::HEART_RATE) {
                this->heart_sensor_->publish_state(buf[3]);
                this->current_request_type_ = RequestType::NONE;
            }
            break;
        case 0x09: //Breath Rate
            if (this->current_request_type_ == RequestType::BREATH_RATE) {
                this->breath_sensor_->publish_state(buf[3]);
                this->current_request_type_ = RequestType::NONE;
            }
            break;
        default:
            ESP_LOGW(TAG, "Unknown command received: 0x%02X", cmd);
            break;
    }
}

bool MMWaveSensor::begin() {
  uint8_t ret_data[10];
  return send_command(0x01, 0x01, 0, nullptr, ret_data);
}

bool MMWaveSensor::config_work_mode(uint8_t mode) {
  uint8_t send_data[1] = {mode};
  uint8_t ret_data[10];
  return send_command(0x01, 0x02, 1, send_data, ret_data);
}

uint8_t MMWaveSensor::get_work_mode() {
  uint8_t ret_data[10];
  if (send_command(0x02, 0x02, 0, nullptr, ret_data)) {
    return ret_data[0];
  }
  return 0;
}

bool MMWaveSensor::config_led_light(uint8_t led, uint8_t state) {
  uint8_t send_data[2] = {led, state};
  uint8_t ret_data[10];
  return send_command(0x01, 0x03, 2, send_data, ret_data);
}

uint8_t MMWaveSensor::get_led_light_state(uint8_t led) {
    uint8_t ret_data[10];
    if(send_command(0x02, 0x03, 0, nullptr, ret_data)){
        return ret_data[led];
    }
    return 0;
}

bool MMWaveSensor::sensor_reset() {
  uint8_t ret_data[10];
  return send_command(0x01, 0x04, 0, nullptr, ret_data);
}

uint16_t MMWaveSensor::get_human_data(uint8_t type) {
  uint8_t ret_data[10];
  if (send_command(0x02, 0x05 + type, 0, nullptr, ret_data)) {
    return (ret_data[0] << 8) | ret_data[1];
  }
  return 0;
}

uint8_t MMWaveSensor::get_heart_rate() {
  uint8_t ret_data[10];
  if (send_command(0x02, 0x08, 0, nullptr, ret_data)) {
    return ret_data[0];
  }
  return 0;
}

uint8_t MMWaveSensor::get_breathe_value() {
  uint8_t ret_data[10];
  if (send_command(0x02, 0x09, 0, nullptr, ret_data)) {
    return ret_data[0];
  }
  return 0;
}

bool MMWaveSensor::send_command(uint8_t control, uint8_t cmd, uint16_t len, uint8_t *send_data, uint8_t *ret_data) {
    uint8_t buf[256];
    buf[0] = 0xAA;
    buf[1] = control;
    buf[2] = cmd;
    buf[3] = (len >> 8) & 0xFF;
    buf[4] = len & 0xFF;

    if (len > 0 && send_data != nullptr) {
        memcpy(&buf[5], send_data, len);
    }

    uint8_t checksum = calculate_checksum(len + 5, buf);
    buf[5 + len] = checksum;

    this->write_array(buf, len + 6);

    // Wait for response with timeout
    unsigned long start_time = millis();
    const unsigned long timeout = 200; // 200ms timeout - Adjust if needed
    uint8_t bytes_read = 0;

    while (millis() - start_time < timeout) {
        if (this->available() > 0) { // Check if any data is available
            bytes_read = this->read_array(ret_data, this->available());

            if (bytes_read >= 6) { // Check for minimum response size (adjust as needed)
                ESP_LOGD(TAG, "Response Received (Bytes Read: %d)", bytes_read);
                for (int i = 0; i < bytes_read; i++) {
                    ESP_LOGD(TAG, "Byte %d: 0x%02X", i, ret_data[i]);
                }
                return true; // Received a response
            }else{
              ESP_LOGD(TAG, "Partial Response Received (Bytes Read: %d), waiting for more", bytes_read);
              for (int i = 0; i < bytes_read; i++) {
                    ESP_LOGD(TAG, "Byte %d: 0x%02X", i, ret_data[i]);
                }
            }
        }
        delay(1); // Small delay to avoid busy-waiting
    }

    ESP_LOGW(TAG, "Timeout waiting for response");
    return false; // Timeout occurred
}

uint8_t MMWaveSensor::calculate_checksum(uint8_t len, uint8_t *buf) {
    uint8_t sum = 0;
    for (uint8_t i = 0; i < len; i++) {
        sum += buf[i];
    }
    return sum;
}

void MMWaveSensor::set_movement_range_sensor(sensor::Sensor *range_sensor) {
    this->movement_range_sensor_ = range_sensor;
}

void MMWaveSensor::set_breath_sensor(sensor::Sensor *breath_sensor) {
    this->breath_sensor_ = breath_sensor;
}

void MMWaveSensor::set_heart_sensor(sensor::Sensor *heart_sensor) {
    this->heart_sensor_ = heart_sensor;
}

void MMWaveSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "MMWave Sensor:");
  LOG_SENSOR("  ", "Presence", this->presence_sensor_);
  LOG_SENSOR("  ", "Movement", this->movement_sensor_);
  LOG_SENSOR("  ", "Movement Range", this->movement_range_sensor_);
  LOG_SENSOR("  ", "Breath Rate", this->breath_sensor_);
  LOG_SENSOR("  ", "Heart Rate", this->heart_sensor_);
}

}  // namespace mmwave_sensor
}  // namespace esphome
