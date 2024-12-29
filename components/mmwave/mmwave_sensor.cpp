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
  // Main processing is done in update()
}

void MMWaveSensor::update() {
  if (this->presence_sensor_ != nullptr) {
    uint16_t presence = this->get_human_data(HUMAN_PRESENCE);
    this->presence_sensor_->publish_state(presence);
  }
  
  if (this->movement_sensor_ != nullptr) {
    uint16_t movement = this->get_human_data(HUMAN_MOVEMENT);
    this->movement_sensor_->publish_state(movement);
  }
  
  if (this->movement_range_sensor_ != nullptr) {
    uint16_t range = this->get_human_data(HUMAN_RANGE);
    this->movement_range_sensor_->publish_state(range);
  }
  
  if (this->breath_sensor_ != nullptr) {
    uint8_t breath = this->get_breathe_value();
    this->breath_sensor_->publish_state(breath);
  }
  
  if (this->heart_sensor_ != nullptr) {
    uint8_t heart = this->get_heart_rate();
    this->heart_sensor_->publish_state(heart);
  }
}

void MMWaveSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "MMWave Sensor:");
  LOG_SENSOR("  ", "Presence", this->presence_sensor_);
  LOG_SENSOR("  ", "Movement", this->movement_sensor_);
  LOG_SENSOR("  ", "Movement Range", this->movement_range_sensor_);
  LOG_SENSOR("  ", "Breath Rate", this->breath_sensor_);
  LOG_SENSOR("  ", "Heart Rate", this->heart_sensor_);
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
  buf[0] = 0xAA;  // Header
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
  
  // Wait for response
  uint32_t start_time = millis();
  while (available() < 6) {
    if (millis() - start_time > 1000) {
      ESP_LOGE(TAG, "Command timeout");
      return false;
    }
    delay(1);
  }
  
  // Read response
  if (ret_data != nullptr) {
    read_array(ret_data, 6);
  }
  
  return true;
}

uint8_t MMWaveSensor::calculate_checksum(uint8_t len, uint8_t *buf) {
  uint8_t sum = 0;
  for (uint8_t i = 0; i < len; i++) {
    sum += buf[i];
  }
  return sum;
}

} // namespace mmwave_sensor
} // namespace esphome
