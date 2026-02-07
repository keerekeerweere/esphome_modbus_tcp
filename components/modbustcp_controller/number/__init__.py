import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import CONF_ADDRESS, CONF_ID, CONF_MAX_VALUE, CONF_MIN_VALUE, CONF_STEP

from .. import (
    MODBUS_WRITE_REGISTER_TYPE,
    SENSOR_VALUE_TYPE,
    ModbusItemBaseSchema,
    SensorItem,
    add_modbus_base_properties,
    modbus_calc_properties,
    modbustcp_controller_ns,
    validate_modbus_register,
)
from ..const import (
    CONF_BITMASK,
    CONF_FORCE_NEW_RANGE,
    CONF_MODBUSTCP_CONTROLLER_ID,
    CONF_REGISTER_COUNT,
    CONF_REGISTER_TYPE,
    CONF_SKIP_UPDATES,
    CONF_USE_WRITE_MULTIPLE,
    CONF_VALUE_TYPE,
    CONF_WRITE_LAMBDA,
)

DEPENDENCIES = ["modbustcp_controller"]
CODEOWNERS = ["@martgras"]

ModbusTCPNumber = modbustcp_controller_ns.class_(
    "ModbusTCPNumber", cg.Component, number.Number, SensorItem
)

CONFIG_SCHEMA = cv.All(
    number.number_schema(ModbusTCPNumber)
    .extend(cv.COMPONENT_SCHEMA)
    .extend(ModbusItemBaseSchema)
    .extend(
        {
            cv.Optional(CONF_REGISTER_TYPE): cv.enum(MODBUS_WRITE_REGISTER_TYPE),
            cv.Optional(CONF_VALUE_TYPE, default="U_WORD"): cv.enum(SENSOR_VALUE_TYPE),
            cv.Optional(CONF_REGISTER_COUNT, default=0): cv.positive_int,
            cv.Optional(CONF_USE_WRITE_MULTIPLE, default=False): cv.boolean,
            cv.Optional(CONF_WRITE_LAMBDA): cv.returning_lambda,
            cv.Optional(CONF_MIN_VALUE): cv.float_,
            cv.Optional(CONF_MAX_VALUE): cv.float_,
            cv.Optional(CONF_STEP): cv.float_,
        }
    ),
    validate_modbus_register,
)


async def to_code(config):
    byte_offset, reg_count = modbus_calc_properties(config)
    value_type = config[CONF_VALUE_TYPE]
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_REGISTER_TYPE],
        config[CONF_ADDRESS],
        byte_offset,
        config[CONF_BITMASK],
        value_type,
        reg_count,
        config[CONF_SKIP_UPDATES],
        config[CONF_FORCE_NEW_RANGE],
    )
    await cg.register_component(var, config)
    min_value = config.get(CONF_MIN_VALUE, 0.0)
    max_value = config.get(CONF_MAX_VALUE, 100.0)
    step = config.get(CONF_STEP, 1.0)
    await number.register_number(
        var, config, min_value=min_value, max_value=max_value, step=step
    )

    paren = await cg.get_variable(config[CONF_MODBUSTCP_CONTROLLER_ID])
    cg.add(var.set_parent(paren))
    cg.add(var.set_use_write_multiple(config[CONF_USE_WRITE_MULTIPLE]))
    cg.add(paren.add_sensor_item(var))

    if CONF_WRITE_LAMBDA in config:
        template_ = await cg.process_lambda(
            config[CONF_WRITE_LAMBDA],
            [
                (ModbusTCPNumber.operator("ptr"), "item"),
                (cg.float_, "x"),
                (cg.std_vector.template(cg.uint8).operator("ref"), "payload"),
            ],
            return_type=cg.optional.template(cg.float_),
        )
        cg.add(var.set_write_template(template_))

    await add_modbus_base_properties(var, config, ModbusTCPNumber)
