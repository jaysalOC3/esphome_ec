#include "esphome/core/log.h"
#include "mmwave_sensor.h"
#include "DFRobot_HumanDetection.h"

namespace esphome {
namespace mmwave_sensor {

static const char *TAG = "mmwave_sensor.sensor";

#define BAUD_RATE 115200

DFRobot_HumanDetection hu(&Serial1);

void MMWaveSensor::setup() {
  Serial.begin(BAUD_RATE);
  
  #if defined(ESP32)
  Serial1.begin(BAUD_RATE, SERIAL_8N1, /*rx =*/21, /*tx =*/20);
  #else
  Serial1.begin(BAUD_RATE);
  #endif

  if (hu.begin() != 0) {
    Serial.println("Sensor initialization failed!");
    while (true); // Halt execution
  }
  Serial.println("Initialization successful");

  if (hu.configWorkMode(hu.eSleepMode) != 0) {
    Serial.println("Failed to switch work mode!");
    while (true); // Halt execution
  }
  Serial.println("Work mode switch successful");

  hu.configLEDLight(hu.eHPLed, 1); 
  hu.sensorRet(); 
}

void MMWaveSensor::update() {
  printHumanPresence(hu.smHumanData(hu.eHumanPresence));
  printHumanMovement(hu.smHumanData(hu.eHumanMovement));

  Serial.print("Body movement parameters: ");
  Serial.println(hu.smHumanData(hu.eHumanMovingRange));
  Serial.print("Respiration rate: ");
  Serial.println(hu.getBreatheValue());
  Serial.print("Heart rate: ");
  Serial.println(hu.getHeartRate());
  Serial.println("-----------------------");
}

void MMWaveSensor::loop() {
  update();
  delay(1000);
}

void MMWaveSensor::dump_config(){
    ESP_LOGCONFIG(TAG, "Empty UART sensor");
}

void MMWaveSensor::printHumanPresence(int presence) {
  Serial.print("Existing information:");
  switch (presence) {
    case 0:
      Serial.println("No one is present");
      break;
    case 1:
      Serial.println("Someone is present");
      break;
    default:
      Serial.println("Read error");
  }
}

void MMWaveSensor::printHumanMovement(int movement) {
  Serial.print("Motion information:");
  switch (movement) {
    case 0:
      Serial.println("None");
      break;
    case 1:
      Serial.println("Still");
      break;
    case 2:
      Serial.println("Active");
      break;
    default:
      Serial.println("Read error");
  }
}

}  // namespace mmwave_sensor
}  // namespace esphome
