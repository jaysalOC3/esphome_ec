#include "esphome/core/log.h"
#include "mmwave_sensor.h"

namespace esphome {
namespace mmwave_sensor {

static const char *TAG = "mmwave_sensor.sensor";

void MMWaveSensor::setup() {

}

void MMWaveSensor::update() {

}

void MMWaveSensor::loop() {

}

void MMWaveSensor::dump_config(){
    ESP_LOGCONFIG(TAG, "Empty UART sensor");
}

}  // namespace mmwave_sensor
}  // namespace esphome