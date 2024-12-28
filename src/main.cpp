#include "esphome.h"

void setup() {
  // Initialize UART component
  auto *uart_component = new esphome::uart::UARTComponent();
  uart_component->set_baud_rate(115200);
  uart_component->set_tx_pin(21);
  uart_component->set_rx_pin(20);

  // Create MMWaveSensor with UART component
  auto *mmwave_sensor_mmwavesensor_id = new esphome::mmwave_sensor::MMWaveSensor(uart_component);

  // Add other setup code here
}

void loop() {
  // Add loop code here
}
