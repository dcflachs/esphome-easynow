#pragma once

#include "esphome/components/switch/switch.h"
// #include "esphome/components/template/number/template_number.h"
#include "esphome/core/component.h"
#include "../ripnetuk_ui/ripnetuk_switch_component.h"

#define LOG_INTERVAL 1000
#
namespace esphome
{
  namespace ripnetuk_clock
  {
    class RipnetUkClockComponent : public Component
    {
    private:
      float speed;
      // ripnetuk_ui::RipnetUkSwitchComponent *_ha_clock_reset = new ripnetuk_ui::RipnetUkSwitchComponent();
      switch_::Switch *_ha_clock_pause;
      //template_::Number *_ha_clock_speed;
      int _epoch_millis;
      int _last_log_millis;
      int _pause_millis;
      bool _paused;

    public:
      RipnetUkClockComponent();
      int time();
      void reset();
      void setPaused(bool is_paused);

      void setup() override;
      void loop() override;
      float get_setup_priority() const override;
    };

  } // namespace debug
} // namespace esphome

