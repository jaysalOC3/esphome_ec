#include "esphome/core/log.h"
#include "mmwave_sensor.h"

namespace esphome {
namespace mmwave_sensor {

static const char *TAG = "mmwave_sensor.sensor";

void EmptyUARTSensor::setup() {

}

void EmptyUARTSensor::update() {

}

void EmptyUARTSensor::loop() {

}

void EmptyUARTSensor::dump_config(){
    ESP_LOGCONFIG(TAG, "Empty UART sensor");
}

}  // namespace mmwave_sensor
}  // namespace esphome