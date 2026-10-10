# Immergas Magis Pro / Combo — Dominus / Panel emulator on the D+/D- Modbus bus (ESPHome, M5 Atom)

Control an **Immergas Magis Combo** boiler from **Home Assistant** by **emulating the Dominus remote and the zone panels** on the **D+/D- (Modbus RS485)** bus — with an **M5Stack Atom + ESPHome**. No cloud, no proprietary Dominus controller.

The boiler is the Modbus **master**; this device is a **slave** that answers as the Dominus (`0x33`) and/or the zone panels (`0x29` / `0x2A` / `0x2B`). It feeds the boiler room temperature & humidity from **any Home Assistant sensor** and exposes the panel/Dominus settings as HA entities.

> ⚠️ **Tested only on the Magis Combo V2 (`MPROCOMBOV2`).** The Dominus app supports many Immergas engines (Zeus, Star, VictrixMaior, …) with different register maps — this project targets **Magis Combo only**. Other models may partially work but are untested. Contributions welcome.

## What's new in v1.3.0

Zone-panel settings decoded by clicking through every menu of a physical zone panel while sniffing the bus. The boiler **reads** these from panel `0x29`, so an emulated panel can now set them from Home Assistant (until now they were hard-coded):

| Register | Entity | Values | Default |
|---|---|---|---|
| `2294` | **Time programs Zone 1** (select) | Manual / Auto | Manual |
| `6105` | **Room probe modulation** (select) | NO / YES | NO |
| `6107` | **Dew point correction** (switch) | on / off | on |

- Defaults equal the previous hard-coded values, so **nothing changes after the upgrade** until you touch them.
- **Auto** makes the boiler follow the CH weekly schedule (comfort / economy). With *Manual* — the previous behaviour — the schedule was ignored while emulating the zone panel. Check your economy setpoint before switching to Auto.
- With a physical panel connected the controls mirror the panel's own replies (the panel is then the source of truth).
- New read-only entities: **A31 Zone 1 room thermostat** (`6102`, RPT / RT / RP), **Time program state (boiler)** (`2090`: Manual / Auto (comfort) / Auto (economy)), **D09 DHW request pending** and **Boiler fault active (2001)**.
- **A31 cannot be set over the bus**: it is a boiler parameter that the boiler writes to the panel but never reads back. Change it in the boiler menu.
- Not transmitted at all (panel-local or unsupported by the boiler): frost-protection temperature, panel anti-bacterial cycle, dehumidification disable, minimum cooling setpoint, holiday program.

See [REGISTER_MAP.md](REGISTER_MAP.md) for the full list of registers the boiler reads from the panel.

## What's new in v1.2.0

> ⚠️ **Upgrade recommended for all v1.0/v1.1 users who emulate a zone panel.** If the boiler's **anti-legionella** function is enabled (parameter **P15**, by default every Monday at night), older versions get the **DHW setpoint stuck at 60 °C** after the disinfection cycle — visible everywhere (HA, Dominus, boiler) until changed by hand. Cause: the emulator adopted the boiler's *active* DHW setpoint (`3015`, written to the panel) as a user change and, acting as the panel, fed 60 °C back to the boiler. Fixed: only device replies (Dominus / physical panel) change the DHW setpoint; the boiler's active setpoint is shown read-only as **`D05 Active DHW setpoint (boiler)`**.

- **Heat-pump data** decoded from the outdoor-unit interface board (`0x1E`, Samsung MIM) — compressor frequency, discharge / evaporator / compressor-shell temperatures, compressor current, fan target speed, EEV position, water target, run flags (`Run request`, `Request accepted`, `Compressor running`, `Fan running`), operating stage, firmware versions. Cross-checked on three buses (D+/D-, T-/T+ BMS and the Samsung NASA F1/F2 bus).
- **Boiler data** from the boiler → panel block: calculated flow setpoint **D04**, main / secondary generator flow & return (heat pump / gas boiler), boiler clock, number of zones **A13**.
- Entity names carry the **Immergas manual codes** (`D71`, `D73`, `D120-D123`, …).
- Optional **bus sniffer** (diagnostic switch, **OFF by default**) — logs every register change with tag `sniff`; handy for bug reports and decoding new registers.
- Full register map: **[REGISTER_MAP.md](REGISTER_MAP.md)** — help with the remaining unknown registers is welcome.

