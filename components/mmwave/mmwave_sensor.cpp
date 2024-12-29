#include "mmwave_sensor.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace mmwave_sensor {

// ... (setup, loop, other functions)

void MMWaveSensor::update() {
  this->request_data(HUMAN_PRESENCE);
  this->request_data(HUMAN_MOVEMENT);
  this->request_data(HUMAN_RANGE);
  this->request_data(BREATH_RATE); // Now correct
  this->request_data(HEART_RATE); // Now correct
}

void MMWaveSensor::request_data(uint8_t type) {
    if(this->pending_request_ == 0){
        uint8_t ret_data[10];
        switch (type) {
            case HUMAN_PRESENCE:
                this->send_command(0x02, 0x05, 0, nullptr, ret_data);
                break;
            case HUMAN_MOVEMENT:
                this->send_command(0x02, 0x06, 0, nullptr, ret_data);
                break;
            case HUMAN_RANGE:
                this->send_command(0x02, 0x07, 0, nullptr, ret_data);
                break;
            case BREATH_RATE:
                this->send_command(0x02, 0x09, 0, nullptr, ret_data);
                break;
            case HEART_RATE:
                this->send_command(0x02, 0x08, 0, nullptr, ret_data);
                break;
        }
        this->pending_request_ = type + 1; //Offsetting to ensure it is not 0
        this->last_request_time_ = millis();
    } else if(millis() - this->last_request_time_ > 200) {
        ESP_LOGW(TAG, "Previous request timed out, requesting new data");
        this->pending_request_ = 0;
    }

}

void MMWaveSensor::process_response() {
    uint8_t buf[256];
    uint8_t bytes_read = this->read_array(buf, this->available());
    if (bytes_read < 6) return;

    //Basic Checksum Validation
    uint8_t received_checksum = buf[bytes_read - 1];
    uint8_t calculated_checksum = calculate_checksum(bytes_read - 1, buf);
    if(received_checksum != calculated_checksum){
        ESP_LOGW(TAG, "Checksum mismatch, discarding data");
        return;
    }

  // Process data based on command
  uint8_t cmd = buf[2];

    switch (cmd) {
        case 0x05: //Presence
            if(this->pending_request_ == HUMAN_PRESENCE + 1){
                this->presence_sensor_->publish_state((buf[3] << 8) | buf[4]);
                this->pending_request_ = 0;
            }
            break;
        case 0x06: //Movement
            if(this->pending_request_ == HUMAN_MOVEMENT + 1){
                this->movement_sensor_->publish_state((buf[3] << 8) | buf[4]);
                this->pending_request_ = 0;
            }
            break;
        case 0x07: //Range
            if(this->pending_request_ == HUMAN_RANGE + 1){
                this->movement_range_sensor_->publish_state((buf[3] << 8) | buf[4]);
                this->pending_request_ = 0;
            }
            break;
        case 0x08: //Heart Rate
            if(this->pending_request_ == HEART_RATE + 1){
                this->heart_sensor_->publish_state(buf[3]);
                this->pending_request_ = 0;
            }
            break;
        case 0x09: //Breath Rate
            if(this->pending_request_ == BREATH_RATE + 1){
                this->breath_sensor_->publish_state(buf[3]);
                this->pending_request_ = 0;
            }
            break;
        default:
            ESP_LOGW(TAG, "Unknown command received: 0x%02X", cmd);
            break; // Important: Added break here!
    } // Closing switch
}

// ... (rest of the functions)

}  // namespace mmwave_sensor
}  // namespace esphome
