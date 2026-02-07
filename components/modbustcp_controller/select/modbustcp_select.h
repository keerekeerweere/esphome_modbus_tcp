#pragma once

#include "esphome/components/modbustcp_controller/modbustcp_controller.h"
#include "esphome/components/select/select.h"
#include "esphome/core/component.h"

#include <string>
#include <vector>

namespace esphome {
namespace modbustcp_controller {

class ModbusTCPSelect : public Component, public select::Select, public SensorItem {
 public:
  ModbusTCPSelect(ModbusRegisterType register_type, uint16_t start_address, uint8_t offset, uint32_t bitmask,
                  SensorValueType value_type, int register_count, uint16_t skip_updates, bool force_new_range) {
    this->register_type = register_type;
    this->start_address = start_address;
    this->offset = offset;
    this->bitmask = bitmask;
    this->sensor_value_type = value_type;
    this->register_count = register_count;
    this->skip_updates = skip_updates;
    this->force_new_range = force_new_range;
  }

  void parse_and_publish(const std::vector<uint8_t> &data) override;
  void dump_config() override;
  void set_parent(ModbusTCPController *parent) { this->parent_ = parent; }
  void set_use_write_multiple(bool use_write_multiple) { this->use_write_multiple_ = use_write_multiple; }
  void set_select_options(const std::vector<std::string> &options);
  void set_option_values(const std::vector<int64_t> &values) { this->option_values_ = values; }

  using transform_func_t = std::function<optional<std::string>(ModbusTCPSelect *, std::string,
                                                             const std::vector<uint8_t> &)>;
  void set_template(transform_func_t &&f) { this->transform_func_ = f; }

  using write_transform_func_t = optional<std::string> (*)(ModbusTCPSelect *, std::string, std::vector<uint8_t> &);
  void set_write_template(write_transform_func_t f) { this->write_transform_func_ = f; }

 protected:
  void control(const std::string &value) override;

  ModbusTCPController *parent_{nullptr};
  bool use_write_multiple_{false};
  std::vector<std::string> options_{};
  std::vector<int64_t> option_values_{};
  optional<transform_func_t> transform_func_{nullopt};
  optional<write_transform_func_t> write_transform_func_{nullopt};
};

}  // namespace modbustcp_controller
}  // namespace esphome
