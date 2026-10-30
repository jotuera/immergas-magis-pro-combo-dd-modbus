# Changelog

## v2.1.1

- **Fix:** a device without restored schedule values (new device, re-created entities) served its defaults to the
  boiler - Calendar 1 on every day - and the boiler adopted them, overwriting the real schedule (seen as DHW
  heating all day). For 2 minutes after boot a schedule register (2310-2347, 2410-2436, 2490-2496) is now served as
  -1 ("not set", like a fresh panel) until its real value is seen on the bus: a READ reply from the physical
  Dominus 0x33 or a boiler write. Registers nobody reports in that window fall back to the HA value as before.
  Entities and names unchanged.

## v2.1.0

- New substitution **`dd_name_prefix`** (default `"Immergas"`): the first part of every entity name
  (`"<prefix> - <name>"`). With the default the names are exactly as in v2.0.x. Set it in the main YAML when the
  D+/D- packages share one device with tt-bms / f1f2-nasa, e.g. `dd_name_prefix: "Immergas DD"`.
- The boot log names the configured DE pin (`dd_uart_de_pin`) instead of a fixed GPIO23.

## v2.0.2

- **Fix ([#2](https://github.com/jotuera/immergas-magis-pro-combo-dd-modbus/issues/2)):** the temperature / humidity
  source selects of Zones 1-3 kept their choice only until a reboot (then back to the default: Upper floor for
  Zone 1, Panel for Zones 2/3). They now restore the last option, and on boot it is applied again so the value sent
  to the boiler comes from the restored source. Entities and names unchanged.
- README: reported working on the Magis Pro 9 V2.

## v2.0.1

- `uart:` in `dd_core` is now a list (`- id: dd_uart`), like the tt-bms and f1f2-nasa repos. With the old map form
  ESPHome replaced it when the packages of several repos were merged on one device ("Couldn't find ID 'dd_uart'").
  A single D+/D- device is unchanged (same resolved configuration, same entities).

## v2.0.0

- **Naming scheme shared with tt-bms** (`dd_` prefix) so the Immergas repos can be combined on one device:
  packages `packages/dd_*.yaml`, languages `packages/lang/dd_en|dd_pl|dd_it.yaml`, component `dd_bus_climate`,
  ids `dd_*`, substitutions `dd_*` / `n_dd_*` / `t_dd_*`.
- Device settings (esp32, logger, debug, device-health sensors) moved from `core` to the main YAML.
- Entity names unchanged (no new entities in HA). Restored values are migrated once from the old global ids
  (`dd_migrated_v2`); the legacy globals will be removed in a later release.
- **Breaking:** new main YAML (file list, `dd_` substitutions).

## v1.6.0

- **Translations**: entity names and the texts shown in HA come from a language file - `packages/lang/en.yaml`, `pl.yaml` or `it.yaml` (Italian: texts and fault descriptions translated, entity names still English - contributions welcome, see TRANSLATING.md). English output is unchanged.
- Fault descriptions also in **Polish**; "Fault language" select: English / Italiano / Polski, default from the language file.
- New: `A11 Heat-pump module present` (0x29 6150), zone phase "Air purge" during U50 (2010 = 1), 7 raw registers not decoded yet (diagnostic, **disabled by default**).
- `setpoint_step` substitution (default 0.1 C) for the setpoint sliders / Zone 1 climate.

## v1.5.0

- The configuration is split into packages (`packages/*.yaml`) loaded from GitHub by the main YAML. The resolved configuration is identical to v1.4.0 (verified with `esphome config`), so nothing changes on the device or in Home Assistant.
- RS485 pins are substitutions (`uart_tx_pin`, `uart_rx_pin`, `uart_de_pin`) with the previous values as defaults.
- The changelog moved to this file.


## v1.4.0

PR #1 by Roberto Pegurri (@robpegurri), tested on Magis Combo V2 with a physical panel + emulated Dominus and here with an emulated panel + physical Dominus: FIX: FC16 writes from the boiler to the emulated Dominus were dropped (panel changes were reverted); schedule changes seen on the bus now update the HA entities. Bus/HA arbitration (range check, panel warm-up, pending HA commands, settle window) replaces the v117 last-writer-wins; the v1.2.0 anti-legionella fix is kept. RS485 DE driven by the UART hardware (flow_control_pin), no busy-wait. Climates backed by the bus (bus_climate) replace thermostat + the v118 sync loop - BREAKING: the "Dom" preset is gone, presets are Comfort / Eco (from the boiler phase). Fault texts in English or Italian ("Fault language"). Diagnostics: uptime, WiFi, loop time, free heap, reset reason, RS485 CRC errors, Dominus TX frames, boiler clock drift, system flow/return/delta T, Zone 1 heating request. Setpoint step 0.1 C. Logger INFO. Packaging: the components and headers are fetched from GitHub (external_components) - only this YAML goes into ESPHome. Small fixes: integer sensors show "1" instead of "1.0" (state_class), "HP Night function (U11)" renamed "HP U11 Night function", "Zone 1 heating request" shows on/off (no "hot").

## v1.3.1

NEW: night function (boiler parameter U11). Inside the U12-U13 window the boiler writes a compressor capacity limit in % to the outdoor-unit interface board (0x1E reg 2, 30 = night function; the MIM echoes it in input reg 12). Read-only entities: "HP Night function (U11)" and "HP Compressor capacity limit" (%, 100 = no limit). On the Samsung side the outdoor unit caps the compressor (~19 Hz instead of 64 Hz). The night function cannot be switched from HA (only the boiler writes it). "Boiler firmware version" renamed to "D91 Boiler firmware version" (same value as D91 on the T-/T+ BMS bus).

## v1.3.0

NEW: zone-panel settings the boiler READS from panel 0x29 are now controllable from HA (they were hard-coded): "Time programs Zone 1" (2294, Manual/Auto - Auto makes the boiler follow the CH weekly schedule, comfort/economy), "Room probe modulation" (6105, NO/YES), "Dew point correction" (6107). Defaults = previous hard-coded values (Manual / NO / ON), so nothing changes until you touch them. With a physical panel connected these controls mirror the panel's own replies instead. NEW read-only: "A31 Zone 1 room thermostat" (6102, RPT/RT/RP - a boiler parameter, the boiler never reads it back, so it cannot be set over the bus), "Time program state (boiler)" (2090: Manual / Auto (comfort) / Auto (economy)), "D09 DHW request pending" and "Boiler fault active (2001)". The emulator now echoes A31 back as written by the boiler (like the physical panel) instead of 0.

## v1.2.0

FIX: DHW setpoint stuck at 60 C after the weekly anti-legionella cycle (boiler P15). The emulator adopted the boiler's ACTIVE DHW setpoint (3015, written to the panel) as a user change and, acting as the panel, fed it back -> the boiler "restored" 60 C forever. Now only device replies (Dominus 0x33 / physical panel) change the DHW setpoint; 3015 is display-only ("D05 Active DHW setpoint (boiler)"). NEW: heat-pump data from the outdoor-unit interface board (0x1E, Samsung MIM): compressor Hz, discharge / evaporator / compressor-shell temps, current, fan target, EEV, water target, run flags, operating stage, firmware versions; boiler clock (2300); main/secondary generator flow/return (3011-3014); calculated flow setpoint D04 (3003); number of zones A13 (4199). Entity names carry the Immergas manual codes (D71, D73, ...). Optional bus sniffer (diagnostic, OFF by default) logging every register change (tag "sniff"). Register map: REGISTER_MAP.md.

## v1.1.0

Fix setpoints (CH/DHW) bouncing/reverting (e.g. CH Setpoint Zone 2 25<->21). Last-writer-wins: the slider AND the climate now hold their value and adopt genuine boiler/Dominus changes. Root cause: the 1s loop forced setpoint = thermostat unconditionally, and make_call() only set target_temperature_low (a no-op on heat-only climates). Now: delta-sync loop + dual target setter.

## v1.0.0

Initial public release.