> v1.2.0 has been tested on a **single-zone Magis Combo V2**. The new heat-pump entities should also work on the Magis Pro (same Samsung outdoor unit + MIM board); the gas-generator entities apply to the Combo only. Multi-zone setups were not re-tested with v1.2.0 — feedback welcome.

## What it does

- **Emulates the Dominus** (`0x33`) and/or **zone panels** (`0x29` = Zone 1, `0x2A` = Zone 2, `0x2B` = Zone 3) so the boiler runs without the original controller.
- **Feeds room temperature & humidity** to the boiler from HA sensors — pick from up to 5 switchable sources per zone (Panel / Basement / Ground floor / Upper floor / Whole house). "Panel" uses the real value read from a physical panel on the bus, if present.
- **Controls**: operating mode, zone CH setpoint, heating offset (U03/U04/U16), Comfort/Eco heat & cool setpoints, set-flow, humidity setting (U07/U08/U18), DHW setpoint.
- **Weekly schedules (chrono)**: 4 shared time-profile calendars (`Calendar 1–4`) + independent day→calendar assignment per zone and for DHW.
- **Monitors**: boiler fault code + **114 fault descriptions in official Immergas English** (extracted from the Dominus app labels), per-zone phase (Comfort/Eco), connection status of each device, firmware versions.
- **Zone-panel settings** (v1.3.0): time programs Manual/Auto, room-probe modulation, dew-point correction (controllable); A31 room thermostat, time-program state, DHW-request-pending and fault flags (read-only).
- **Heat pump & generators** (v1.2.0): compressor Hz / current / temperatures, fan, EEV, run flags and stage from the outdoor-unit interface board; heat-pump and gas-boiler flow/return temperatures; calculated flow setpoint (D04); active DHW setpoint; boiler clock.
- **Local web UI** (ESPHome web server, password-protected) as a fallback independent of Home Assistant.

## Hardware

- **M5Stack Atom Lite** (ESP32) + isolated **RS485** transceiver (e.g. M5 Isolated RS485 Unit / MAX3485).
- Pins: **TX = GPIO26, RX = GPIO32, DE = GPIO23**.
- Wiring: Immergas **D+ / D-** → RS485 **A / B** (swap if you get CRC errors).
- Bus: **Modbus RTU, 9600 8E1** (even parity, 2-wire).
- **A 120 Ω termination resistor across A/B is required** for the Atom to work standalone on the bus (the boiler provides bias; only termination is missing). The M5 Isolated RS485 kit includes one.

## How emulation works (important)

You emulate **either the Dominus or the panels — never the same address as a physical device** on the bus (two devices answering one address = collision). Switches:

- `Emulate Dominus` (0x33), `Emulate Panel Zone 1/2/3` (0x29 / 0x2A / 0x2B) — mutually exclusive with the physical device on that address.
- `Emulate Modbus` — master on/off (stops all RX/TX).
- **Collision safety guard:** you cannot enable emulation of an address while a physical device is already answering there (the connection sensor shows "connected"). Turn the physical device off first.
- Address **`0x1E`** is the **outdoor-unit interface board** (Samsung MIM, "A22" in the wiring diagram; link status = manual code **D62**) — a **real device**. It is only read (heat-pump data) and **never transmitted to** (hard guard in the code).

Typical setups:
- **Physical Dominus present** → emulate a panel (Zone 1) to feed room temp; leave Dominus emulation off.
- **No Dominus** → emulate the Dominus (0x33) to own the operating mode and settings.

## Register map (D+/D-, known)

