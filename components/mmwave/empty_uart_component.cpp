#include "esphome/core/log.h"
#include "dfrobot_sensor_component.h"

namespace esphome {
namespace dfrobot_sensor_component {

static const char *TAG = "dfrobot_sensor_component.component";

void EmptyUARTComponent::setup() {

}

void EmptyUARTComponent::loop() {

}

void EmptyUARTComponent::dump_config(){
    ESP_LOGCONFIG(TAG, "Empty UART component");
}

}  // namespace dfrobot_sensor_component
}  // namespace esphome