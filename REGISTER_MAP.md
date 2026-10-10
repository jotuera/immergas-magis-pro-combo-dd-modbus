# Register map — Immergas Magis Combo, D+/D- bus

**Device:** Immergas **Magis Combo 9 Plus V2** (main board firmware **6.0**). RS-485 bus **D+/D-**, **9600 8E1**, **120 Ω termination required**.
**Roles:** the **boiler is the Modbus master**; every other device is a slave. The boiler cyclically reads (func 03/04) and writes (func 06/16).
**Sources:** passive sniffing (47 logs, 218 registers), the v1.2.0 bus sniffer, cross-checks against the **T-/T+ BMS** bus and the **Samsung NASA F1/F2** bus, the Magis Combo manual (EN rev. 8.0 / PL rev. 5.0), the zone remote panel manual **3.030863**, and the Dominus app screen map (`CFG-WFC01_IM_MBUS_MPROCOMBOV2.json`).

Confidence: **CONFIRMED** = verified live (correlation / second bus / panel reading), **MANUAL** = matched by the manual's description, **CAND.** = strong candidate, **?** = unknown.
Direction: **R** = boiler reads from the device (func 03), **I** = boiler reads an input register (func 04), **W** = boiler writes to the device (func 06/16).
Manual codes: in the PDF manuals the code column is **shifted against the descriptions** (line wrapping) and numbering differs between firmware versions — codes are matched **by description**; numbers verified live are marked.

## Addresses
| Address | Device | Notes |
|---|---|---|
| **0x1E** (30) | **Outdoor-unit interface board** ("A22" in the wiring diagram; Samsung **MIM**, OEM mode) | always present; links the Immergas board to the Samsung NASA bus. Link status = manual code **D62**. Never transmit to it. |
| **0x29** (41) | Zone 1 remote panel | physical or emulated; missing → **E121** |
| **0x2A / 0x2B** (42/43) | Zone 2 / 3 panels | only with A13 > 1 |
| **0x33** (51) | **Dominus** (Wi-Fi module) | missing → **E142** |

## 0x1E — outdoor-unit interface board (heat pump)
Verified on **three buses** (D+/D-, T-/T+, NASA) at standby and during a forced compressor run. Temperatures ×0.1 °C.

