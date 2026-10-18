#pragma once
// Arbitration between HA commands and values seen on the bus for the registers the emulated
// Dominus serves (setpoints, offset, levels, mode).
//
// Why: the same register travels on several frames (boiler -> 0x33 WRITE10, 0x29 READ reply,
// boiler -> 0x29 WRITE10) and during a change they disagree for a few seconds, e.g.
//   WRITE10 0x33 = new  ->  READ 0x29 = old (panel not updated yet)  ->  WRITE10 0x29 = new
// A plain "adopt anything that differs" follows every flip and can serve the old value back
// to the boiler, which reads a value different from its last write as a Dominus command.
// Rules, in order:
//  0. Implausible values (outside the register's range, e.g. 0xFFFF = -1 or a 0 °C setpoint)
//     are never adopted.
//  1. Panel warm-up: for WARMUP_MS after the panel (0x29) starts replying again (panel
//     reconnected, power cut, Atom boot) its READ replies are ignored. A freshly started panel
//     answers with placeholders (2015 = 0, 2017 = 0, 2095 = 0xFFFF) until the boiler has
//     written the real values to it; adopting them made the boiler apply DHW 10 °C / offset 0.
//  2. HA command -> pending: bus values != requested are ignored until the bus confirms the
//     requested value, or PENDING_MS expires (then the bus wins again).
//  3. After any change (adopted value, or confirmed command) the register settles for SETTLE_MS:
//     differing values in the panel's READ replies are lagging copies and are ignored.
//     Writes from the boiler (to 0x33 or 0x29) are never held back by the settle window: they
//     carry the boiler's own value, and ignoring one would leave us serving a value different
//     from it, which the boiler would read back as a Dominus command.
//  4. Otherwise a value that differs from the local one is adopted (change made on the panel).
// The bus repeats every register every few seconds, so an ignored genuine value is picked up
// on a later frame once the windows expire.

#include <climits>
#include <cstdint>
#include "esphome/core/hal.h"
#include "esphome/core/log.h"

