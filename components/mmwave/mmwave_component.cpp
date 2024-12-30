#include "esphome/core/log.h"
#include "dfrobot_sensor_component.h"

namespace esphome {
namespace dfrobot_sensor_component {

static const char *TAG = "mmwave_component.component";

void MMWaveComponent::setup() {

}

void MMWaveComponent::loop() {

}

void MMWaveComponent::dump_config(){
    ESP_LOGCONFIG(TAG, "MMWave component");
}

}  // namespace dfrobot_sensor_component
}  // namespace esphome