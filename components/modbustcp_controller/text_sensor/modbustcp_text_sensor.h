#pragma once

#include "esphome/components/modbustcp_controller/modbustcp_controller.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/core/component.h"

#include <string>
#include <vector>

namespace esphome {
namespace modbustcp_controller {

class ModbusTCPTextSensor : public Component, public text_sensor::TextSensor, public SensorItem {
 public:
  ModbusTCPTextSensor(ModbusRegisterType register_type, uint16_t start_address, uint8_t offset, uint32_t bitmask,
                      int register_count, uint16_t skip_updates, bool force_new_range) {
    this->register_type = register_type;
    this->start_address = start_address;
    this->offset = offset;
    this->bitmask = bitmask;
    this->register_count = register_count;
    this->skip_updates = skip_updates;
    this->force_new_range = force_new_range;
  }

  void parse_and_publish(const std::vector<uint8_t> &data) override;
  void dump_config() override;
  void set_raw_encode(bool raw_encode) { this->raw_encode_ = raw_encode; }

  using transform_func_t = std::function<optional<std::string>(ModbusTCPTextSensor *, std::string,
                                                             const std::vector<uint8_t> &)>;
  void set_template(transform_func_t &&f) { this->transform_func_ = f; }

 protected:
  optional<transform_func_t> transform_func_{nullopt};
  bool raw_encode_{false};
};

}  // namespace modbustcp_controller
}  // namespace esphome
