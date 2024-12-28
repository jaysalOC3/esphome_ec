# spal configuration for mmWave sensor:

logger:
  level: DEBUG # Helpful for debugging

uart:
  id: mmwave_uart
  tx_pin: 21
  rx_pin: 20
  baud_rate: 115200

external_components:
  - source: ./components/mmwave  # Path to your mmwave component

mmwave:
  id: my_mmwave_component
  uart_id: mmwave_uart
  work_mode: sleep # or falling

sensor:
  - platform: mmwave
    name: "mmwave sensor"
