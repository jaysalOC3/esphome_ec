#include "esphome/core/log.h"
#include "mmwave_component.h"

namespace esphome {
namespace mmwave_component {

static const char *TAG = "mmwave_component.component";

void MMWaveComponent::setup() {
ESP_LOGD(TAG, "Setting up MMWave component...");
    
    // Basic UART setup verification
    if (this->available()) {
        ESP_LOGD(TAG, "UART is available");
    } else {
        ESP_LOGW(TAG, "UART not available");
    }
}

void MMWaveComponent::loop() {
    static uint32_t last_log = 0;
    if (millis() - last_log > 5000) {  // Log every 5 seconds
        ESP_LOGD(TAG, "MMWave component loop running");
        last_log = millis();
    }
    
    if (this->available()) {
        ESP_LOGV(TAG, "Data available on UART");
        // Just read and log available data for now
        while (this->available()) {
            uint8_t c = this->read();
            ESP_LOGV(TAG, "Received byte: 0x%02X", c);
        }
    }
}

void MMWaveComponent::dump_config(){
    ESP_LOGCONFIG(TAG, "MMWave Component:");
    ESP_LOGCONFIG(TAG, "  UART configured");
}

}  // namespace mmwave_component
}  // namespace esphome