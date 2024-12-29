import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, uart
from esphome.const import (
    CONF_ID,
    UNIT_EMPTY,
    ICON_MOTION_SENSOR,
    ICON_PULSE,
    DEVICE_CLASS_EMPTY,
    UNIT_BEATS_PER_MINUTE,
    ICON_COUNTER,
    DEVICE_CLASS_MOTION,
    UNIT_PERCENT,
)

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor"]

mmwave_sensor_ns = cg.esphome_ns.namespace("mmwave_sensor")
MMWaveSensor = mmwave_sensor_ns.class_("MMWaveSensor", sensor.Sensor, cg.PollingComponent, uart.UARTDevice)

CONF_PRESENCE = "presence"
CONF_MOVEMENT = "movement"
CONF_MOVEMENT_RANGE = "movement_range"
CONF_BREATH_RATE = "breath_rate"
CONF_HEART_RATE = "heart_rate"

CONFIG_SCHEMA = (
    sensor.sensor_schema(
        MMWaveSensor,
        unit_of_measurement=UNIT_EMPTY,
        icon=ICON_MOTION_SENSOR,
        accuracy_decimals=0,
    )
    .extend({
        cv.Optional(CONF_PRESENCE): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_MOTION_SENSOR,
            device_class=DEVICE_CLASS_MOTION,
            accuracy_decimals=0,
        ),
        cv.Optional(CONF_MOVEMENT): sensor.sensor_schema(
            unit_of_measurement=UNIT_EMPTY,
            icon=ICON_MOTION_SENSOR,
            device_class=DEVICE_CLASS_MOTION,
            accuracy_decimals=0,
        ),
        cv.Optional(CONF_MOVEMENT_RANGE): sensor.sensor_schema(
            unit_of_measurement=UNIT_PERCENT,
            icon=ICON_COUNTER,
            accuracy_decimals=0,
        ),
        cv.Optional(CONF_BREATH_RATE): sensor.sensor_schema(
            unit_of_measurement=UNIT_BEATS_PER_MINUTE,
            icon=ICON_PULSE,
            accuracy_decimals=0,
        ),
        cv.Optional(CONF_HEART_RATE): sensor.sensor_schema(
            unit_of_measurement=UNIT_BEATS_PER_MINUTE,
            icon=ICON_PULSE,
            accuracy_decimals=0,
        ),
    })
    .extend(cv.polling_component_schema("60s"))
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID], config[CONF_UART_ID])
    await cg.register_component(var, config)
    
    if CONF_PRESENCE in config:
        sens = await sensor.new_sensor(config[CONF_PRESENCE])
        cg.add(var.set_presence_sensor(sens))
        
    if CONF_MOVEMENT in config:
        sens = await sensor.new_sensor(config[CONF_MOVEMENT])
        cg.add(var.set_movement_sensor(sens))
        
    if CONF_MOVEMENT_RANGE in config:
        sens = await sensor.new_sensor(config[CONF_MOVEMENT_RANGE])
        cg.add(var.set_movement_range_sensor(sens))
        
    if CONF_BREATH_RATE in config:
        sens = await sensor.new_sensor(config[CONF_BREATH_RATE])
        cg.add(var.set_breath_sensor(sens))
        
    if CONF_HEART_RATE in config:
        sens = await sensor.new_sensor(config[CONF_HEART_RATE])
        cg.add(var.set_heart_sensor(sens))
