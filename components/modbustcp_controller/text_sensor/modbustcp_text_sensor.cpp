#include "modbustcp_text_sensor.h"
#include "esphome/core/log.h"

namespace esphome {
namespace modbustcp_controller {

static const char *const TAG = "modbustcp_controller.text_sensor";

void ModbusTCPTextSensor::dump_config() { LOG_TEXT_SENSOR(TAG, "Modbus TCP Controller Text Sensor", this); }

void ModbusTCPTextSensor::parse_and_publish(const std::vector<uint8_t> &data) {
  size_t max_len = data.size();
  size_t register_size = this->get_register_size();
  if (register_size > 0 && register_size < max_len) {
    max_len = register_size;
  }

  if (this->offset >= max_len) {
    ESP_LOGW(TAG, "Offset %u out of range for payload size %u", this->offset, (unsigned) max_len);
    return;
  }

  size_t len = max_len - this->offset;
  std::string value;

  if (this->raw_encode_) {
    std::vector<uint8_t> slice(data.begin() + this->offset, data.begin() + this->offset + len);
    value = format_hex_pretty(slice);
  } else {
    value.reserve(len);
    for (size_t i = 0; i < len; i++) {
      char c = static_cast<char>(data[this->offset + i]);
      if (c == '\0') {
        break;
      }
      value.push_back(c);
    }
  }

  if (this->transform_func_.has_value()) {
    auto val = (*this->transform_func_)(this, value, data);
    if (val.has_value()) {
      ESP_LOGV(TAG, "Value overwritten by lambda");
      value = val.value();
    }
  }

  this->publish_state(value);
}

}  // namespace modbustcp_controller
}  // namespace esphome
