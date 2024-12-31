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
                ESP_LOGW(TAG, "Data available on UART; Loop()");
                // State machine variables
                static enum {
                    STATE_HEADER_START,
                    STATE_HEADER_END,
                    STATE_LENGTH_H,
                    STATE_LENGTH_L,
                    STATE_DATA,
                    STATE_CHECKSUM
                } state = STATE_HEADER_START;

                static uint8_t data[12];
                static uint8_t dataIndex = 0;
                static uint16_t dataLength = 0;

                while (this->available())
                {
                    uint8_t c = this->read();
                    ESP_LOGW(TAG, "Data: 0x%02X", c);
                    
                    switch (state)
                    {
                    case STATE_HEADER_START:
                        if (c == 0x53)
                        {
                            data[dataIndex++] = c;
                            state = STATE_HEADER_END;
                            ESP_LOGW(TAG, "Found Header Start: 0x53");
                        }
                        else
                        {
                            dataIndex = 0; // Reset if not header start
                            ESP_LOGE(TAG, "Error: Expecting Header Start: 0x53");
                            ESP_LOGE(TAG, "Got: 0x%02X", c);
                        }
                        break;
                    case STATE_HEADER_END:
                        if (c == 0x59)
                        {
                            data[dataIndex++] = c;
                            state = STATE_LENGTH_H;
                            ESP_LOGW(TAG, "Found Header End: 0x59");
                        }
                        else
                        {
                            state = STATE_HEADER_START; // Reset if not header end
                            dataIndex = 0;
                            ESP_LOGE(TAG, "Error: Expecting Header End: 0x59 (Resetting States)");
                        }
                        break;
                    case STATE_LENGTH_H:
                        data[dataIndex++] = c;
                        dataLength = c << 8;
                        state = STATE_LENGTH_L;
                        break;
                    case STATE_LENGTH_L:
                        data[dataIndex++] = c;
                        dataLength |= c;
                        state = STATE_DATA;
                        break;
                    case STATE_DATA:
                        data[dataIndex++] = c;
                        if (dataIndex >= dataLength + 6)
                        {
                            state = STATE_CHECKSUM;
                        }
                        break;
                    case STATE_CHECKSUM:
                        // ... (Logging code remains the same) ...

                        // Calculate checksum (optimized)
                        uint8_t calculatedChecksum = 0;
                        for (int i = 0; i < dataIndex - 1; i++)
                        {
                            calculatedChecksum += data[i];
                        }
                        calculatedChecksum &= 0xFF;

                        // ... (Logging and checksum comparison remain the same) ...

                        // Reset state and dataIndex
                        state = STATE_HEADER_START;
                        dataIndex = 0;
                        dataLength = 0;
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