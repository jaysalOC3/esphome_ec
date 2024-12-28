#include "esphome/core/log.h"
#include "mmwave_sensor.h"
#include "DFRobot_HumanDetection.h"

namespace esphome {
namespace mmwave_sensor {

static const char *TAG = "mmwave_sensor.sensor";

#define BAUD_RATE 115200

void MMWaveSensor::setup() {
  ESP_LOGI(TAG, "Starting setup...");

  Serial.begin(BAUD_RATE);
#if defined(ESP32)
  Serial1.begin(BAUD_RATE, SERIAL_8N1, rx=21, tx=20);
#else
  Serial1.begin(BAUD_RATE);
#endif

  int retry_count = 0;
  const int max_retries = 3;

  while (hu.begin() != 0 && retry_count < max_retries) {
    ESP_LOGE(TAG, "Sensor initialization failed! Retrying...");
    retry_count++;
    delay(1000);
  }

  if (retry_count == max_retries) {
    ESP_LOGE(TAG, "Sensor initialization failed after %d retries. Halting execution.", max_retries);
    while (true); // Halt execution
  }

  ESP_LOGI(TAG, "Initialization successful");

  retry_count = 0;
  while (hu.configWorkMode(hu.eSleepMode) != 0 && retry_count < max_retries) {
    ESP_LOGE(TAG, "Failed to switch work mode! Retrying...");
    retry_count++;
    delay(1000);
  }

  if (retry_count == max_retries) {
    ESP_LOGE(TAG, "Failed to switch work mode after %d retries. Halting execution.", max_retries);
    while (true); // Halt execution
  }

  ESP_LOGI(TAG, "Work mode switch successful");

  hu.configLEDLight(hu.eHPLed, 1); 
  hu.sensorRet(); 
}

void MMWaveSensor::update() {
  ESP_LOGI(TAG, "Updating sensor data...");

  printHumanPresence(hu.smHumanData(hu.eHumanPresence));
  printHumanMovement(hu.smHumanData(hu.eHumanMovement));

  ESP_LOGI(TAG, "Body movement parameters: %d", hu.smHumanData(hu.eHumanMovingRange));
  ESP_LOGI(TAG, "Respiration rate: %d", hu.getBreatheValue());
  ESP_LOGI(TAG, "Heart rate: %d", hu.getHeartRate());
  ESP_LOGI(TAG, "-----------------------");
}

void MMWaveSensor::loop() {
  update();
  delay(1000);
}

void MMWaveSensor::dump_config(){
    ESP_LOGCONFIG(TAG, "Empty UART sensor");
}

void MMWaveSensor::printHumanPresence(int presence) {
  ESP_LOGI(TAG, "Existing information:");
  switch (presence) {
    case 0:
      ESP_LOGI(TAG, "No one is present");
      break;
    case 1:
      ESP_LOGI(TAG, "Someone is present");
      break;
    default:
      ESP_LOGI(TAG, "Read error");
  }
}

void MMWaveSensor::printHumanMovement(int movement) {
  ESP_LOGI(TAG, "Motion information:");
  switch (movement) {
    case 0:
      ESP_LOGI(TAG, "None");
      break;
    case 1:
      ESP_LOGI(TAG, "Still");
      break;
    case 2:
      ESP_LOGI(TAG, "Active");
      break;
    default:
      ESP_LOGI(TAG, "Read error");
  }
}

}  // namespace mmwave_sensor
}  // namespace esphome
