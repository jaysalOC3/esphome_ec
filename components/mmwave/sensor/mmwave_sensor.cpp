#include "mmwave_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace mmwave_sensor {

static const char *TAG = "mmwave_sensor.sensor";

void MMWaveSensor::setup() {}

void MMWaveSensor::loop() {}

void MMWaveSensor::update() {}

void MMWaveSensor::dump_config() { ESP_LOGCONFIG(TAG, "MMWave custom sensor"); }

}  // namespace mmwave_sensor
}  // namespace esphome