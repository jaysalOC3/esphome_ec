#include "esphome/core/log.h"
#include "mmwave_component.h"

namespace esphome
{
    namespace mmwave_ns
    {

        static const char *TAG = "mmwave";
        enum ReceiveState
        {
            RECEIVE_STATE_WAITING_HEADER,
            RECEIVE_STATE_HEADER,
            RECEIVE_STATE_CON,
            RECEIVE_STATE_CMD,
            RECEIVE_STATE_LEN_MSB,
            RECEIVE_STATE_LEN_LSB,
            RECEIVE_STATE_DATA,
            RECEIVE_STATE_CHECKSUM,
            RECEIVE_STATE_TRAILER1,
            RECEIVE_STATE_TRAILER2,
        };

        uint8_t MMWaveComponent::begin()
        {
            // Allow time for device initialization
            delay(1000);

            // Example command structure
            uint8_t command[] = {0x01, 0x83, 0x0F};
            size_t command_length = sizeof(command);

            // Write command to UART - don't compare return value
            this->write_array(command, command_length);
            this->flush(); // Ensure data is sent

            // Wait for response
            uint32_t start_time = millis();
            std::array<uint8_t, this->BUFFER_SIZE> response_buffer{};
            size_t bytes_read = 0;

            while (millis() - start_time < 1000)
            { // 1 second timeout
                if (this->available())
                {
                    while (this->available() && bytes_read < this->BUFFER_SIZE)
                    {
                        response_buffer[bytes_read++] = this->read();
                    }
                    break;
                }
                delay(10);
            }

            if (bytes_read == 0)
            {
                ESP_LOGE(TAG, "No response received");
                return 0;
            }

            // Process response
            ESP_LOGD(TAG, "Received %d bytes", bytes_read);
            for (size_t i = 0; i < bytes_read; i++)
            {
                ESP_LOGV(TAG, "Byte %d: 0x%02X", i, response_buffer[i]);
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

            std::array<uint8_t, 20> cmdBuf{};
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
                    this->flush();
                    timeStart1 = millis();
                }

                if (this->available() > 0)
                {
                    data = this->read();
                }

                if ((millis() - timeStart) > this->TIME_OUT)
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
                    // Add your state machine logic here
                    break;
                }
                delay(50);
            }
            return 0;
        }
        void MMWaveComponent::processReceivedData(const uint8_t *data, size_t length)
        {
            ESP_LOGD(TAG, "processReceivedData() - Received %u bytes", length);
            for (size_t i = 0; i < length; i++)
            {
                ESP_LOGV(TAG, "  Byte %d: 0x%02X", i, data[i]);
            }

            if (!this->verifyChecksum(data, length))
            {
                ESP_LOGE(TAG, "processReceivedData() - Checksum error!");
                return;
            }

            uint8_t con = data[2];
            uint8_t cmd = data[3];
            uint16_t len = (data[4] << 8) | data[5];

            ESP_LOGD(TAG, "processReceivedData() - con: 0x%02X, cmd: 0x%02X, len: %u", con, cmd, len);
            // ... your data handling logic ...
        }

        bool MMWaveComponent::verifyChecksum(const uint8_t *data, size_t length)
        {
            uint8_t calculatedChecksum = sumData(length - 2, data);
            uint8_t receivedChecksum = data[length - 2];

            if (calculatedChecksum == receivedChecksum)
            {
                return true;
            }
            else
            {
                ESP_LOGE(TAG, "verifyChecksum() - Checksum mismatch! Calculated: 0x%02X, Received: 0x%02X",
                         calculatedChecksum, receivedChecksum);
                return false;
            }
        }

        uint8_t MMWaveComponent::sumData(uint8_t len, const uint8_t *buf)
        {
            ESP_LOGD(TAG, "Summing data");
            uint16_t sum = 0;
            for (uint8_t i = 0; i < len; i++)
            {
                sum += buf[i];
            }
            return sum & 0xFF;
        }

        void MMWaveComponent::setup()
        {
            ESP_LOGD(TAG, "Setting up MMWave component...");
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
            {
                ESP_LOGD(TAG, "MMWave component loop running");
                last_log = millis();
            }

            const size_t MAX_BYTES_TO_READ = 32;
            uint8_t data[MAX_BYTES_TO_READ];
            size_t bytes_read = 0;

            while (this->available() && bytes_read < MAX_BYTES_TO_READ)
            {
                data[bytes_read++] = this->read();
            }

            if (bytes_read > 0)
            {
                ESP_LOGV(TAG, "Received %u bytes", bytes_read);
                for (size_t i = 0; i < bytes_read; ++i)
                {
                    ESP_LOGV(TAG, "  Byte %d: 0x%02X", i, data[i]);
                }

                uint8_t retData[this->BUFFER_SIZE];
                this->getData(0x01, 0x01, bytes_read, data, retData);
            }

            yield();
        }

        void MMWaveComponent::dump_config()
        {
            ESP_LOGCONFIG(TAG, "MMWave Component:");
            ESP_LOGCONFIG(TAG, "  UART configured");
        }

    } // namespace mmwave_ns
} // namespace esphome
