#include "esphome/core/log.h"
#include "mmwave_component.h"

namespace esphome
{
    namespace mmwave_ns
    {

        static const char *TAG = "mmwave";

        void MMWaveComponent::setup()
        {
            ESP_LOGD(TAG, "Setting up MMWave component... Setup()");
        }

        void MMWaveComponent::loop()
        {
            if (this->available())
            {
                // State machine variables
                static enum {
                    STATE_HEADER_START,
                    STATE_HEADER_END,
                    STATE_DATA,
                    STATE_CHECKSUM
                } state = STATE_HEADER_START;

                static uint8_t header[2]; // Assuming a 2-byte header
                static uint8_t data[10];  // Assuming 10 bytes of data (adjust as needed)
                static uint8_t dataIndex = 0;
                static uint8_t checksum;

                while (this->available())
                {
                    uint8_t c = this->read();

                    switch (state)
                    {
                    case STATE_HEADER_START:
                        if (c == 0x53)
                        { // First header byte
                            header[0] = c;
                            state = STATE_HEADER_END;
                        }
                        break;
                    case STATE_HEADER_END:
                        if (c == 0x59)
                        { // Second header byte
                            header[1] = c;
                            state = STATE_DATA;
                            dataIndex = 0;
                        }
                        else
                        {
                            state = STATE_HEADER_START; // Invalid header, restart
                        }
                        break;
                    case STATE_DATA:
                        data[dataIndex++] = c;
                        if (dataIndex >= sizeof(data))
                        {
                            state = STATE_CHECKSUM;
                        }
                        break;
                    case STATE_CHECKSUM:
                        checksum = c;

                        // --- Debugging ---
                        ESP_LOGD(TAG, "Data bytes:");
                        for (int i = 0; i < dataIndex; i++)
                        {
                            ESP_LOGD(TAG, "  data[%d]: 0x%02X", i, data[i]);
                        }
                        // --- End Debugging ---

                        // Example checksum calculation (replace with your actual logic)
                        uint8_t calculatedChecksum = 0;
                        for (int i = 0; i < dataIndex; i++)
                        {
                            calculatedChecksum += data[i];
                        }

                        ESP_LOGD(TAG, "Received checksum: 0x%02X", checksum);
                        ESP_LOGD(TAG, "Calculated checksum: 0x%02X", calculatedChecksum);

                        if (checksum == calculatedChecksum)
                        {
                            ESP_LOGW(TAG, "Checksum PASSED!");
                        }
                        else
                        {
                            ESP_LOGW(TAG, "Checksum error!");
                        }
                        break;
                    }
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