import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID

DEPENDENCIES = ["uart"]

DFRobot_HumanDetection = cg.global_ns.class_(
    "DFRobot_HumanDetection", cg.Component, uart.UARTDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(DFRobot_HumanDetection),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(uart.UART_DEVICE_SCHEMA)
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await uart.register_uart_device(var, config)

    try:
        uart_bus = await cg.get_variable(config[uart.CONF_UART_ID])
        var = cg.new_Pvariable(config[CONF_ID], uart_bus)
    except KeyError:
        raise cv.Invalid("No UART bus configured for DFRobot_HumanDetection")
