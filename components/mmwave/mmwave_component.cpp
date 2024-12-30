#include "esphome/core/log.h"
#include "mmwave_component.h"

namespace esphome
{
    namespace mmwave_ns
    {

        static const char *TAG = "mmwave";

        uint8_t MMWaveComponent::begin(void)
        {
            delay(10000); // Startup initialization wait time (might be necessary)
            uint8_t data = 0x0f;
            std::array<uint8_t, 10> buf;
            if (getData(0x01, 0x83, 1, &data, buf.data()) == 0)
            {
                return 0;
            }
            return 1;
        }

        void MMWaveComponent::setup()
        {
            ESP_LOGD(TAG, "Setting up MMWave component...");

            // Basic UART setup verification
            if (this->begin() == 0)
            {
                ESP_LOGD(TAG, "MMWave sensor initialized successfully");
            }
            else
            {
                ESP_LOGE(TAG, "MMWave sensor initialization failed!");
            }
        }

        void MMWaveComponent::loop()
        {
            static uint32_t last_log = 0;
            if (millis() - last_log > 5000)
            { // Log every 5 seconds
                ESP_LOGD(TAG, "MMWave component loop running");
                last_log = millis();
            }

            if (this->available())
            {
                ESP_LOGV(TAG, "Data available on UART");
                // Just read and log available data for now
                while (this->available())
                {
                    uint8_t c = this->read();
                    ESP_LOGV(TAG, "Received byte: 0x%02X", c);
                }
            }
        }

        void MMWaveComponent::dump_config()
        {
            ESP_LOGCONFIG(TAG, "MMWave Component:");
            ESP_LOGCONFIG(TAG, "  UART configured");
        }

    } // namespace mmwave_component
} // namespace esphome