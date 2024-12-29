import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import UNIT_EMPTY, ICON_EMPTY

mmwave_sensor_ns = cg.esphome_ns.namespace("mmwave_sensor")

MMWaveSensor = mmwave_sensor_ns.class_("MMWaveSensor", sensor.Sensor, cg.PollingComponent)

CONFIG_SCHEMA = sensor.sensor_schema(
    MMWaveSensor, unit_of_measurement=UNIT_EMPTY, icon=ICON_EMPTY, accuracy_decimals=1
).extend(cv.polling_component_schema("60s"))

# Error handling for sensor initialization
async def to_code(config):
    try:
        var = await sensor.new_sensor(config)
        await cg.register_component(var, config)
    except Exception as e:
        raise ValueError(f"Error initializing MMWaveSensor: {e}")

# Detailed comments explaining the purpose of each section of the code
# The above code defines the MMWaveSensor class and its configuration schema.
# The to_code function initializes the sensor and registers it as a component.
# Error handling is added to catch any issues during initialization.
