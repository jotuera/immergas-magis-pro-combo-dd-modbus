#include "bus_climate.h"

#include <cmath>

#include "esphome/core/log.h"

namespace esphome {
namespace bus_climate {

static const char *const TAG = "bus_climate";

static bool same_float(float a, float b) {
  if (std::isnan(a) || std::isnan(b))
    return std::isnan(a) && std::isnan(b);
  return std::fabs(a - b) < 0.05f;
}

climate::ClimateTraits BusClimate::traits() {
  climate::ClimateTraits traits;
  traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_TEMPERATURE | climate::CLIMATE_SUPPORTS_ACTION);
  if (this->humidity_f_)
    traits.add_feature_flags(climate::CLIMATE_SUPPORTS_CURRENT_HUMIDITY);
  traits.add_supported_mode(climate::CLIMATE_MODE_OFF);
  for (auto mode : this->modes_)
    traits.add_supported_mode(mode);
  for (auto preset : this->presets_)
    traits.add_supported_preset(preset);
  return traits;
}

bool BusClimate::refresh_() {
  bool changed = false;
  if (this->current_f_) {
    auto v = this->current_f_();
    float nv = v.has_value() ? *v : NAN;
    if (!same_float(nv, this->current_temperature)) {
      this->current_temperature = nv;
      changed = true;
    }
  }
  if (this->target_f_) {
    auto v = this->target_f_();
    if (v.has_value() && !same_float(*v, this->target_temperature)) {
      this->target_temperature = *v;
      changed = true;
    }
  }
  if (this->mode_f_) {
    auto v = this->mode_f_();
    if (v.has_value() && *v != this->mode) {
      this->mode = *v;
      changed = true;
    }
  }
  if (this->humidity_f_) {
    auto v = this->humidity_f_();
    float nv = v.has_value() ? *v : NAN;
    if (!same_float(nv, this->current_humidity)) {
      this->current_humidity = nv;
      changed = true;
    }
  }
  if (this->preset_f_) {
    auto v = this->preset_f_();
    if (v.has_value()) {
      const std::string &p = *v;
      if (p == "COMFORT") {
        changed |= this->set_preset_(climate::CLIMATE_PRESET_COMFORT);
      } else if (p == "ECO") {
        changed |= this->set_preset_(climate::CLIMATE_PRESET_ECO);
      } else if (p == "HOME") {
        changed |= this->set_preset_(climate::CLIMATE_PRESET_HOME);
      } else if (p == "AWAY") {
        changed |= this->set_preset_(climate::CLIMATE_PRESET_AWAY);
      } else if (p == "BOOST") {
        changed |= this->set_preset_(climate::CLIMATE_PRESET_BOOST);
      } else if (p == "SLEEP") {
        changed |= this->set_preset_(climate::CLIMATE_PRESET_SLEEP);
      } else if (p == "ACTIVITY") {
        changed |= this->set_preset_(climate::CLIMATE_PRESET_ACTIVITY);
      } else if (p.empty() || p == "NONE") {
        changed |= this->set_preset_(climate::CLIMATE_PRESET_NONE);
      } else {
        changed |= this->set_custom_preset_(p.c_str(), p.size());
      }
    }
  }
  if (this->action_f_) {
    auto v = this->action_f_();
    if (v.has_value() && *v != this->action) {
      this->action = *v;
      changed = true;
    }
  }
  return changed;
}

void BusClimate::setup() {
  if (!this->custom_presets_.empty())
    this->set_supported_custom_presets(this->custom_presets_);
  this->refresh_();
  this->publish_state();
}

void BusClimate::update() {
  if (this->refresh_())
    this->publish_state();
}

void BusClimate::control(const climate::ClimateCall &call) {
  if (call.get_mode().has_value()) {
    climate::ClimateMode m = *call.get_mode();
    ESP_LOGD(TAG, "'%s': mode request %d", this->get_name().c_str(), (int) m);
    this->set_mode_trigger_.trigger(m);
  }
  if (call.get_target_temperature().has_value()) {
    float t = *call.get_target_temperature();
    ESP_LOGD(TAG, "'%s': target request %.1f", this->get_name().c_str(), t);
    this->set_target_trigger_.trigger(t);
  }
  if (call.get_preset().has_value()) {
    static const char *const NAMES[] = {"NONE", "HOME", "AWAY", "BOOST", "COMFORT", "ECO", "SLEEP", "ACTIVITY"};
    unsigned idx = (unsigned) *call.get_preset();
    std::string name = idx < sizeof(NAMES) / sizeof(NAMES[0]) ? NAMES[idx] : "NONE";
    ESP_LOGD(TAG, "'%s': preset request %s", this->get_name().c_str(), name.c_str());
    this->set_preset_trigger_.trigger(name);
  } else if (call.has_custom_preset()) {
    std::string name(call.get_custom_preset().c_str(), call.get_custom_preset().size());
    ESP_LOGD(TAG, "'%s': custom preset request %s", this->get_name().c_str(), name.c_str());
    this->set_preset_trigger_.trigger(name);
  }
  // Re-read the bus-backed state and always publish, so HA drops any optimistic value
  // the device did not accept (e.g. an unsupported mode change).
  this->refresh_();
  this->publish_state();
}

void BusClimate::dump_config() { LOG_CLIMATE("", "Bus Climate", this); }

}  // namespace bus_climate
}  // namespace esphome
