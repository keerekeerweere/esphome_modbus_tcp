import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import CONF_ADDRESS, CONF_ID, CONF_OPTIONS

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
    CONF_OPTION_VALUES,
    CONF_OPTIONS_MAP,
    CONF_REGISTER_COUNT,
    CONF_REGISTER_TYPE,
    CONF_SKIP_UPDATES,
    CONF_USE_WRITE_MULTIPLE,
    CONF_VALUE_TYPE,
    CONF_WRITE_LAMBDA,
)

DEPENDENCIES = ["modbustcp_controller"]
CODEOWNERS = ["@martgras"]

ModbusTCPSelect = modbustcp_controller_ns.class_(
    "ModbusTCPSelect", cg.Component, select.Select, SensorItem
)


def _preprocess_optionsmap(config):
    if CONF_OPTIONS_MAP in config:
        if CONF_OPTIONS in config or CONF_OPTION_VALUES in config:
            raise cv.Invalid("optionsmap can't be used together with options/option_values")
        options_map = config[CONF_OPTIONS_MAP]
        if not options_map:
            raise cv.Invalid("optionsmap must not be empty")
        # Accept either a dict or a list of one-key dicts
        if isinstance(options_map, list):
            options = []
            values = []
            for item in options_map:
                if not isinstance(item, dict) or len(item) != 1:
                    raise cv.Invalid("optionsmap list entries must be one-key maps")
                (k, v), = item.items()
                options.append(str(k))
                values.append(int(v))
        else:
            options = list(options_map.keys())
            values = list(options_map.values())
        config = dict(config)
        config[CONF_OPTIONS] = options
        config[CONF_OPTION_VALUES] = values
    return config


def _preprocess_option_values_alias(config):
    if CONF_OPTION_VALUES not in config:
        return config
    raw = config[CONF_OPTION_VALUES]
    if not isinstance(raw, list):
        return config
    if not raw or not all(isinstance(item, dict) for item in raw):
        return config
    options = []
    values = []
    for item in raw:
        if len(item) != 1:
            raise cv.Invalid("option_values map entries must have exactly one key/value")
        (k, v), = item.items()
        options.append(str(k))
        values.append(int(v))
    config = dict(config)
    config[CONF_OPTIONS] = options
    config[CONF_OPTION_VALUES] = values
    return config


def _validate_select_config(config):
    if not config[CONF_OPTIONS]:
        raise cv.Invalid("options must not be empty")
    if config[CONF_VALUE_TYPE] in ["FP32", "FP32_R"]:
        raise cv.Invalid("value_type for select must be an integer type (not FP32/FP32_R)")
    if CONF_OPTION_VALUES in config:
        if len(config[CONF_OPTION_VALUES]) != len(config[CONF_OPTIONS]):
            raise cv.Invalid("option_values length must match options length")
    return config


CONFIG_SCHEMA = cv.All(
    _preprocess_optionsmap,
    _preprocess_option_values_alias,
    select.select_schema(ModbusTCPSelect)
    .extend(cv.COMPONENT_SCHEMA)
    .extend(ModbusItemBaseSchema)
    .extend(
        {
            cv.Optional(CONF_OPTIONS): cv.ensure_list(cv.string),
            cv.Optional(CONF_REGISTER_TYPE): cv.enum(MODBUS_WRITE_REGISTER_TYPE),
            cv.Optional(CONF_VALUE_TYPE, default="U_WORD"): cv.enum(SENSOR_VALUE_TYPE),
            cv.Optional(CONF_REGISTER_COUNT, default=0): cv.positive_int,
            cv.Optional(CONF_USE_WRITE_MULTIPLE, default=False): cv.boolean,
            cv.Optional(CONF_OPTION_VALUES): cv.Any(
                cv.ensure_list(cv.int_),
                cv.ensure_list(cv.Schema({cv.string: cv.int_})),
            ),
            cv.Optional(CONF_OPTIONS_MAP): cv.Schema({cv.string: cv.int_}),
            cv.Optional(CONF_WRITE_LAMBDA): cv.returning_lambda,
        }
    ),
    validate_modbus_register,
    _validate_select_config,
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
    await select.register_select(var, config, options=config[CONF_OPTIONS])

    paren = await cg.get_variable(config[CONF_MODBUSTCP_CONTROLLER_ID])
    cg.add(var.set_parent(paren))
    cg.add(var.set_use_write_multiple(config[CONF_USE_WRITE_MULTIPLE]))
    cg.add(paren.add_sensor_item(var))
    cg.add(var.set_select_options(config[CONF_OPTIONS]))
    if CONF_OPTION_VALUES in config:
        cg.add(var.set_option_values(config[CONF_OPTION_VALUES]))

    if CONF_WRITE_LAMBDA in config:
        template_ = await cg.process_lambda(
            config[CONF_WRITE_LAMBDA],
            [
                (ModbusTCPSelect.operator("ptr"), "item"),
                (cg.std_string, "x"),
                (cg.std_vector.template(cg.uint8).operator("ref"), "payload"),
            ],
            return_type=cg.optional.template(cg.std_string),
        )
        cg.add(var.set_write_template(template_))

    await add_modbus_base_properties(
        var, config, ModbusTCPSelect, cg.std_string, cg.std_string
    )
