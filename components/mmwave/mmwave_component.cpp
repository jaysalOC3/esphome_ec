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

        uint8_t MMWaveComponent::getData(uint8_t con, uint8_t cmd, uint16_t len,
                                         uint8_t *senData, uint8_t *retData)
        {
            uint32_t timeStart = millis();
            uint32_t timeStart1 = 0;
            uint8_t data = 0, state = CMD_WHITE;
            uint16_t _len = 0;
            uint8_t count = 0;

            std::array<uint8_t, 20> cmdBuf;
            cmdBuf[0] = 0x53;
            cmdBuf[1] = 0x59;
            cmdBuf[2] = con;
            cmdBuf[3] = cmd;
            cmdBuf[4] = (len >> 8) & 0xff;
            cmdBuf[5] = len & 0xff;
            memcpy(&cmdBuf[6], senData, len);
            cmdBuf[6 + len] = sumData(6 + len, cmdBuf.data());
            cmdBuf[7 + len] = 0x54;
            cmdBuf[8 + len] = 0x43;

            while (true)
            {
                if ((millis() - timeStart1) > 500)
                {
                    while (this->available() > 0)
                    {
                        this->read();
                    }
                    this->write_array(cmdBuf.data(), 9 + len);
                    timeStart1 = millis();
                }

                if (this->available() > 0)
                {
                    data = this->read();
                }

                if ((millis() - timeStart) > TIME_OUT)
                {
                    ESP_LOGD(TAG, "Time out");
                    return 2;
                }

                switch (state)
                {
                case CMD_WHITE:
                    if (data == 0x53)
                    {
                        retData[0] = data;
                        state = CMD_HEAD;
                        count = 0;
                    }
                    break;
                case CMD_HEAD:
                    // ... (rest of the state machine logic - same as in your code) ...
                }
                delay(50);
            }
            delay(50);
            return 0;
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