#pragma once

#include <functional>
#include <string>
#include <vector>

#include "esphome/core/automation.h"
#include "esphome/core/component.h"
#include "esphome/components/climate/climate.h"

namespace esphome {
namespace bus_climate {

// Climate entity whose state comes entirely from the bus (via lambdas reading globals):
// it never runs its own control logic. HA commands are forwarded to YAML automations,
// which write the globals the emulator serves; the next update() reflects the result.
class BusClimate : public climate::Climate, public PollingComponent {
 public:
  void set_current_lambda(std::function<optional<float>()> f) { this->current_f_ = std::move(f); }
  void set_target_lambda(std::function<optional<float>()> f) { this->target_f_ = std::move(f); }
  void set_mode_lambda(std::function<optional<climate::ClimateMode>()> f) { this->mode_f_ = std::move(f); }
  void set_action_lambda(std::function<optional<climate::ClimateAction>()> f) { this->action_f_ = std::move(f); }
  void add_supported_mode(climate::ClimateMode mode) { this->modes_.push_back(mode); }
  void set_humidity_lambda(std::function<optional<float>()> f) { this->humidity_f_ = std::move(f); }
  void set_preset_lambda(std::function<optional<std::string>()> f) { this->preset_f_ = std::move(f); }
  void add_supported_preset(climate::ClimatePreset preset) { this->presets_.push_back(preset); }
  void add_custom_preset(const char *name) { this->custom_presets_.push_back(name); }
  Trigger<std::string> *get_set_preset_trigger() { return &this->set_preset_trigger_; }

  Trigger<float> *get_set_target_trigger() { return &this->set_target_trigger_; }
  Trigger<climate::ClimateMode> *get_set_mode_trigger() { return &this->set_mode_trigger_; }

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::LATE; }

 protected:
  climate::ClimateTraits traits() override;
  void control(const climate::ClimateCall &call) override;
  bool refresh_();

  std::function<optional<float>()> current_f_;
  std::function<optional<float>()> target_f_;
  std::function<optional<climate::ClimateMode>()> mode_f_;
  std::function<optional<climate::ClimateAction>()> action_f_;
  std::vector<climate::ClimateMode> modes_;
  std::function<optional<float>()> humidity_f_;
  std::function<optional<std::string>()> preset_f_;
  std::vector<climate::ClimatePreset> presets_;
  std::vector<const char *> custom_presets_;  // string literals from codegen (static storage)
  Trigger<std::string> set_preset_trigger_;
  Trigger<float> set_target_trigger_;
  Trigger<climate::ClimateMode> set_mode_trigger_;
};

}  // namespace bus_climate
}  // namespace esphome