| Reg | Code | Description | Scale |
|---|---|---|---|
| 2000 | — | Operating mode (0=standby, 1=summer/DHW, 2=cooling, 3=winter) | RW |
| 2005 / 2011 / 3009 | — | Zone room temperature | ×0.1 °C |
| 2006 / 3010 | D41/D42/D102 | Zone relative humidity | % |
| 2010 | — | Zone status / phase (bit6=Comfort, bit7=Eco, bit2=Manual) | R |
| 2015 | — | Zone CH setpoint | ×0.1 °C, RW |
| 2017 / 2219 | U03/U04/U16 | Zone heating offset | ×0.1 °C, RW |
| 2095 | D05 | DHW setpoint | ×0.1 °C, RW |
| 2100 | — | Fault code (0 = none, else E-code) | R |
| 2210 / 2211 | — | Comfort / Eco heat setpoint | ×0.1 °C, RW |
| 2214 / 2215 | — | Comfort / Eco cool setpoint | ×0.1 °C, RW |
| 2216 | U07/U08/U18 | Humidity setting | %, RW |
| 2217 | — | Set-flow temperature | ×0.1 °C, RW |
| 2300 | D140–D142 | Boiler clock: [day of week 3 bit][hour 5 bit][minute 8 bit], 1 = Mon (CET, no DST) | R |
| 2302 / 2303 | D143–D145 | Controller date (day.month / year) | R |
| 2310–2347 | — | 4 calendar profiles (4 time ranges each, on/off) — hi byte = hour, lo byte = minute | RW |
| 2410–2416 / 2420–2426 / 2430–2436 | — | Day → calendar for Zone 1 / 2 / 3 (value 1–4) | RW |
| 2490–2496 | — | Day → calendar for DHW (single, shared house circuit) | RW |
| 3002 | D06 | External ambient temperature | ×0.1 °C |
| 3003 | D04 | Calculated flow setpoint (heating curve + U03; 0 without demand) | ×0.1 °C |
| 3011 / 3012 | D20 / D08 | Main generator (heat pump) flow / return | ×0.1 °C |
| 3013 / 3014 | D01 / — | Secondary generator (gas boiler) flow / return | ×0.1 °C |
| 3015 | D05 | **Active** DHW setpoint used by the boiler (e.g. 60 during anti-legionella) | ×0.1 °C |
| 3016 | D03 | DHW cylinder temperature | ×0.1 °C |
| 3042 | — | Panel firmware version | ×0.01 |
| 4199 (0x33) | A13 | Number of zones | — |
| 0x1E I3 / I4 / I5 / I6 | D73 / D74 / D79 / D72 | Compressor discharge / evaporator / outdoor probe / compressor shell | ×0.1 °C |
| 0x1E I11 / I13 / I14 / I16 | D71 / D75 / D76 / D77 | Compressor Hz / current (×0.1 A) / fan target rpm / EEV steps | — |
| 0x1E W20 / W21 / W22 | D08 / D20 / D24 | Water return (TW1) / flow (TW2) / refrigerant liquid, sent by the boiler to the HP | ×0.1 °C |

Addresses: `0x33` Dominus · `0x29` Zone 1 panel · `0x2A` Zone 2 · `0x2B` Zone 3 · `0x1E` outdoor-unit interface board (real, read-only). Full map with directions, confidence and unknowns: **[REGISTER_MAP.md](REGISTER_MAP.md)**.

## Configuration

Edit the `substitutions:` block at the top of the YAML — these point to **your** Home Assistant sensors:

- `temp_src_*` / `hum_src_*` — Zone 1 temperature/humidity sources (slots 2–5; slot 1 "Panel" = live bus value).
- `temp_src_z2_*` / `hum_src_z2_*`, `temp_src_z3_*` / `hum_src_z3_*` — Zone 2 / 3 sources.
- `author`, `github_url`, `project_version` — project metadata (shown as device info).

## Setup

1. `cp secrets.yaml.example secrets.yaml` and fill in WiFi + web UI credentials.
2. Point the `substitutions:` sources to your HA sensors (see above).
3. Flash `immergas-magis-pro-combo-dd-modbus.yaml` with ESPHome, wire the RS485 (with the **120 Ω** resistor), adopt in Home Assistant.

## Notes

- **Shared calendars:** the 4 time-profiles are shared across all zones and DHW; only the day→calendar assignment is per-zone (confirmed from the Dominus app config).
- **DHW is a single circuit** — one hot-water schedule for the whole house (no per-zone DHW).
- **Boiler fault descriptions** are the official Immergas English texts (from the Dominus app labels), not machine translation.

## Disclaimer

Reverse-engineered for **interoperability / personal use**. Not affiliated with or endorsed by Immergas. Emulating a controller and writing to registers changes appliance behaviour — **use at your own risk**.

## License

MIT — see [LICENSE](LICENSE).
