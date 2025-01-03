#include "somose.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace somose {

static const char *const TAG = "somose";

static const uint8_t SOMOSE_MOISTURE_CMD[] = {0x76};
static const uint8_t SOMOSE_TEMPERATURE_CMD[] = {0x74};

static const uint8_t SOMOSE_DEFAULT_DELAY = 20;     // ms, for initialization and temperature measurement
static const uint8_t SOMOSE_READ_DELAY = 200;       // ms, time to wait for conversion result
static const uint8_t SOMOSE_SOFTRESET_DELAY = 120;  // ms

static const uint8_t SOMOSE_ATTEMPTS = 6;  // safety margin, normally 3 attempts are enough: 3*30=90ms
static const uint8_t SOMOSE_INIT_ATTEMPTS = 20;

void SoMoSeComponent::setup() {
  ESP_LOGI(TAG, "somose initialization");
  this->update();
}

void SoMoSeComponent::restart_read_moisture_() {
  if (this->read_count_ == SOMOSE_ATTEMPTS) {
    this->read_count_ = 0;
    this->status_set_error("Measurements reading timed-out!");
    return;
  }
  this->read_count_++;
  this->set_timeout(SOMOSE_READ_DELAY, [this]() { this->read_data_moisture_(); });
}

void SoMoSeComponent::read_data_temperature_() {
  uint8_t data[1];

  if (this->read_count_ > 1)
    ESP_LOGI(TAG, "Read attempt %d at %ums", this->read_count_, (unsigned) (millis() - this->start_time_));

  if (this->read(data, 1) != i2c::ERROR_OK) {
    this->status_set_warning("SoMoSe read failed, retrying soon");
    this->restart_read_temperature_();
    return;
  }

  if (this->read_count_ > 1)
    ESP_LOGI(TAG, "Success at %ums", (unsigned) (millis() - this->start_time_));

  uint32_t raw_temperature = uint32_t(data[0]);

  if (this->temperature_sensor_ != nullptr) {
    // float temperature = ((200.0f * (float) raw_temperature) / 1048576.0f) - 50.0f;
    float temperature = (float) raw_temperature;
    this->temperature_sensor_->publish_state(temperature);
  }

  this->status_clear_warning();
  this->read_count_ = 0;
}

void SoMoSeComponent::restart_read_temperature_() {
  if (this->read_count_ == SOMOSE_ATTEMPTS) {
    this->read_count_ = 0;
    this->status_set_error("Measurements reading timed-out!");
    return;
  }
  this->read_count_++;
  this->set_timeout(SOMOSE_READ_DELAY, [this]() { this->read_data_temperature_(); });
}

void SoMoSeComponent::read_data_moisture_() {
  uint8_t dataMoisture[2];

  if (this->read_count_ > 1)
    ESP_LOGI(TAG, "Read attempt %d at %ums", this->read_count_, (unsigned) (millis() - this->start_time_));

  if (this->read(dataMoisture, 2) != i2c::ERROR_OK) {
    this->status_set_warning("SoMoSe read failed, retrying soon");
    this->restart_read_moisture_();
    return;
  }

  if (this->read_count_ > 1)
    ESP_LOGI(TAG, "Success at %ums", (unsigned) (millis() - this->start_time_));

  uint32_t raw_moisture = uint32_t(dataMoisture[1]);

  if (this->moisture_sensor_ != nullptr) {
    float moisture;
    if (raw_moisture == 0) {  // unrealistic value
      moisture = NAN;
    } else {
      // moisture = (float) raw_moisture * 100.0f / 256.0f;
      moisture = (float) raw_moisture * 100.0f / 256;
    }
    if (std::isnan(moisture)) {
      ESP_LOGW(TAG, "Invalid moisture! Sensor reported 0%% Hum");
    }
    this->moisture_sensor_->publish_state(moisture);
  }

  this->status_clear_warning();
  this->read_count_ = 0;
}

void SoMoSeComponent::update() {
  if (this->read_count_ != 0)
    return;
  this->start_time_ = millis();

  if (this->write(SOMOSE_MOISTURE_CMD, sizeof(SOMOSE_MOISTURE_CMD)) != i2c::ERROR_OK) {
    this->status_set_warning("Communication with SoMoSe failed!");
    return;
  }
  this->restart_read_moisture_();

  ESP_LOGI(TAG, "next temp...");
  delayMicroseconds(200);

  if (this->write(SOMOSE_TEMPERATURE_CMD, sizeof(SOMOSE_TEMPERATURE_CMD)) != i2c::ERROR_OK) {
    this->status_set_warning("Communication with SoMoSe failed!");
    return;
  }
  this->restart_read_temperature_();
}

float SoMoSeComponent::get_setup_priority() const { return setup_priority::DATA; }

void SoMoSeComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "somose:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Communication with somose failed!");
  }
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Moisture", this->moisture_sensor_);
}

}  // namespace somose
}  // namespace esphome
