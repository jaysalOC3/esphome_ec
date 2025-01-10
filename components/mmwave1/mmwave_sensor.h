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

  void set_presence_sensor(sensor::Sensor *presence_sensor);
  void set_movement_sensor(sensor::Sensor *movement_sensor);
  void set_movement_range_sensor(sensor::Sensor *range_sensor);
  void set_breath_sensor(sensor::Sensor *breath_sensor);
  void set_heart_sensor(sensor::Sensor *heart_sensor);

  void request_data(uint8_t type);
  void process_response();

 protected:
  bool begin();
  bool config_work_mode(uint8_t mode);
  uint8_t start_work_mode();
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

 private:
  enum class RequestType : uint8_t { // Use enum class and specify underlying type
    NONE = 0,
    HUMAN_PRESENCE = 1,
    HUMAN_MOVEMENT = 2,
    HUMAN_RANGE = 3,
    BREATH_RATE = 4,
    HEART_RATE = 5
  };

  RequestType current_request_type_ = RequestType::NONE;
  RequestType next_request_type_ = RequestType::NONE;
  unsigned long last_request_time_ = 0;
};

}  // namespace mmwave_sensor
}  // namespace esphome
