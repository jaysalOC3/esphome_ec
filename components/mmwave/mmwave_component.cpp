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

                        ESP_LOGD(TAG, "Data bytes:");
                        for (int i = 0; i < dataIndex; i++)
                        {
                            ESP_LOGD(TAG, "  data[%d]: 0x%02X", i, data[i]);
                        }

                        // Create a temporary buffer for checksum calculation
                        uint8_t checksumBuffer[14];    // Adjust size if needed
                        checksumBuffer[0] = header[0]; // Header byte 1
                        checksumBuffer[1] = header[1]; // Header byte 2
                        // Assuming data[0] is the command byte and data[1] is the length
                        memcpy(&checksumBuffer[2], data, dataIndex);

                        // Calculate the checksum
                        uint8_t calculatedChecksum = sumData(sizeof(checksumBuffer) - 1, checksumBuffer);

                        ESP_LOGD(TAG, "Received checksum: 0x%02X", checksum);
                        ESP_LOGD(TAG, "Calculated checksum: 0x%02X", calculatedChecksum);

                        if (checksum == calculatedChecksum)
                        {
                            ESP_LOGD(TAG, "Checksum PASSED!");
                            // ... rest of your code (mode checking, etc.) ...
                        }
                        else
                        {
                            ESP_LOGW(TAG, "Checksum error!");
                        }
                    }
                }
            }
        }

        void MMWaveComponent::dump_config()
        {
            ESP_LOGCONFIG(TAG, "MMWave Component:");
            ESP_LOGCONFIG(TAG, "  UART configured");
        }

        uint8_t MMWaveComponent::sumData(uint8_t len, uint8_t *buf)
        {
            uint16_t data = 0;
            uint8_t *_buf = buf;
            for (uint8_t i = 0; i < len; i++)
            {
                data += _buf[i];
            }
            return data & 0xff;
        }

    } // namespace mmwave_component
} // namespace esphome