| Reg | Dir | Meaning | Code | NASA / T-T+ | Scale | Confidence |
|---|---|---|---|---|---|---|
| 0 | W | **Heat-pump run request** (0/1) | — | NASA Capacity Request | bit | CONFIRMED |
| 1 | R/W | **Water target for the HP.** R1 = stable target (= D04 on demand, **5.0 °C idle**). W1 is unusual: every cycle (~2 s) it alternates **0 / 50** (func 06 request/echo with different values); the real target is written only when it changes → use R1/I10 | — | NASA Water Outlet Target | ×0.1 °C | CONFIRMED (R1) |
| 2–6 | R | R2/R3/R5/R6 = 0, R4 = 1 (constant) | — | — | — | ? |
| 15–18 | R | constant 5 / 30 / 10 / 30 | T21–T24 (screed drying)? | — | — | CAND. |
| 3 | I | **Compressor discharge temperature** | **D73** | NASA 820A, T-T+ 4554 | ×0.1 °C | CONFIRMED |
| 4 | I | **Evaporator temperature** (outdoor coil) | **D74** (live) | NASA 8218, T-T+ D74 | ×0.1 °C | CONFIRMED |
| 5 | I | **Outdoor-unit probe temperature — RAW** (no P07 correction); the boiler forwards it ~2 s later as 3002 **after applying P07** | **D79** | NASA 420C | ×0.1 °C | CONFIRMED |
| 6 | I | **Compressor temperature** (shell/top) | **D72** | NASA 8280, T-T+ 4585 | ×0.1 °C | CONFIRMED |
| 10 | I | **Water target** — acknowledged by the HP | — | Water Outlet Target | ×0.1 °C | CONFIRMED |
| 11 | I | **Compressor frequency** | **D71** (live) | NASA Compressor Freq, T-T+ D71 | Hz | CONFIRMED |
| 12 | I | constant 0 (standby and CH run) | — | — | — | ? |
| 13 | I | **Compressor current** (inverter estimate) | **D75** | NASA Outdoor Current | ×0.1 A | CONFIRMED |
| 14 | I | **Fan target speed** | **D76** (panel probably ÷10) | NASA Fan target, T-T+ 4587 | rpm | CONFIRMED |
| 15 | I | constant 0 | — | — | — | ? |
| 16 | I | **EEV position** (0–2000) | **D77** | NASA EEV, T-T+ D77 | steps | CONFIRMED |
| 17 | I | **Target discharge temperature** (61–70 in summer = DHW) | — | NASA Target Discharge | ×0.1 °C | CONFIRMED |
| 18 / 21 | I | constant 1 | D78 (4-way valve)? | — | — | CAND. |
| 19 | I | **Operating stage**: 1 standby, 2 starting, 3 running | D80? | = NASA Start Stage 3 / 6 | enum | CONFIRMED (code CAND.) |
| 20 | I | constant 0 | — | — | — | ? |
| 22–29 | I | **Firmware versions** (register = 2 parts as decimal pairs): 22/23 outdoor main board (19.07.04.01), **24/25 outdoor inverter (10.00.02.01)**, 26/27 inverter memory (19.04.10.01), **28/29 interface board MIM (20.02.11.01)**. Register order ≠ manual code order. MIM/inverter assignment: NASA 20.00.01 0x0608 (DB91-02164A) + MIM MCU sticker "D2164A 200211"; NASA 10.00.00 0x8601 (DB91-02338A) | D120–D135 (MANUAL) | NASA 0x0608 / 0x8601 | — | CONFIRMED (modules) |
| 36 | I | **Flag: compressor running** | — | NASA Compressor Status (to the second) | bit | CONFIRMED |
| 37 | I | constant 0 | — | — | — | ? |
| 38 | I | **Flag: fan running** (drops ~1 min after the compressor) | — | — | bit | CONFIRMED |
| 40 | I | **Flag: request accepted** (together with W0) | — | — | bit | CONFIRMED |
| 41 / 42 | I | constant 0 / 1 | D78? | — | — | CAND. |
| 44–46 | I | constant 0 | — | — | — | ? |
| 5 | W | constant 0 (boiler command) | — | — | — | ? |
| 20 | W | **Water return temperature (TW1)** — the boiler supplies it to Samsung (the outdoor unit has no water probes) | D08 (live) | NASA Flow Temp Return | ×0.1 °C | CONFIRMED |
| 21 | W | **Water flow temperature (TW2)** | D20 (live) | NASA 4204 TW2 | ×0.1 °C | CONFIRMED |
| 22 | W | **Refrigerant liquid temperature** | D24 (live) | NASA EVA In | ×0.1 °C | CONFIRMED |

0x1E does **not** carry the actual fan speed, power or DC-link voltage (only on NASA F1/F2). **D78** (4-way valve HT/CL) is not located yet — candidates are the constant flags 18/21/41/42 (should flip in cooling/defrost).

