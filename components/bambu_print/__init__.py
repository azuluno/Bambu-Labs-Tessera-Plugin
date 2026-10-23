"""ESPHome component for the Bambu Lab Tessera tile."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, smart_display, text_sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["smart_display"]

bambu_print_ns = cg.esphome_ns.namespace("bambu_print")
BambuPrint = bambu_print_ns.class_("BambuPrint", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(BambuPrint),
        cv.Required("progress"): cv.use_id(sensor.Sensor),
        cv.Required("remaining_time"): cv.use_id(sensor.Sensor),
        cv.Required("nozzle_temperature"): cv.use_id(sensor.Sensor),
        cv.Required("bed_temperature"): cv.use_id(sensor.Sensor),
        cv.Required("current_layer"): cv.use_id(sensor.Sensor),
        cv.Required("total_layers"): cv.use_id(sensor.Sensor),
        cv.Required("stage"): cv.use_id(text_sensor.TextSensor),
        cv.Required("light_state"): cv.use_id(text_sensor.TextSensor),
        cv.Required("pause_button"): cv.string,
        cv.Required("resume_button"): cv.string,
        cv.Required("stop_button"): cv.string,
        cv.Required("chamber_light"): cv.string,
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    for key in (
        "progress",
        "remaining_time",
        "nozzle_temperature",
        "bed_temperature",
        "current_layer",
        "total_layers",
    ):
        cg.add(getattr(var, f"set_{key}")(await cg.get_variable(config[key])))

    for key in ("stage", "light_state"):
        cg.add(getattr(var, f"set_{key}")(await cg.get_variable(config[key])))

    for key in ("pause_button", "resume_button", "stop_button", "chamber_light"):
        cg.add(getattr(var, f"set_{key}")(config[key]))

    await smart_display.register_plugin(var, __file__)