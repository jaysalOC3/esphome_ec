#include "esphome/core/log.h"
#include "mmwave_component.h"

namespace esphome {
namespace dfrobot_mmwave_component {

static const char *TAG = "mmwave_component.component";

void MMWaveComponent::setup() {

}

void MMWaveComponent::loop() {

}

void MMWaveComponent::dump_config(){
    ESP_LOGCONFIG(TAG, "MMWave component");
}

}  // namespace mmwave_component
}  // namespace esphome