## 0x29 — Zone 1 panel
### Boiler → panel (W): this block is the panel's "Information" menu, 1:1 (manual 3.030863)
| Reg | Meaning | Code | Scale | Confidence |
|---|---|---|---|---|
| 2000 | Operating mode (0 standby, 1 summer DHW, 2 cooling, 3 winter CH+DHW) | — | enum | CONFIRMED |
| 2001 | **Status flags** (bitfield): **bit 3 = always 1**; **bit 0 (8→9) = DHW request pending** — set ~9 min before every DHW cycle, cleared when the 3-way valve switches to DHW (15/15 cases); **bit 2 (8→12) = boiler anomaly active (2100 ≠ 0)** — matches to the second both E193 "Device in test mode" (service menu M) and E121 "Zone 1 panel missing". Dominus: `mb-water-request` | bit 0 = D09 (DHW request) | bitfield | CONFIRMED |
| 2010 | Zone status / phase | — | bitfield | CONFIRMED |
| 2015 / 2016 / 2017 | Zone setpoint / max / offset | — | ×0.1 °C | CONFIRMED |
| **2090** | **Time-program state** (boiler reply to 2294, ~5 s later): **4 = manual (bit 2), 64 = auto / comfort phase (bit 6), 128 = auto / economy phase (bit 7)** (64 → 128 seen 3 s after the boiler clock was moved past the end of a comfort window). Whether it tracks the Zone 1 CH or the DHW program is still open | — | bitfield | CONFIRMED (link), CAND. (scope) |
| 2100 | Fault code (0 = none, e.g. 142 = Dominus missing) | — | enum | CONFIRMED |
| 2218 / 2219 | Zone max / offset (copies) | — | ×0.1 °C | CONFIRMED |
| 2293 | DHW temperature (copy) | — | ×0.1 °C | CONFIRMED |
| **2300** | **Boiler clock**: [day of week 3 b][hour 5 b][minute 8 b], 1 = Mon … 7 = Sun; the boiler runs on CET (no DST) | **D140/D141/D142** | — | CONFIRMED |
| **2302** | **Date**: high byte = day, low byte = month | **D143/D144** | — | CONFIRMED |
| **2303** | **Year** | **D145** | — | CONFIRMED |
| **3002** | Outdoor temperature **after the P07 probe correction** (P07 = +5 K → 3002 jumped 10.5 → 15.5 while the raw 0x1E I5 stayed) | D06 | ×0.1 °C | CONFIRMED |
| **3003** | **Calculated flow setpoint** (heating curve + U03; 0 without demand) | **D04** (live) | ×0.1 °C | CONFIRMED |
| 3009 / 3010 | Room temperature / humidity (panel echo) | — | ×0.1 °C / % | CONFIRMED |
| **3011** | **Main generator (heat pump) flow** | **D20** (live) | ×0.1 °C | CONFIRMED |
| **3012** | **Main generator return** | **D08** (live) | ×0.1 °C | CONFIRMED |
| **3013** | **Secondary generator (gas boiler) flow** ("Flow temp. 2"; DHW by gas → 65.6 °C) | D01 (first manual entry, unverified) | ×0.1 °C | CONFIRMED (code MANUAL) |
| **3014** | **Secondary generator return** ("Return temp. 2") | no D code | ×0.1 °C | CONFIRMED |
| **3015** | **Active DHW setpoint** used by the boiler (60 during anti-legionella) | D05 | ×0.1 °C | CONFIRMED |
| 3016 | DHW cylinder temperature | D03 | ×0.1 °C | CONFIRMED |
| 3031 | Board firmware version (600 = 6.00) | D91 | ×0.01 | CONFIRMED |
| **6102** | **A31 "Zone 1 room thermostat"** passed to the panel: **RPT = 0, RP = 2** (RT probably 1). Changing A31 RPT → RP switched 6102 0 → 2, and back RP → RPT 2 → 0 (both directions verified) | **A31** | enum | CONFIRMED (RT = CAND.) |
| 6150 | constant 1 | — | — | ? |

### Panel → boiler (R)
| Reg | Meaning | Confidence |
|---|---|---|
| 0 / 1 | 233 / 60 — probably a legacy copy of room temperature / humidity | CAND. |
| 2000 | Operating mode | CONFIRMED |
| 2005 / 2006 | **Room temperature / humidity** (the only values the boiler takes from the panel for control) | CONFIRMED |
| 2015–2017 | Setpoint / max / offset | CONFIRMED |
| 2095, 2290–2292 | DHW setpoint (+ copies) | CONFIRMED |
| 2103 | constant 0 | ? |
| **2294** | **Panel "time programs active"**: −1 = not set, **0 = off (manual), 1 = auto**. Verified both ways; on panel connect the boiler writes 0 | CONFIRMED |
| **6105** | **Panel "room-probe modulation" setting** (Service → Zone definition): −1 = not set (fresh panel), **0 = NO, 1 = YES**. On panel connect the boiler writes 0. Verified YES → 1 and NO → 0 | CONFIRMED |
| **6107** | **Panel "dew-point setpoint correction"** (1 = on, 0 = off). Turning it off and on again on the panel switched 6107 1 → 0 → 1 (both directions verified) | CONFIRMED |
| 2210 / 2211 | Comfort / Eco heating | CONFIRMED |
| 2214 / 2215 / 2216 | Comfort / Eco cooling, humidity setting | CONFIRMED |
| 2218 / 2219 | Max / offset (copies) | CONFIRMED |
| 2310–2347 | Calendars 1–4 (on/off profiles, shared by all zones) | CONFIRMED |
| 2410–2416 | Day → calendar — Zone 1 | CONFIRMED |
| 2490–2496 | Day → calendar — DHW | CONFIRMED |
| 2707 | Eco cooling (copy) | CONFIRMED |
| 3042 | Panel firmware version (201 = 2.01) | CONFIRMED |

**Set-flow 2217 is read only from the Dominus (0x33)** — from the panel the boiler reads 2214–2216 and 2218–2219, deliberately skipping 2217, so a panel (physical or emulated) cannot control set-flow.

