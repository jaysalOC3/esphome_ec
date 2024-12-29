#pragma once
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome {
namespace mmwave_sensor {

class MMWaveSensor : public sensor::Sensor, public PollingComponent, public uart::UARTDevice {
 public:
  MMWaveSensor(uart::UARTComponent *uart) : UARTDevice(uart) {}

  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
  
  void set_presence_sensor(sensor::Sensor *presence_sensor) { presence_sensor_ = presence_sensor; }
  void set_movement_sensor(sensor::Sensor *movement_sensor) { movement_sensor_ = movement_sensor; }
  void set_movement_range_sensor(sensor::Sensor *range_sensor) { movement_range_sensor_ = range_sensor; }
  void set_breath_sensor(sensor::Sensor *breath_sensor) { breath_sensor_ = breath_sensor; }
  void set_heart_sensor(sensor::Sensor *heart_sensor) { heart_sensor_ = heart_sensor; }

 protected:
  bool begin();
  bool config_work_mode(uint8_t mode);
  uint8_t get_work_mode();
  bool config_led_light(uint8_t led, uint8_t state);
  uint8_t get_led_light_state(uint8_t led);
  bool sensor_reset();
  
  uint16_t get_human_data(uint8_t type);
  uint8_t get_heart_rate();
  uint8_t get_breathe_value();
  
  bool send_command(uint8_t control, uint8_t cmd, uint16_t len, uint8_t *send_data, uint8_t *ret_data);
  uint8_t calculate_checksum(uint8_t len, uint8_t *buf);

  sensor::Sensor *presence_sensor_{nullptr};
  sensor::Sensor *movement_sensor_{nullptr};
  sensor::Sensor *movement_range_sensor_{nullptr};
  sensor::Sensor *breath_sensor_{nullptr};
  sensor::Sensor *heart_sensor_{nullptr};
  
  static const uint8_t SLEEP_MODE = 0x02;
  static const uint8_t HP_LED = 0x04;
  
  static const uint8_t HUMAN_PRESENCE = 0x00;
  static const uint8_t HUMAN_MOVEMENT = 0x01;
  static const uint8_t HUMAN_RANGE = 0x02;
};

} // namespace mmwave_sensor
} // namespace esphome
