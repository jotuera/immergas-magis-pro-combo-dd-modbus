from esphome import automation
import esphome.codegen as cg
from esphome.components import climate
import esphome.config_validation as cv

DEPENDENCIES = ["climate"]

# Own enum/validator definitions: do not depend on names exported by esphome.components.climate,
# which differ between ESPHome releases (e.g. ClimateAction is missing in 2026.8).
climate_ns = cg.esphome_ns.namespace("climate")
ClimateMode = climate_ns.enum("ClimateMode")
ClimateAction = climate_ns.enum("ClimateAction")
ClimatePreset = climate_ns.enum("ClimatePreset")
MODES = {
    "OFF": ClimateMode.CLIMATE_MODE_OFF,
    "HEAT_COOL": ClimateMode.CLIMATE_MODE_HEAT_COOL,
    "COOL": ClimateMode.CLIMATE_MODE_COOL,
    "HEAT": ClimateMode.CLIMATE_MODE_HEAT,
    "DRY": ClimateMode.CLIMATE_MODE_DRY,
    "FAN_ONLY": ClimateMode.CLIMATE_MODE_FAN_ONLY,
    "AUTO": ClimateMode.CLIMATE_MODE_AUTO,
}
PRESETS = {
    "NONE": ClimatePreset.CLIMATE_PRESET_NONE,
    "HOME": ClimatePreset.CLIMATE_PRESET_HOME,
    "AWAY": ClimatePreset.CLIMATE_PRESET_AWAY,
    "BOOST": ClimatePreset.CLIMATE_PRESET_BOOST,
    "COMFORT": ClimatePreset.CLIMATE_PRESET_COMFORT,
    "ECO": ClimatePreset.CLIMATE_PRESET_ECO,
    "SLEEP": ClimatePreset.CLIMATE_PRESET_SLEEP,
    "ACTIVITY": ClimatePreset.CLIMATE_PRESET_ACTIVITY,
}

bus_climate_ns = cg.esphome_ns.namespace("bus_climate")
BusClimate = bus_climate_ns.class_("BusClimate", climate.Climate, cg.PollingComponent)

CONF_CURRENT_TEMPERATURE_LAMBDA = "current_temperature_lambda"
CONF_TARGET_TEMPERATURE_LAMBDA = "target_temperature_lambda"
CONF_MODE_LAMBDA = "mode_lambda"
CONF_ACTION_LAMBDA = "action_lambda"
CONF_SUPPORTED_MODES = "supported_modes"
CONF_SET_TARGET_TEMPERATURE_ACTION = "set_target_temperature_action"
CONF_SET_MODE_ACTION = "set_mode_action"
CONF_CURRENT_HUMIDITY_LAMBDA = "current_humidity_lambda"
CONF_PRESET_LAMBDA = "preset_lambda"
CONF_SUPPORTED_PRESETS = "supported_presets"
CONF_CUSTOM_PRESETS = "custom_presets"
CONF_SET_PRESET_ACTION = "set_preset_action"

CONFIG_SCHEMA = (
    climate.climate_schema(BusClimate)
    .extend(
        {
            cv.Required(CONF_CURRENT_TEMPERATURE_LAMBDA): cv.returning_lambda,
            cv.Required(CONF_TARGET_TEMPERATURE_LAMBDA): cv.returning_lambda,
            cv.Required(CONF_MODE_LAMBDA): cv.returning_lambda,
            cv.Required(CONF_ACTION_LAMBDA): cv.returning_lambda,
            cv.Required(CONF_SUPPORTED_MODES): cv.ensure_list(cv.enum(MODES, upper=True)),
            cv.Optional(CONF_SET_TARGET_TEMPERATURE_ACTION): automation.validate_automation(
                single=True
            ),
            cv.Optional(CONF_SET_MODE_ACTION): automation.validate_automation(
                single=True
            ),
            cv.Optional(CONF_CURRENT_HUMIDITY_LAMBDA): cv.returning_lambda,
            # preset_lambda returns "COMFORT", "ECO", ... (standard) or a custom preset name; {} = unknown
            cv.Optional(CONF_PRESET_LAMBDA): cv.returning_lambda,
            cv.Optional(CONF_SUPPORTED_PRESETS, default=[]): cv.ensure_list(
                cv.one_of(*PRESETS, upper=True)
            ),
            cv.Optional(CONF_CUSTOM_PRESETS, default=[]): cv.ensure_list(cv.string_strict),
            cv.Optional(CONF_SET_PRESET_ACTION): automation.validate_automation(
                single=True
            ),
        }
    )
    .extend(cv.polling_component_schema("2s"))
)


async def to_code(config):
    var = await climate.new_climate(config)
    await cg.register_component(var, config)

    for key, setter, rtype in (
        (CONF_CURRENT_TEMPERATURE_LAMBDA, var.set_current_lambda, cg.float_),
        (CONF_TARGET_TEMPERATURE_LAMBDA, var.set_target_lambda, cg.float_),
        (CONF_MODE_LAMBDA, var.set_mode_lambda, ClimateMode),
        (CONF_ACTION_LAMBDA, var.set_action_lambda, ClimateAction),
    ):
        lam = await cg.process_lambda(
            config[key], [], return_type=cg.optional.template(rtype)
        )
        cg.add(setter(lam))

    for mode in config[CONF_SUPPORTED_MODES]:
        cg.add(var.add_supported_mode(mode))

    if CONF_SET_TARGET_TEMPERATURE_ACTION in config:
        await automation.build_automation(
            var.get_set_target_trigger(),
            [(float, "x")],
            config[CONF_SET_TARGET_TEMPERATURE_ACTION],
        )
    if CONF_SET_MODE_ACTION in config:
        await automation.build_automation(
            var.get_set_mode_trigger(),
            [(ClimateMode, "x")],
            config[CONF_SET_MODE_ACTION],
        )

    if CONF_CURRENT_HUMIDITY_LAMBDA in config:
        lam = await cg.process_lambda(
            config[CONF_CURRENT_HUMIDITY_LAMBDA], [], return_type=cg.optional.template(cg.float_)
        )
        cg.add(var.set_humidity_lambda(lam))
    if CONF_PRESET_LAMBDA in config:
        lam = await cg.process_lambda(
            config[CONF_PRESET_LAMBDA], [], return_type=cg.optional.template(cg.std_string)
        )
        cg.add(var.set_preset_lambda(lam))
    for preset in config[CONF_SUPPORTED_PRESETS]:
        cg.add(var.add_supported_preset(PRESETS[preset]))
    for name in config[CONF_CUSTOM_PRESETS]:
        cg.add(var.add_custom_preset(name))
    if CONF_SET_PRESET_ACTION in config:
        await automation.build_automation(
            var.get_set_preset_trigger(),
            [(cg.std_string, "x")],
            config[CONF_SET_PRESET_ACTION],
        )
