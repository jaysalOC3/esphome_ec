// mmwave_component.h
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

namespace esphome
{
    namespace mmwave_ns
    {

        class MMWaveComponent : public uart::UARTDevice, public Component
        {
        public:
            MMWaveComponent() = default;

            void setup() override;
            void loop() override;
            void dump_config() override;

            // Add setter for UART parent
            void set_uart_parent(uart::UARTComponent *parent)
            {
                uart::UARTDevice::set_uart_parent(parent);
            }

        private:
            static const size_t BUFFER_SIZE = 10;
            static const uint8_t CMD_WHITE = 0;
            static const uint8_t CMD_HEAD = 1;
            static const uint32_t TIME_OUT = 1000;
            // Receive state machine variables
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
            ReceiveState receiveState = RECEIVE_STATE_WAITING_HEADER;
            uint8_t receiveBuffer[BUFFER_SIZE];
            size_t receiveBufferIndex = 0;
            uint16_t expectedDataLength = 0;

            // Helper functions
            uint8_t begin();
            void processReceivedData(const uint8_t *data, size_t length);
            bool verifyChecksum(const uint8_t *data, size_t length);
            uint8_t sumData(uint8_t len, const uint8_t *buf);
            uint8_t getData(uint8_t con, uint8_t cmd, uint16_t len,
                            uint8_t *senData, uint8_t *retData);
        };

    } // namespace mmwave_ns
} // namespace esphome
