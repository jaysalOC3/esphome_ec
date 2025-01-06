esp32:
  board: esp32-s3-devkitc-1
  framework:
    type: esp-idf

# Enable logging
logger:
  level: VERY_VERBOSE 

debug:
  update_interval: 5s 

text_sensor:
  - platform: debug
    device:
      name: "Device Info" 
    reset_reason:
      name: "Reset Reason"

external_components:
  - source: github://jaysalOC3/esphome_ec@dev
      - mmwave
    refresh: 0s

uart:
  id: uart_bus
  tx_pin: GPIO21
  rx_pin: GPIO20
  baud_rate: 115200  

mmwave:
  id: mmwave_component
  uart_id: uart_bus
  