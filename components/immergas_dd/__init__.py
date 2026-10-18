"""Shared helpers of the D+/D- emulator, used from the YAML lambdas.

lww_guard.h   - arbitration between Home Assistant commands and values seen on the bus.
faults_i18n.h - boiler fault descriptions (English / Italian).

Enable with an empty `immergas_dd:` block; the headers are then available in every lambda.
"""
import esphome.codegen as cg
import esphome.config_validation as cv

CODEOWNERS = ["@jotuera", "@robpegurri"]

CONFIG_SCHEMA = cv.Schema({})


async def to_code(config):
    cg.add_global(cg.RawStatement('#include "esphome/components/immergas_dd/lww_guard.h"'))
    cg.add_global(cg.RawStatement('#include "esphome/components/immergas_dd/faults_i18n.h"'))
