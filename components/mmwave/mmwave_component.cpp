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
                        // TODO: Implement your checksum calculation here
                        // Example (replace with your actual checksum logic):
                        uint8_t calculatedChecksum = 0;
                        for (int i = 0; i < dataIndex; i++)
                        {
                            calculatedChecksum += data[i];
                        }
                        if (checksum == calculatedChecksum)
                        {
                            // Check if the command is a response to a mode query (0x02, 0xA8)
                            if (header[0] == 0x02 && header[1] == 0xA8)
                            {
                                // Check the current mode from the data bytes (assuming it's at data[6])
                                // Replace '1' with your desired mode from eWorkMode
                                if (data[6] != 1)
                                {
                                    ESP_LOGW(TAG, "Sensor in incorrect mode: %d", data[6]);
                                    // TODO: Implement mode switching logic here (e.g., call configWorkMode)
                                }
                                else
                                {
                                    ESP_LOGD(TAG, "Sensor in correct mode");
                                }
                            }
                            // TODO: Process other valid packets as needed
                        }
                        else
                        {
                            ESP_LOGW(TAG, "Checksum error!");
                        }
                        state = STATE_HEADER_START; // Start looking for the next packet
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