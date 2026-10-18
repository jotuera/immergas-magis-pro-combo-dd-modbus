# Translating

Entity names and the texts shown in Home Assistant come from one language file in `packages/lang/`:

| File | Language | Status |
|---|---|---|
| `en.yaml` | English | complete (default) |
| `pl.yaml` | Polski | complete |
| `it.yaml` | Italiano | texts (`t_*`) and fault descriptions translated, entity names (`n_*`) still English |

## How it works

Each file is a block of `substitutions:`:

- `n_<id>` - name of the entity with that `id` (keep the `Immergas - ` prefix and the manual codes such as `D71`, `U03`, `A31`).
- `t_*` - texts shown as states or select options (calendars, operating modes, zone phase, compressor stage, ...).
- `fault_language` - default of the "Fault language" select (`English`, `Italiano` or `Polski`).

Fault descriptions are in `components/immergas_dd/faults_i18n.h` (one column per language).

## Adding or completing a language

1. Copy `packages/lang/en.yaml` to `packages/lang/<code>.yaml` and translate the values (not the keys).
2. Keep select texts unique inside each select (the code compares them).
3. Validate with `esphome config` using your file in the `packages:` list, then open a pull request.

Note: the entity name is part of the entity identity in Home Assistant - after a language change HA creates new entities.