namespace lww {

enum Key {
  ZONE1_SETPOINT,
  ZONE1_OFFSET,
  DHW_SETPOINT,
  COMFORT_HEAT,
  ECO_HEAT,
  COMFORT_COOL,
  ECO_COOL,
  HUMIDITY,
  SET_FLOW,
  OPERATING_MODE,
  KEY_COUNT
};

enum Verdict { SAME, ADOPT, CONFIRMED, IGNORE_PENDING, IGNORE_STALE, IGNORE_RANGE, IGNORE_WARMUP };

static const int NONE = INT_MIN;
static const uint32_t PENDING_MS = 15000;
static const uint32_t SETTLE_MS = 6000;
static const uint32_t WARMUP_MS = 60000;
static const uint32_t PANEL_GAP_MS = 10000;  // no panel reply for this long = panel was offline

// Accepted raw range per key (x10 for temperatures). Generous on purpose: only placeholders and
// garbage are excluded, never a value the boiler or the panel can legitimately hold.
struct Range {
  int lo, hi;
};
static const Range RANGES[KEY_COUNT] = {
    {50, 350},    // ZONE1_SETPOINT  5.0 .. 35.0 °C
    {-150, 150},  // ZONE1_OFFSET   -15 .. +15 K
    {100, 650},   // DHW_SETPOINT   10.0 .. 65.0 °C
    {50, 350},    // COMFORT_HEAT
    {50, 350},    // ECO_HEAT
    {50, 350},    // COMFORT_COOL
    {50, 350},    // ECO_COOL
    {0, 100},     // HUMIDITY %
    {100, 900},   // SET_FLOW       10 .. 90 °C
    {0, 3},       // OPERATING_MODE
};

struct Slot {
  int pending = NONE;
  uint32_t pending_ms = 0;
  uint32_t changed_ms = 0;
};

inline Slot &slot(Key k) {
  static Slot slots[KEY_COUNT];
  return slots[k];
}

struct Panel {
  uint32_t last_reply_ms = 0;
  uint32_t back_ms = 0;  // when the panel started replying again
  bool seen = false;
};

inline Panel &panel() {
  static Panel p;
  return p;
}

// Call on every READ reply from the physical panel (0x29).
inline void panel_reply() {
  Panel &p = panel();
  uint32_t now = esphome::millis();
  if (!p.seen || now - p.last_reply_ms > PANEL_GAP_MS) {
    p.back_ms = now;
    ESP_LOGW("dd_lww", "Panel 0x29 replying again: its values are ignored for %u s (warm-up)",
             (unsigned) (WARMUP_MS / 1000));
  }
  p.seen = true;
  p.last_reply_ms = now;
}

inline bool panel_warming_up() {
  const Panel &p = panel();
  return p.seen && esphome::millis() - p.back_ms < WARMUP_MS;
}

// HA command. Call BEFORE overwriting the served value: old = current served value.
inline void local_write(Key k, int old_value, int new_value) {
  (void) old_value;
  Slot &s = slot(k);
  uint32_t now = esphome::millis();
  s.pending = new_value;
  s.pending_ms = now;
  s.changed_ms = now;
}

// Value seen on the bus. current = value served now; addr = device the frame belongs to;
// is_write = the boiler wrote it (WRITE06/WRITE10), otherwise it is a READ reply from a device.
inline Verdict bus_value(Key k, int current, int v, uint8_t addr, bool is_write) {
  Slot &s = slot(k);
  uint32_t now = esphome::millis();
  if (v < RANGES[k].lo || v > RANGES[k].hi) return IGNORE_RANGE;
  bool panel_reply = !is_write && addr == 0x29;
  if (panel_reply && panel_warming_up()) return IGNORE_WARMUP;
  if (s.pending != NONE) {
    if (v == s.pending) {
      s.pending = NONE;
      s.changed_ms = now;  // lagging copies of older values may still follow the confirmation
      return v == current ? CONFIRMED : ADOPT;
    }
    if (now - s.pending_ms < PENDING_MS) return IGNORE_PENDING;
    s.pending = NONE;  // not confirmed in time: the bus wins
  }
  if (v == current) return SAME;
  if (panel_reply && s.changed_ms != 0 && now - s.changed_ms < SETTLE_MS) return IGNORE_STALE;
  s.changed_ms = now;
  return ADOPT;
}

inline void log_verdict(const char *name, Verdict vd, const char *source, uint8_t addr, int current, int v) {
  static const char *const TAG = "dd_lww";
  switch (vd) {
    case ADOPT:
      ESP_LOGW(TAG, "ADOPT %s from %s 0x%02X: %d -> %d", name, source, addr, current, v);
      break;
    case CONFIRMED:
      ESP_LOGI(TAG, "CONFIRMED %s = %d by %s 0x%02X", name, v, source, addr);
      break;
    case IGNORE_PENDING:
      ESP_LOGW(TAG, "IGNORE %s = %d from %s 0x%02X (command %d pending)", name, v, source, addr, current);
      break;
    case IGNORE_STALE:
      ESP_LOGW(TAG, "IGNORE %s = %d from %s 0x%02X (settling, current %d)", name, v, source, addr, current);
      break;
    case IGNORE_RANGE:
      ESP_LOGW(TAG, "IGNORE %s = %d from %s 0x%02X (out of range, current %d)", name, v, source, addr, current);
      break;
    case IGNORE_WARMUP:
      if (v != current)
        ESP_LOGW(TAG, "IGNORE %s = %d from %s 0x%02X (panel warm-up, current %d)", name, v, source, addr, current);
      break;
    default:
      break;
  }
}

// Bus value -> verdict + log. True when the caller must store v and publish it.
inline bool adopt(Key k, const char *name, const char *source, uint8_t addr, int current, int v) {
  bool is_write = source != nullptr && source[0] == 'W';  // "WRITE06" / "WRITE10" vs "READ"
  Verdict vd = bus_value(k, current, v, addr, is_write);
  log_verdict(name, vd, source, addr, current, v);
  return vd == ADOPT;
}

}  // namespace lww