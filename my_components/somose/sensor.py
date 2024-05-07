import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import i2c, sensor
from esphome.components import sensirion_common
from esphome.automation import maybe_simple_id
from esphome.const import (
    CONF_ID,
    CONF_TEMPERATURE,
    CONF_MOISTURE,
    ICON_WATER_PERCENT,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
    UNIT_PERCENT,
)

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensirion_common"]


somose_ns = cg.esphome_ns.namespace("somose")
SoMoSeComponent = somose_ns.class_(
    "SoMoSeComponent", cg.PollingComponent, sensirion_common.SensirionI2CDevice
)


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(SoMoSeComponent),
            cv.Optional(CONF_TEMPERATURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_CELSIUS,
                accuracy_decimals=2,
                device_class=DEVICE_CLASS_TEMPERATURE,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_MOISTURE): sensor.sensor_schema(
                unit_of_measurement=UNIT_PERCENT,
                icon=ICON_WATER_PERCENT,
                accuracy_decimals=2,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x55))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    # cg.add(var.set_variant(config[CONF_VARIANT]))

    if temperature := config.get(CONF_TEMPERATURE):
        sens = await sensor.new_sensor(temperature)
        cg.add(var.set_temperature_sensor(sens))

    if moisture := config.get(CONF_MOISTURE):
        sens = await sensor.new_sensor(moisture)
        cg.add(var.set_moisture_sensor(sens))


SOMOSE_REFERENCE_ACTION_SCHEMA = maybe_simple_id(
    {
        cv.Required(CONF_ID): cv.use_id(SoMoSeComponent),
    }
)