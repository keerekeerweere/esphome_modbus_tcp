import esphome.codegen as cg
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import CONF_ADDRESS, CONF_ID

from .. import (
    MODBUS_REGISTER_TYPE,
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
    CONF_RAW_ENCODE,
    CONF_REGISTER_COUNT,
    CONF_REGISTER_TYPE,
    CONF_SKIP_UPDATES,
)

DEPENDENCIES = ["modbustcp_controller"]
CODEOWNERS = ["@martgras"]

ModbusTCPTextSensor = modbustcp_controller_ns.class_(
    "ModbusTCPTextSensor", cg.Component, text_sensor.TextSensor, SensorItem
)

CONFIG_SCHEMA = cv.All(
    text_sensor.text_sensor_schema(ModbusTCPTextSensor)
    .extend(cv.COMPONENT_SCHEMA)
    .extend(ModbusItemBaseSchema)
    .extend(
        {
            cv.Optional(CONF_REGISTER_TYPE): cv.enum(MODBUS_REGISTER_TYPE),
            cv.Optional(CONF_REGISTER_COUNT, default=1): cv.positive_int,
            cv.Optional(CONF_RAW_ENCODE, default=False): cv.boolean,
        }
    ),
    validate_modbus_register,
)


async def to_code(config):
    byte_offset, reg_count = modbus_calc_properties(config)
    var = cg.new_Pvariable(
        config[CONF_ID],
        config[CONF_REGISTER_TYPE],
        config[CONF_ADDRESS],
        byte_offset,
        config[CONF_BITMASK],
        reg_count,
        config[CONF_SKIP_UPDATES],
        config[CONF_FORCE_NEW_RANGE],
    )
    await cg.register_component(var, config)
    await text_sensor.register_text_sensor(var, config)

    paren = await cg.get_variable(config[CONF_MODBUSTCP_CONTROLLER_ID])
    cg.add(paren.add_sensor_item(var))
    cg.add(var.set_raw_encode(config[CONF_RAW_ENCODE]))

    await add_modbus_base_properties(
        var, config, ModbusTCPTextSensor, cg.std_string, cg.std_string
    )
