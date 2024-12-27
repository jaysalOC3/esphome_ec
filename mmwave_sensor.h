#ifndef MM_WAVE_SENSOR_H
#define MM_WAVE_SENSOR_H

#include "esphome.h"
#include "DFRobot_HumanDetection.h"

class MmWaveSensor : public Component, public UARTDevice, public Sensor {
 public:
  DFRobot_HumanDetection sensor;

  Sensor *presence_sensor = new Sensor();
  Sensor *motion_sensor = new Sensor();
  Sensor *movement_param_sensor = new Sensor();
  Sensor *respiration_rate_sensor = new Sensor();
  Sensor *heart_rate_sensor = new Sensor();

  MmWaveSensor(UARTComponent *uart)
      : Component(), UARTDevice(uart), sensor(uart) {}  // Changed from get_stream() to uart directly

  void setup() override {
    ESP_LOGI("MmWaveSensor", "Initializing sensor...");
    if (sensor.begin() != 0) {
      ESP_LOGE("MmWaveSensor", "Sensor initialization failed!");
      return;
    }
    ESP_LOGI("MmWaveSensor", "Sensor initialization successful!");

    ESP_LOGI("MmWaveSensor", "Configuring work mode...");
    if (sensor.configWorkMode(sensor.eSleepMode) != 0) {
      ESP_LOGE("MmWaveSensor", "Failed to configure work mode!");
    } else {
      ESP_LOGI("MmWaveSensor", "Work mode configured successfully.");
    }
  }

  void loop() override {  // Changed from update() to loop()
    read_uart_data();
  }

  void read_uart_data() {
    while (available()) {
      uint8_t data = read();
      // Process the UART data and convert it into sensor readings
      // This is a placeholder for the actual implementation
    }
  }

  void update_sensors() {  // This method can be used to update sensor values if needed
    uint16_t presence = sensor.smHumanData(sensor.eHumanPresence);
    presence_sensor->publish_state(presence);

    uint16_t motion = sensor.smHumanData(sensor.eHumanMovement);
    motion_sensor->publish_state(motion);

    uint16_t movement_param = sensor.smHumanData(sensor.eHumanMovingRange);
    movement_param_sensor->publish_state(movement_param);

    uint8_t respiration_rate = sensor.getBreatheValue();
    respiration_rate_sensor->publish_state(respiration_rate);

    uint8_t heart_rate = sensor.getHeartRate();
    heart_rate_sensor->publish_state(heart_rate);
  }
};

#endif // MM_WAVE_SENSOR_H
