#include "modbustcp_number.h"
#include "esphome/core/log.h"

namespace esphome {
namespace modbustcp_controller {

static const char *const TAG = "modbustcp_controller.number";

void ModbusTCPNumber::dump_config() { LOG_NUMBER(TAG, "Modbus TCP Controller Number", this); }

void ModbusTCPNumber::parse_and_publish(const std::vector<uint8_t> &data) {
  float result = payload_to_float(data, *this);

  if (this->transform_func_.has_value()) {
    auto val = (*this->transform_func_)(this, result, data);
    if (val.has_value()) {
      ESP_LOGV(TAG, "Value overwritten by lambda");
      result = val.value();
    }
  }

  ESP_LOGD(TAG, "Number new state: %.02f", result);
  this->publish_state(result);
}

void ModbusTCPNumber::control(float value) {
  ModbusCommandItem cmd;
  std::vector<uint8_t> raw_data;

  if (this->write_transform_func_.has_value()) {
    auto val = (*this->write_transform_func_)(this, value, raw_data);
    if (val.has_value()) {
      value = val.value();
    } else if (raw_data.empty()) {
      ESP_LOGV(TAG, "Communication handled by lambda - exiting control");
      return;
    }
  }

  if (!raw_data.empty()) {
    ESP_LOGV(TAG, "Modbus TCP Number write raw: %s", format_hex_pretty(raw_data).c_str());
    cmd = ModbusCommandItem::create_custom_command(
        this->parent_, raw_data,
        [this](ModbusRegisterType register_type, uint16_t start_address, const std::vector<uint8_t> &data) {
          this->parent_->on_write_register_response(this->register_type, this->start_address, data);
        });
  } else {
    auto payload_words = float_to_payload(value, this->sensor_value_type);
    if (payload_words.empty()) {
      ESP_LOGW(TAG, "No payload generated for value_type %d", (int) this->sensor_value_type);
      return;
    }
    uint16_t write_address = this->start_address + this->offset / 2;

    ESP_LOGV(TAG, "write_state '%s': new value = %.02f type = %d address = %X offset = %x", this->get_name().c_str(),
             value, (int) this->register_type, this->start_address, this->offset);

    if (this->use_write_multiple_ || payload_words.size() > 1) {
      cmd = ModbusCommandItem::create_write_multiple_command(this->parent_, write_address, payload_words.size(),
                                                             payload_words);
    } else {
      cmd = ModbusCommandItem::create_write_single_command(this->parent_, write_address, payload_words[0]);
    }
  }

  this->parent_->queue_command(cmd);
  this->publish_state(value);
}

}  // namespace modbustcp_controller
}  // namespace esphome
