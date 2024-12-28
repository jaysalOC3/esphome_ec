#include "mmwave_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mmwave_sensor {

static const char *TAG = "mmwave_sensor.sensor";

void MMWaveSensor::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MMWaveSensor...");
  // Add any additional setup code here
}

void MMWaveSensor::loop() {
  // Add any additional loop code here
}

void MMWaveSensor::update() {
  ESP_LOGD(TAG, "Updating MMWaveSensor...");
  // Add any additional update code here
}

void MMWaveSensor::dump_config() {
  ESP_LOGCONFIG(TAG, "MMWave custom sensor");
  // Add any additional dump config code here
}

}  // namespace mmwave_sensor
}  // namespace esphome
