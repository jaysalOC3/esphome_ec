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
                static uint8_t data[12];   // Assuming a 12-byte data packet
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
                            data[dataIndex++] = c; // Save the byte
                            state = STATE_HEADER_END;
                        }
                        break;
                    case STATE_HEADER_END:
                        if (c == 0x59)
                        { // Second header byte
                            data[dataIndex++] = c; // Save the byte
                            state = STATE_DATA;
                        }
                        else
                        {
                            state = STATE_HEADER_START; // Invalid header, restart
                            dataIndex = 0;              // Reset dataIndex
                        }
                        break;
                    case STATE_DATA:
                        data[dataIndex++] = c;
                        // Assuming fixed data length of 10 bytes (including command, length, etc.)
                        if (dataIndex >= 12)
                        { // Check if all 12 bytes are received
                            state = STATE_CHECKSUM;
                        }
                        break;
                    case STATE_CHECKSUM:
                        ESP_LOGD(TAG, "Data bytes:");
                        for (int i = 0; i < dataIndex; i++)
                        {
                            ESP_LOGD(TAG, "  data[%d]: 0x%02X", i, data[i]);
                        }

                        // Calculate checksum using the received data (excluding the last byte)
                        uint8_t calculatedChecksum = sumData(dataIndex - 1, data);

                        ESP_LOGD(TAG, "Received checksum: 0x%02X", c);
                        ESP_LOGD(TAG, "Calculated checksum: 0x%02X", calculatedChecksum);

                        if (c == calculatedChecksum)
                        {
                            ESP_LOGD(TAG, "Checksum PASSED!");
                            // ... rest of your code (mode checking, etc.) ...
                        }
                        else
                        {
                            ESP_LOGW(TAG, "Checksum error!");
                        }

                        // Reset state and dataIndex for the next packet
                        state = STATE_HEADER_START;
                        dataIndex = 0;
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