# Translating

Entity names and the texts shown in Home Assistant come from one language file in `packages/lang/`:

| File | Language | Status |
|---|---|---|
| `dd_en.yaml` | English | complete (default) |
| `dd_pl.yaml` | Polski | complete |
| `dd_it.yaml` | Italiano | texts (`t_dd_*`) and fault descriptions translated, entity names (`n_dd_*`) still English |

## How it works

Each file is a block of `substitutions:`:

- `n_dd_<id>` - name of the entity with that `id` (keep the `Immergas - ` prefix and the manual codes such as `D71`, `U03`, `A31`).
- `t_dd_*` - texts shown as states or select options (calendars, operating modes, zone phase, compressor stage, ...).
- `dd_fault_language` - default of the "Fault language" select (`English`, `Italiano` or `Polski`).

Fault descriptions are in `components/immergas_dd/faults_i18n.h` (one column per language).

## Adding or completing a language

1. Copy `packages/lang/dd_en.yaml` to `packages/lang/dd_<code>.yaml` and translate the values (not the keys).
2. Keep select texts unique inside each select (the code compares them).
3. Validate with `esphome config` using your file in the `packages:` list, then open a pull request.

Note: the entity name is part of the entity identity in Home Assistant - after a language change HA creates new entities.
