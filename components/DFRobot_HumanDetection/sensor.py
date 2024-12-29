import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID
)

# Define your own constants
CONF_PRESENCE = "presence"
CONF_MOVEMENT = "movement"
CONF_DISTANCE = "distance"
CONF_HEART_RATE = "heart_rate"
CONF_BREATH_STATE = "breath_state"
CONF_BREATH_VALUE = "breath_value"

DEPENDENCIES = ["uart"]

DFRobot_HumanDetectionComponent = cg.global_ns.class_(
    "DFRobot_HumanDetectionComponent", cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DFRobot_HumanDetectionComponent),
            cv.Optional(CONF_PRESENCE): sensor.sensor_schema(
                unit_of_measurement=None, icon="mdi:human-presence", accuracy_decimals=0
            ),
            cv.Optional(CONF_MOVEMENT): sensor.sensor_schema(
                unit_of_measurement=None, icon="mdi:motion-sensor", accuracy_decimals=0
            ),
            cv.Optional(CONF_DISTANCE): sensor.sensor_schema(
                unit_of_measurement="cm", icon="mdi:ruler", accuracy_decimals=0
            ),
            cv.Optional(CONF_HEART_RATE): sensor.sensor_schema(
                unit_of_measurement="bpm", icon="mdi:heart-pulse", accuracy_decimals=0
            ),
            cv.Optional(CONF_BREATH_STATE): sensor.sensor_schema(
                unit_of_measurement=None, icon="mdi:lungs", accuracy_decimals=0
            ),
            cv.Optional(CONF_BREATH_VALUE): sensor.sensor_schema(
                unit_of_measurement=None, icon="mdi:chart-line", accuracy_decimals=0
            ),
            # Add other sensors here, following the same pattern
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    if CONF_PRESENCE in config:
        sens = await sensor.new_sensor(config[CONF_PRESENCE])
        cg.add(var.set_presence_sensor(sens))
    if CONF_MOVEMENT in config:
        sens = await sensor.new_sensor(config[CONF_MOVEMENT])
        cg.add(var.set_movement_sensor(sens))
    if CONF_DISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_DISTANCE])
        cg.add(var.set_distance_sensor(sens))
    if CONF_HEART_RATE in config:
        sens = await sensor.new_sensor(config[CONF_HEART_RATE])
        cg.add(var.set_heart_rate_sensor(sens))
    if CONF_BREATH_STATE in config:
        sens = await sensor.new_sensor(config[CONF_BREATH_STATE])
        cg.add(var.set_breath_state_sensor(sens))
    if CONF_BREATH_VALUE in config:
        sens = await sensor.new_sensor(config[CONF_BREATH_VALUE])
        cg.add(var.set_breath_value_sensor(sens))
    # Add other sensors here, following the same pattern