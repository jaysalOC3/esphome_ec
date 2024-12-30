#include "esphome/core/log.h"
#include "mmwave_component.h"

namespace esphome {

static const char *TAG = "mmwave_component.component";

void MMWaveComponent::setup() {

}

void MMWaveComponent::loop() {

}

void MMWaveComponent::dump_config(){
    ESP_LOGCONFIG(TAG, "MMWave component");
}

}  // namespace esphome