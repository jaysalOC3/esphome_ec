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
  - platform: mmwave.heart_rate
    name: "Heart Rate"
    id: heart_rate_sensor
  - platform: mmwave.breathe_value
    name: "Breathe Value"
    id: breathe_value_sensor
  - platform: mmwave.presence_distance
    name: "Presence Distance"
    id: presence_distance_sensor
  - platform: mmwave.movement
    name: "Movement"
    id: movement_sensor
  - platform: mmwave.moving_range
    name: "Moving Range"
    id: moving_range_sensor
  - platform: mmwave.sleep_quality
    name: "Sleep Quality"
    id: sleep_quality_sensor
  - platform: mmwave.wake_duration
    name: "Wake Duration"
    id: wake_duration_sensor
  - platform: mmwave.light_sleep
    name: "Light Sleep Duration"
    id: light_sleep_sensor
  - platform: mmwave.deep_sleep
    name: "Deep Sleep Duration"
    id: deep_sleep_sensor
  - platform: mmwave.sleep_disturbances
    name: "Sleep Disturbances"
    id: sleep_disturbances_sensor
  - platform: mmwave.sleep_quality_rating
    name: "Sleep Quality Rating"
    id: sleep_quality_rating_sensor
  - platform: mmwave.abnormal_struggle
    name: "Abnormal Struggle"
    id: abnormal_struggle_sensor
  - platform: mmwave.unattended_time
    name: "Unattended Time"
    id: unattended_time_sensor
  - platform: mmwave.unattended_state
    name: "Unattended State"
    id: unattended_state_sensor
  - platform: mmwave.fall_state
    name: "Fall State"
    id: fall_state_sensor
  - platform: mmwave.fall_break_height
    name: "Fall Break Height"
    id: fall_break_height_sensor
  - platform: mmwave.static_residency_time
    name: "Static Residency Time"
    id: static_residency_time_sensor
  - platform: mmwave.accumulated_height_duration
    name: "Accumulated Height Duration"
    id: accumulated_height_duration_sensor
  - platform: mmwave.seated_horizontal_distance
    name: "Seated Horizontal Distance"
    id: seated_horizontal_distance_sensor
  - platform: mmwave.motion_horizontal_distance
    name: "Motion Horizontal Distance"
    id: motion_horizontal_distance_sensor
