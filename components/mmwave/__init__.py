import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, text_sensor  # Import text_sensor
from esphome.const import CONF_ID, CONF_NAME

DEPENDENCIES = ["uart"]

mmwave_component_ns = cg.esphome_ns.namespace("mmwave_ns")
MMWaveComponent = mmwave_component_ns.class_(
    "MMWaveComponent", cg.Component, uart.UARTDevice
)

CONF_PACKET_TEXT_SENSOR_ID = "packet_text_sensor_id"
CONF_NUM_PACKETS = "num_packets"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MMWaveComponent),
            cv.Optional(CONF_PACKET_TEXT_SENSOR_ID): text_sensor.TEXT_SENSOR_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_NUM_PACKETS, default=5): cv.int_,
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    cg.add(var.set_num_packets_to_store(config[CONF_NUM_PACKETS]))

    if CONF_PACKET_TEXT_SENSOR_ID in config:
        conf = config[CONF_PACKET_TEXT_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await text_sensor.register_text_sensor(sens, conf)
        cg.add(var.set_packet_text_sensor(sens))