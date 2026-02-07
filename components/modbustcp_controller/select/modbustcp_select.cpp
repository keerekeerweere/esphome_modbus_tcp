#include "modbustcp_select.h"
#include "esphome/core/log.h"

namespace esphome {
namespace modbustcp_controller {

static const char *const TAG = "modbustcp_controller.select";

void ModbusTCPSelect::dump_config() { LOG_SELECT(TAG, "Modbus TCP Controller Select", this); }

void ModbusTCPSelect::set_select_options(const std::vector<std::string> &options) {
  this->options_ = options;
}

void ModbusTCPSelect::parse_and_publish(const std::vector<uint8_t> &data) {
  int64_t raw_value = payload_to_number(data, this->sensor_value_type, this->offset, this->bitmask);
  int32_t index = -1;

  if (!this->option_values_.empty()) {
    for (size_t i = 0; i < this->option_values_.size(); i++) {
      if (this->option_values_[i] == raw_value) {
        index = static_cast<int32_t>(i);
        break;
      }
    }
  } else {
    index = static_cast<int32_t>(raw_value);
  }

  if (index < 0 || static_cast<size_t>(index) >= this->options_.size()) {
    ESP_LOGW(TAG, "Select value %d out of range (options=%u)", index, (unsigned) this->options_.size());
    return;
  }

  std::string value = this->options_[index];

  if (this->transform_func_.has_value()) {
    auto val = (*this->transform_func_)(this, value, data);
    if (val.has_value()) {
      ESP_LOGV(TAG, "Value overwritten by lambda");
      value = val.value();
    }
  }

  this->publish_state(value);
}

void ModbusTCPSelect::control(const std::string &value) {
  ModbusCommandItem cmd;
  std::vector<uint8_t> raw_data;

  std::string selected = value;
  if (this->write_transform_func_.has_value()) {
    auto val = (*this->write_transform_func_)(this, selected, raw_data);
    if (val.has_value()) {
      selected = val.value();
    } else if (raw_data.empty()) {
      ESP_LOGV(TAG, "Communication handled by lambda - exiting control");
      return;
    }
  }

  int32_t index = -1;
  for (size_t i = 0; i < this->options_.size(); i++) {
    if (this->options_[i] == selected) {
      index = static_cast<int32_t>(i);
      break;
    }
  }

  if (index < 0) {
    ESP_LOGW(TAG, "Unknown option '%s'", selected.c_str());
    return;
  }

  int64_t raw_value = index;
  if (!this->option_values_.empty()) {
    raw_value = this->option_values_[index];
  }

  if (!raw_data.empty()) {
    ESP_LOGV(TAG, "Modbus TCP Select write raw: %s", format_hex_pretty(raw_data).c_str());
    cmd = ModbusCommandItem::create_custom_command(
        this->parent_, raw_data,
        [this](ModbusRegisterType register_type, uint16_t start_address, const std::vector<uint8_t> &data) {
          this->parent_->on_write_register_response(this->register_type, this->start_address, data);
        });
  } else {
    std::vector<uint16_t> payload_words;
    number_to_payload(payload_words, raw_value, this->sensor_value_type);
    if (payload_words.empty()) {
      ESP_LOGW(TAG, "No payload generated for value_type %d", (int) this->sensor_value_type);
      return;
    }
    uint16_t write_address = this->start_address + this->offset / 2;

    if (this->use_write_multiple_ || payload_words.size() > 1) {
      cmd = ModbusCommandItem::create_write_multiple_command(this->parent_, write_address, payload_words.size(),
                                                             payload_words);
    } else {
      cmd = ModbusCommandItem::create_write_single_command(this->parent_, write_address, payload_words[0]);
    }
  }

  this->parent_->queue_command(cmd);
  this->publish_state(selected);
}

}  // namespace modbustcp_controller
}  // namespace esphome
