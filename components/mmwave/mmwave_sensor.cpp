// mmwave_sensor.cpp
#include "mmwave_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace mmwave_sensor {

static const char *const TAG = "mmwave_sensor.sensor";

void MMWaveSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MMWaveSensor...");
  if (!this->is_sensor_ready()) {
    ESP_LOGE(TAG, "MMWaveSensor setup failed: Sensor not ready");
    this->mark_failed();
    return;
  }
  
  // Initialize random number generator
  this->gen.seed(random_uint32());
  ESP_LOGD(TAG, "Random number generator initialized");
}

void MMWaveSensor::loop() {
  if (this->is_sensor_ready()) {
    this->read_sensor_data();
  } else {
    ESP_LOGW(TAG, "MMWaveSensor loop: Sensor not ready");
  }
}

void MMWaveSensor::update() {
  ESP_LOGD(TAG, "Updating MMWaveSensor...");
  float random_value = this->dist(this->gen);
  ESP_LOGD(TAG, "Publishing random value: %.1f", random_value);
  this->publish_state(random_value);
}

void MMWaveSensor::read_sensor_data() {
  // This is now handled in update() for the random value demo
}

void MMWaveSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "MMWave custom sensor");
  LOG_SENSOR("  ", "MMWave Sensor", this);
  ESP_LOGCONFIG(TAG, "  Update interval: %.1fs", this->get_update_interval() / 1000.0f);
}

}  // namespace mmwave_sensor
}  // namespace esphome
