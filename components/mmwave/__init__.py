import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart, text_sensor, binary_sensor, button, number
from esphome.const import CONF_ID, CONF_NAME, CONF_ON_PRESS

DEPENDENCIES = ["uart"]

mmwave_component_ns = cg.esphome_ns.namespace("mmwave_ns")
MMWaveComponent = mmwave_component_ns.class_(
    "MMWaveComponent", cg.Component, uart.UARTDevice
)

CONF_PACKET_TEXT_SENSOR_ID = "packet_text_sensor_id"
CONF_CONFIG_TEXT_SENSOR_ID = "config_text_sensor_id"
CONF_POSITION_TEXT_SENSOR_ID = "position_text_sensor_id"
CONF_MOVEMENT_SENSOR_ID = "movement_sensor_id"

CONF_PRESENCE_SENSOR_ID = "presence_sensor_id"
CONF_SLEEP_STATE_SENSOR_ID = "sleep_state_sensor_id"
CONF_AVERAGE_RESPIRATION_SENSOR_ID = "average_respiration_sensor_id"
CONF_AVERAGE_HEARTBEAT_SENSOR_ID = "average_heartbeat_sensor_id"
CONF_TURNOVER_SENSOR_ID = "turnover_sensor_id"
CONF_LARGE_BODYMOVE_SENSOR_ID = "large_bodymove_sensor_id"
CONF_MINOR_BODYMOVE_SENSOR_ID = "minor_bodymove_sensor_id"
CONF_APNEA_EVENTS_SENSOR_ID = "apnea_events_sensor_id"

CONF_HUMAN_PRESENCE_SENSOR_ID = "human_presence_sensor_id"
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
            cv.Optional(CONF_CONFIG_TEXT_SENSOR_ID): text_sensor.TEXT_SENSOR_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_POSITION_TEXT_SENSOR_ID): text_sensor.TEXT_SENSOR_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_MOVEMENT_SENSOR_ID): text_sensor.TEXT_SENSOR_SCHEMA.extend(
                {
                    cv.GenerateID(): cv.declare_id(text_sensor.TextSensor),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_PRESENCE_SENSOR_ID): number.NUMBER_SCHEMA.extend(
                { 
                    cv.GenerateID(): cv.declare_id(number.Number),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_SLEEP_STATE_SENSOR_ID): number.NUMBER_SCHEMA.extend(
                { 
                    cv.GenerateID(): cv.declare_id(number.Number),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_AVERAGE_RESPIRATION_SENSOR_ID): number.NUMBER_SCHEMA.extend(
                { 
                    cv.GenerateID(): cv.declare_id(number.Number),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_AVERAGE_RESPIRATION_SENSOR_ID): number.NUMBER_SCHEMA.extend(
                { 
                    cv.GenerateID(): cv.declare_id(number.Number),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_AVERAGE_HEARTBEAT_SENSOR_ID): number.NUMBER_SCHEMA.extend(
                { 
                    cv.GenerateID(): cv.declare_id(number.Number),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_TURNOVER_SENSOR_ID): number.NUMBER_SCHEMA.extend(
                { 
                    cv.GenerateID(): cv.declare_id(number.Number),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_LARGE_BODYMOVE_SENSOR_ID): number.NUMBER_SCHEMA.extend(
                { 
                    cv.GenerateID(): cv.declare_id(number.Number),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_MINOR_BODYMOVE_SENSOR_ID): number.NUMBER_SCHEMA.extend(
                { 
                    cv.GenerateID(): cv.declare_id(number.Number),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_APNEA_EVENTS_SENSOR_ID): number.NUMBER_SCHEMA.extend(
                { 
                    cv.GenerateID(): cv.declare_id(number.Number),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
            cv.Optional(CONF_NUM_PACKETS, default=5): cv.int_,
            cv.Optional(CONF_HUMAN_PRESENCE_SENSOR_ID): binary_sensor.BINARY_SENSOR_SCHEMA.extend(  # Add this
                {
                    cv.GenerateID(): cv.declare_id(binary_sensor.BinarySensor),
                    cv.Optional(CONF_NAME): cv.string,
                }
            ),
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

    if CONF_CONFIG_TEXT_SENSOR_ID in config:
        conf = config[CONF_CONFIG_TEXT_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await text_sensor.register_text_sensor(sens, conf)
        cg.add(var.set_config_text_sensor(sens))

    if CONF_POSITION_TEXT_SENSOR_ID in config:
        conf = config[CONF_POSITION_TEXT_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await text_sensor.register_text_sensor(sens, conf)
        cg.add(var.set_position_sensor(sens))

    if CONF_MOVEMENT_SENSOR_ID in config:
        conf = config[CONF_MOVEMENT_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await text_sensor.register_text_sensor(sens, conf)
        cg.add(var.set_movement_sensor(sens))

    if CONF_PRESENCE_SENSOR_ID in config:
        conf = config[CONF_PRESENCE_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(sens, conf, min_value=0, max_value=255, step=1)
        cg.add(var.set_presence_sensor(sens))

    if CONF_SLEEP_STATE_SENSOR_ID in config:
        conf = config[CONF_SLEEP_STATE_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(sens, conf, min_value=0, max_value=255, step=1)
        cg.add(var.set_sleep_state_sensor(sens))

    if CONF_AVERAGE_RESPIRATION_SENSOR_ID in config:
        conf = config[CONF_AVERAGE_RESPIRATION_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(sens, conf, min_value=0, max_value=255, step=1)
        cg.add(var.set_average_respiration_sensor(sens))

    if CONF_AVERAGE_HEARTBEAT_SENSOR_ID in config:
        conf = config[CONF_AVERAGE_HEARTBEAT_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(sens, conf, min_value=0, max_value=255, step=1)
        cg.add(var.set_average_heartbeat_sensor(sens))

    if CONF_TURNOVER_SENSOR_ID in config:
        conf = config[CONF_TURNOVER_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(sens, conf, min_value=0, max_value=255, step=1)
        cg.add(var.set_turnover_sensor(sens))

    if CONF_LARGE_BODYMOVE_SENSOR_ID in config:
        conf = config[CONF_LARGE_BODYMOVE_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(sens, conf, min_value=0, max_value=255, step=1)
        cg.add(var.set_large_bodymove_sensor(sens))

    if CONF_MINOR_BODYMOVE_SENSOR_ID in config:
        conf = config[CONF_MINOR_BODYMOVE_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(sens, conf, min_value=0, max_value=255, step=1)
        cg.add(var.set_minor_bodymove_sensor(sens))

    if CONF_APNEA_EVENTS_SENSOR_ID in config:
        conf = config[CONF_APNEA_EVENTS_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await number.register_number(sens, conf, min_value=0, max_value=255, step=1)
        cg.add(var.set_apnea_events_sensor(sens))

    if CONF_HUMAN_PRESENCE_SENSOR_ID in config: 
        conf = config[CONF_HUMAN_PRESENCE_SENSOR_ID]
        sens = cg.new_Pvariable(conf[CONF_ID])
        await binary_sensor.register_binary_sensor(sens, conf)
        cg.add(var.set_human_presence_sensor(sens)) 
