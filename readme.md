# Universal Modbus-TCP esphome

port is an Option / standard 502


# Modbus_TCP (nearly same as original modbus (rtu)  

```yaml
external_components:
  - source: github://creepystefan/esphome_tcp
    refresh: 0s
esphome:
  min_version: 2025.11.0
```
# for esp8266
```yaml
esp8266:
  board: YOURBOARDVARIANT
```
# for esp32 / ONLY Framework ARDUINO
```yaml
esp32:
  board: esp32dev / YOURBOARDVARIANT
  framework:
    type: arduino
```
# for modbus TCP
```yaml
modbus:
  - id: modbustesttcp
    host: 192.168.178.46
    port: 502               # 502 is default
    send_wait_time: 250ms   # 250ms is default
```
alle Components orignal from ESPHOME

in modbus_controller:  address = UNIT ID
platform: modbustcp_controller

## Example config (modbustcp_controller)
```yaml
modbustcp_controller:
  - id: modbus_device
    modbustcp_id: modbustesttcp
    address: 1
    update_interval: 5s   # default 60s

sensor:
  - platform: modbustcp_controller
    modbustcp_controller_id: modbus_device
    name: "A-Voltage"
    address: 1021
    value_type: FP32
    register_type: read
    accuracy_decimals: 2

binary_sensor:
  - platform: modbustcp_controller
    modbustcp_controller_id: modbus_device
    name: "Error Status"
    register_type: read
    address: 0x3200

switch:
  - platform: modbustcp_controller
    modbustcp_controller_id: modbus_device
    id: test_switch
    register_type: coil
    address: 2
    name: "Test Switch"
    bitmask: 1

number:
  - platform: modbustcp_controller
    modbustcp_controller_id: modbus_device
    name: "Setpoint"
    address: 100
    register_type: holding
    value_type: S_WORD
    min_value: 0
    max_value: 100
    step: 1

select:
  - platform: modbustcp_controller
    modbustcp_controller_id: modbus_device
    name: "Mode"
    address: 200
    register_type: holding
    value_type: U_WORD
    options:
      - "Off"
      - "Auto"
      - "Manual"
    option_values:
      - 0
      - 2
      - 3

text_sensor:
  - platform: modbustcp_controller
    modbustcp_controller_id: modbus_device
    name: "Device Name"
    address: 300
    register_type: read
    register_count: 8
    raw_encode: false
```

### value_type
datatype of the modbus register data. Default is `U_WORD`.

- `U_WORD`: unsigned 16-bit, 1 register
- `S_WORD`: signed 16-bit, 1 register
- `U_DWORD`: unsigned 32-bit, 2 registers
- `S_DWORD`: signed 32-bit, 2 registers
- `U_DWORD_R`: little endian unsigned 32-bit, 2 registers
- `S_DWORD_R`: little endian signed 32-bit, 2 registers
- `U_QWORD`: unsigned 64-bit, 4 registers
- `S_QWORD`: signed 64-bit, 4 registers
- `U_QWORD_R`: little endian unsigned 64-bit, 4 registers
- `S_QWORD_R`: little endian signed 64-bit, 4 registers
- `FP32`: 32-bit float, 2 registers
- `FP32_R`: little endian 32-bit float, 2 registers

### Platform notes
- `select`: `value_type` must be integer (no `FP32` / `FP32_R`).
- `text_sensor`: set `raw_encode: true` to publish hex instead of ASCII.


# useful link
https://ipc2u.de/artikel/wissenswertes/detaillierte-beschreibung-des-modbus-tcp-protokolls-mit-befehlsbeispielen/
