uart:
  id: uart_bus
  tx_pin: GPIO21
  rx_pin: GPIO20
  baud_rate: 115200  

mmwave:
  id: mmwave_component
  uart_id: uart_bus
  
  # Text sensors
  packet_text_sensor_id:
    name: "MMWave Packet Data"
  config_text_sensor_id:
    name: "MMWave Config Data"
  movement_sensor_id:
    name: "Movement Rate"
  position_text_sensor_id:
    name: "Position Data"
    
  # Binary sensors
  presence_sensor_id:
    name: "Presence"
    
  # Number sensors
  sleep_state_sensor_id:
    name: "Sleep State"
  average_respiration_sensor_id:
    name: "Average Respiration"
  average_heartbeat_sensor_id:
    name: "Average Heartbeat" 
  turnover_sensor_id:
    name: "Turnover"
  large_bodymove_sensor_id:
    name: "Large Body Movement"
  minor_bodymove_sensor_id:
    name: "Minor Body Movement"
  apnea_events_sensor_id:
    name: "Apnea Events"


button:
  - platform: template
    name: "MMWave Begin"
    id: mmwave_begin_btn 
    on_press: 
      then:
        - lambda: |-
            id(mmwave_component).begin();

  - platform: template
    name: "MMWave Start Work Mode"
    id: mmwave_control_btn 
    on_press: 
      then:
        - lambda: |-
            id(mmwave_component).start_work_mode();

  - platform: template
    name: "Send Sleep Command"
    id: sleep_command_btn 
    on_press: 
      then:
        - lambda: |-
            id(mmwave_component).send_sleep_mode_command();

  - platform: template
    name: "Sensor Restart"
    id: mmwave_sensor_restart_btn 
    on_press: 
      then:
        - lambda: |-
            id(mmwave_component).sensor_restart();

  - platform: template
    name: "Get Sleep Composite"
    id: sleep_composite_btn 
    on_press: 
      then:
        - lambda: |-
            id(mmwave_component).get_sleep_composite();
            
sensor:
number:
binary_sensor:
text_sensor:
  