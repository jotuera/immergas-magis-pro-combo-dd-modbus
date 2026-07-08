# Immergas Magis Pro / Combo — Dominus / Panel emulator on the D+/D- Modbus bus (ESPHome, M5 Atom)

Control an **Immergas Magis Combo** boiler from **Home Assistant** by **emulating the Dominus remote and the zone panels** on the **D+/D- (Modbus RS485)** bus — with an **M5Stack Atom + ESPHome**. No cloud, no proprietary Dominus controller.

The boiler is the Modbus **master**; this device is a **slave** that answers as the Dominus (`0x33`) and/or the zone panels (`0x29` / `0x2A` / `0x2B`). It feeds the boiler room temperature & humidity from **any Home Assistant sensor** and exposes the panel/Dominus settings as HA entities.

> ⚠️ **Tested only on the Magis Combo V2 (`MPROCOMBOV2`).** The Dominus app supports many Immergas engines (Zeus, Star, VictrixMaior, …) with different register maps — this project targets **Magis Combo only**. Other models may partially work but are untested. Contributions welcome.

## What it does

- **Emulates the Dominus** (`0x33`) and/or **zone panels** (`0x29` = Zone 1, `0x2A` = Zone 2, `0x2B` = Zone 3) so the boiler runs without the original controller.
- **Feeds room temperature & humidity** to the boiler from HA sensors — pick from up to 5 switchable sources per zone (Panel / Basement / Ground floor / Upper floor / Whole house). "Panel" uses the real value read from a physical panel on the bus, if present.
- **Controls**: operating mode, zone CH setpoint, heating offset (U03/U04/U16), Comfort/Eco heat & cool setpoints, set-flow, humidity setting (U07/U08/U18), DHW setpoint.
- **Weekly schedules (chrono)**: 4 shared time-profile calendars (`Calendar 1–4`) + independent day→calendar assignment per zone and for DHW.
- **Monitors**: boiler fault code + **114 fault descriptions in official Immergas English** (extracted from the Dominus app labels), per-zone phase (Comfort/Eco), connection status of each device, firmware versions.
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
- Address **`0x1E` (machine room)** is a **real device** — it is only monitored read-only and **never transmitted to**.

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
| 2302 / 2303 | — | Controller date (day.month / year) | R |
| 2310–2347 | — | 4 calendar profiles (4 time ranges each, on/off) — hi byte = hour, lo byte = minute | RW |
| 2410–2416 / 2420–2426 / 2430–2436 | — | Day → calendar for Zone 1 / 2 / 3 (value 1–4) | RW |
| 2490–2496 | — | Day → calendar for DHW (single, shared house circuit) | RW |
| 3002 | D06 | External ambient temperature | ×0.1 °C |
| 3016 | D03 | DHW cylinder temperature | ×0.1 °C |
| 3042 | — | Panel firmware version | ×0.01 |

Addresses: `0x33` Dominus · `0x29` Zone 1 panel · `0x2A` Zone 2 · `0x2B` Zone 3 · `0x1E` machine room (real, monitor-only).

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
