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

                static enum {
                    STATE_HEADER_START,
                    STATE_HEADER_END,
                    STATE_LENGTH_H,
                    STATE_LENGTH_L,
                    STATE_DATA,
                    STATE_CHECKSUM,
                    STATE_TAIL_1,
                    STATE_TAIL_2,
                } state = STATE_HEADER_START;

                // Use a larger buffer or dynamic allocation if needed
                static uint8_t data[20];
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
                            dataIndex = 0;
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
                            state = STATE_HEADER_START;
                            dataIndex = 0;
                            ESP_LOGE(TAG, "Error: Expecting Header End: 0x59");
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
                        { // +6 for header, con, cmd, lenH, lenL
                            state = STATE_CHECKSUM;
                        }
                        break;
                    case STATE_CHECKSUM:
                    {
                        data[dataIndex++] = c;

                        // Calculate checksum
                        uint8_t calculatedChecksum = 0;
                        for (int i = 0; i < dataIndex - 1; i++)
                        {
                            calculatedChecksum += data[i];
                        }
                        calculatedChecksum &= 0xFF;

                        if (calculatedChecksum == c)
                        {
                            ESP_LOGW(TAG, "Checksum OK");
                            state = STATE_TAIL_1;
                        }
                        else
                        {
                            ESP_LOGE(TAG, "Checksum Error: Expected 0x%02X, Got 0x%02X",
                                     calculatedChecksum, c);
                            state = STATE_HEADER_START;
                            dataIndex = 0;
                            dataLength = 0;
                        }
                        break;
                    }
                    case STATE_TAIL_1:
                        if (c == 0x54)
                        {
                            data[dataIndex++] = c;
                            state = STATE_TAIL_2;
                        }
                        else
                        {
                            ESP_LOGE(TAG, "Error: Expecting Tail Byte 1 (0x54)");
                            state = STATE_HEADER_START;
                            dataIndex = 0;
                            dataLength = 0;
                        }
                        break;
                    case STATE_TAIL_2:
                        if (c == 0x43)
                        {
                            data[dataIndex++] = c;
                            ESP_LOGW(TAG, "Complete message received");

                            // Process the received data here
                            processSensorData(data, dataIndex);

                            state = STATE_HEADER_START;
                            dataIndex = 0;
                            dataLength = 0;
                        }
                        else
                        {
                            ESP_LOGE(TAG, "Error: Expecting Tail Byte 2 (0x43)");
                            state = STATE_HEADER_START;
                            dataIndex = 0;
                            dataLength = 0;
                        }
                        break;
                    }
                }
            }
        }

        // Example data processing function (replace with your actual logic)
        void MMWaveComponent::processSensorData(uint8_t *data, uint8_t dataLen)
        {
            ESP_LOGI(TAG, "Processing sensor data:");
            for (int i = 0; i < dataLen; i++)
            {
                ESP_LOGI(TAG, "  data[%d] = 0x%02X", i, data[i]);
            }
            // ... decode and use the sensor data ...
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