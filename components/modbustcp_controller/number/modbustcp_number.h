#pragma once

#include "esphome/components/modbustcp_controller/modbustcp_controller.h"
#include "esphome/components/number/number.h"
#include "esphome/core/component.h"

#include <vector>

namespace esphome {
namespace modbustcp_controller {

class ModbusTCPNumber : public Component, public number::Number, public SensorItem {
 public:
  ModbusTCPNumber(ModbusRegisterType register_type, uint16_t start_address, uint8_t offset, uint32_t bitmask,
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

  using transform_func_t = std::function<optional<float>(ModbusTCPNumber *, float, const std::vector<uint8_t> &)>;
  void set_template(transform_func_t &&f) { this->transform_func_ = f; }

  using write_transform_func_t = optional<float> (*)(ModbusTCPNumber *, float, std::vector<uint8_t> &);
  void set_write_template(write_transform_func_t f) { this->write_transform_func_ = f; }

 protected:
  void control(float value) override;

  ModbusTCPController *parent_{nullptr};
  bool use_write_multiple_{false};
  optional<transform_func_t> transform_func_{nullopt};
  optional<write_transform_func_t> write_transform_func_{nullopt};
};

}  // namespace modbustcp_controller
}  // namespace esphome
