# spal

# example configuration:

sensor:
  - platform: mmwave_sensor
    name: Empty UART sensor

uart:
  tx_pin: D0
  rx_pin: D1
  baud_rate: 9600

# Example configuration for mmWave sensor:

sensor:
  - platform: mmwave_sensor
    name: mmWave Human Detection Sensor

uart:
  tx_pin: 21
  rx_pin: 20
  baud_rate: 115200
