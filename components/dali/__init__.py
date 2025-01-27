import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.const import CONF_ID

assert __package__ == "esphome.components.dali"

# Names for configuration
CONF_TX_PIN = 'tx_pin'
CONF_RX_PIN = 'rx_pin'

# Define component namespace and class name for esphome
DALI_ns = cg.esphome_ns.namespace('DALI')
DALIInterface = DALI_ns.class_('DALIInterface', cg.Component)

# Config schema for the usage in the device yaml
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(DALIInterface),
    cv.Required(CONF_TX_PIN): pins.gpio_output_pin_schema,
    cv.Optional(CONF_RX_PIN): pins.gpio_input_pin_schema,
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):

    # Registration of the component
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Set TX pin
    tx_pin = await cg.gpio_pin_expression(config[CONF_TX_PIN])
    cg.add(var.set_tx_pin(tx_pin))

    # Set RX pin
    if CONF_RX_PIN in config:
        rx_pin = await cg.gpio_pin_expression(config[CONF_RX_PIN])
        cg.add(var.set_rx_pin(rx_pin))
