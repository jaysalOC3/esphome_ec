# spal configuration for mmWave sensor:

---
logger:
  level: DEBUG # Helpful for debugging

external_components:
  - source: github://jaysalOC3/esphome_ec@setup-begin
    components:
      - mmwave
    refresh: 5s

uart:
  id: uart_bus
  tx_pin: GPIO21
  rx_pin: GPIO20
  baud_rate: 115200  

mmwave:
  id: mmwave_component
  uart_id: uart_bus
