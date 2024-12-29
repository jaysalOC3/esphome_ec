#include "mmwave_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

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
  this->request_data(HUMAN_PRESENCE);
  this->request_data(HUMAN_MOVEMENT);
  this->request_data(HUMAN_RANGE);
  this->request_data(BREATH_RATE);
  this->request_data(HEART_RATE);
}

void MMWaveSensor::request_data(uint8_t type) {
    using namespace esphome::mmwave_sensor;
    if(this->pending_request_ == 0){
        uint8_t ret_data[10];
        switch (type) {
            case HUMAN_PRESENCE:
                this->send_command(0x02, 0x05, 0, nullptr, ret_data);
                break;
            case HUMAN_MOVEMENT:
                this->send_command(0x02, 0x06, 0, nullptr, ret_data);
                break;
            case HUMAN_RANGE:
                this->send_command(0x02, 0x07, 0, nullptr, ret_data);
                break;
            case BREATH_RATE:
                this->send_command(0x02, 0x09, 0, nullptr, ret_data);
                break;
            case HEART_RATE:
                this->send_command(0x02, 0x08, 0, nullptr, ret_data);
                break;
        }
        this->pending_request_ = type + 1;
        this->last_request_time_ = millis();
    } else if(millis() - this->last_request_time_ > 200) {
        ESP_LOGW(TAG, "Previous request timed out, requesting new data");
        this->pending_request_ = 0;
    }

}

void MMWaveSensor::process_response() {
    using namespace esphome::mmwave_sensor;
    uint8_t buf[256];
    uint8_t bytes_read = this->read_array(buf, this->available());
    if (bytes_read < 6) return;

    //Basic Checksum Validation
    uint8_t received_checksum = buf[bytes_read - 1];
    uint8_t calculated_checksum = calculate_checksum(bytes_read - 1, buf);
    if(received_checksum != calculated_checksum){
        ESP_LOGW(TAG, "Checksum mismatch, discarding data");
        return;
    }

  // Process data based on command
  uint8_t cmd = buf[2];

    switch (cmd) {
        case 0x05: //Presence
            if(this->pending_request_ == HUMAN_PRESENCE + 1){
                this->presence_sensor_->publish_state((buf[3] << 8) | buf[4]);
                this->pending_request_ = 0;
            }
            break;
        case 0x06: //Movement
            if(this->pending_request_ == HUMAN_MOVEMENT + 1){
                this->movement_sensor_->publish_state((buf[3] << 8) | buf[4]);
                this->pending_request_ = 0;
            }
            break;
        case 0x07: //Range
            if(this->pending_request_ == HUMAN_RANGE + 1){
                this->movement_range_sensor_->publish_state((buf[3] << 8) | buf[4]);
                this->pending_request_ = 0;
            }
            break;
        case 0x08: //Heart Rate
            if(this->pending_request_ == HEART_RATE + 1){
                this->heart_sensor_->publish_state(buf[3]);
                this->pending_request_ = 0;
            }
            break;
        case 0x09: //Breath Rate
            if(this->pending_request_ == BREATH_RATE + 1){
                this->breath_sensor_->publish_state(buf[3]);
                this->pending_request_ = 0;
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

bool MMWaveSensor::send_command(uint8_t control, uint8_t cmd, uint16_t len, uint8_t *send_data, uint8_t *ret_data) { // Corrected: No line break here
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

  // Wait for response - Non Blocking
  return true;
}

uint8_t MMWaveSensor::calculate_checksum(uint8_t len, uint8_t *buf) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < len; i++) {
    sum += buf[i];
  }
  return sum;
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
