import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import climate_ir
from esphome.components import ir_remote_base
from esphome.const import  CONF_DEBOUNCE

AUTO_LOAD = ["climate_ir", "ir_remote_base"]

electra_ns = cg.esphome_ns.namespace("electra")
ElectraClimate = electra_ns.class_("ElectraClimate", climate_ir.ClimateIR)

CONFIG_SCHEMA = climate_ir.climate_ir_with_receiver_schema(ElectraClimate).extend(
    {
        cv.Optional(CONF_DEBOUNCE): cv.positive_time_period_milliseconds
    }
)

async def to_code(config):
    ir_remote_base.load_ir_remote()

    var = await climate_ir.new_climate_ir(config)
    if CONF_DEBOUNCE in config:
        cg.add(var.set_debounce(config[CONF_DEBOUNCE]))