### Heating curve → D04 (3003) → HP water target (0x1E W1/I10)
Verified with a forced test (U03 changes):
`D04 = R05 + (R03 − T_out) / (R03 − R02) × (R04 − R05) + U03`, clamped to [R05, R04]; T_out = **0x1E I5** (outdoor-unit probe).
With R02 = −12, R03 = 20, R05 = 24, R04 = 35 and T_out 15.5 °C: U03 = 0 → **25.5** (measured 255), U03 = +4 → **29.5** (295), T_out 15.3 → **29.6** (296, 1 s after I5 changed), U03 = −2 → 23.5 → clamped to **24.0** (R05). **U03 shifts the curve 1:1.** U03 appears on the bus as 0x33 2017 / 2218 (×0.1 °C).

## 0x33 — Dominus
| Reg | Dir | Meaning | Confidence |
|---|---|---|---|
| 2000 | R/W | Operating mode | CONFIRMED |
| 2001–2002 | W | Dominus presence probe [12, 0] / status flags | CONFIRMED |
| 2010 / 2011 | W | Zone status / room temperature (32767 = none) | CONFIRMED |
| 2012 | W | constant 0 | ? |
| 2015–2017 | R/W | Zone 1 setpoint / max / offset (2017 = U03) | CONFIRMED |
| 2020–2025 / 2030–2035 / 2040–2045 | R | Status, temperature and setpoint of zones 2 / 3 / 4 (Dominus map) | CONFIRMED |
| 2095 | R/W | DHW setpoint | CONFIRMED |
| 2100 / 2101 | W | Fault code / reset | CONFIRMED |
| 2210–2218 | R/W | Comfort/Eco heat & cool, humidity, **set-flow 2217**, offset 2218 | CONFIRMED |
| 2220–2228 / 2230–2238 / 2240–2248 | R/W | Same for zones 2 / 3 / 4 | CONFIRMED |
| 2310–2347 | R/W | Calendars 1–4 | CONFIRMED |
| 2410–2416 / 2420–2426 / 2430–2436 / 2440–2446 | R/W | Day → calendar, zones 1 / 2 / 3 / 4 | CONFIRMED |
| 2490–2496 | R/W | Day → calendar, DHW | CONFIRMED |
| 3002 / 3016 | W | Outdoor temperature / DHW temperature | CONFIRMED |
| 3031 | W | Firmware version (60 = 6.0) | CONFIRMED |
| **4199** | W | **Number of zones** (Dominus `mb-number-zones`) | **A13** — CONFIRMED |

## 0x2A / 0x2B — Zone 2 / 3 panels
Same register set as 0x29 (2005/2006, 2015–2017, 2210–2216, …), per-zone values. Active only with A13 > 1.

## Bus-related faults
- **E121** — Zone 1 device (panel 0x29) missing: the boiler blocks Zone 1 control; changes made on the Dominus do not hold.
- **E142** — Dominus (0x33) missing.

## Anti-legionella (P15)
Parameter **P15** (anti-legionella enable; P13 duration, P16 hour, P17 day) is **not transmitted on D+/D-** — comparing sniffer baselines with P15 = ON and P15 = OFF showed no constant register change besides the DHW setpoint. Only the **effect** is visible: the boiler's active DHW setpoint in **3015** goes up during the cycle. Up to v1.1.0 the emulator adopted 3015 as a user setpoint and fed it back as the panel, leaving the setpoint stuck at 60 °C — fixed in v1.2.0.

## Still unknown — help welcome
- 0x1E: I12, I15, I20, I37, I44–46, W5 (always 0), R2–6, R15–18; flags I18/21/41/42 — **D78** candidates (needs cooling or defrost data).
- **Registers the boiler READS from panel 0x29**: 2000, 2005–2006, 2015–2017, 2095, 2103, 2210–2211, 2214–2216, 2218–2219, 2290–2292, 2294, 2310–2347, 2410–2416, 2490–2496, 2707, 3042, 6105, 6107. Only these can be controlled by an emulated panel; **6102 (A31) is never read**, so A31 cannot be set over the bus.
- 0x29: 2103, 6150 (2001: bits 0 and 2 decoded, bit 3 always 1; 6102 = A31).
- 0x33: 2012.

The **bus sniffer** switch in the firmware logs every register change (log tag `sniff`) — a capture during cooling, defrost or heat-pump DHW would help close the remaining gaps.

## Integration / service parameters and D+/D-
Changing the gas/heat-pump integration parameters (I01–I10), A05, A12, A18, P07–P24, T02–T23, U11 or entering service menu M on the boiler panel does **not** change any D+/D- register, except: **2001 bit 2 + fault 2100 = E193** while in test mode (menu M), and **3002** when the outdoor-probe correction **P07** is changed. Integration parameters live on the T-/T+ BMS bus (see the sister project `immergas-magis-combo-tt-bms`).
