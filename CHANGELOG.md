# Changelog

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
