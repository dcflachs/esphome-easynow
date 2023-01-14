#pragma once

#include "esphome/components/switch/switch.h"
#include "esphome/core/component.h"
#include "../ripnetuk_ui/ripnetuk_switch_component.h"
#include "../ripnetuk_ui/ripnetuk_button_component.h"
#include "../ripnetuk_ui/ripnetuk_number_component.h"

#define LOG_INTERVAL 1000

namespace esphome
{
  namespace ripnetuk_clock
  {
    class RipnetUkClockSwitch;

    class RipnetUkClockComponent : public Component
    {
    private:
      ripnetuk_ui::RipnetUkSwitchComponent *_ha_clock_pause_switch = new ripnetuk_ui::RipnetUkSwitchComponent("Clock Pause", this->_paused, [this](bool state)
                                                                                                              { this->setPaused(state); });
      ripnetuk_ui::RipnetUkButtonComponent *_ha_clock_reset_button = new ripnetuk_ui::RipnetUkButtonComponent("Clock Reset", [this]
                                                                                                              { this->reset(); });
      ripnetuk_ui::RipnetUkNumberComponent *_ha_clock_speed_number = new ripnetuk_ui::RipnetUkNumberComponent("Clock Speed", this->_speed, number::NUMBER_MODE_BOX, 0, 4, 0.05, [this](float state)
                                                                                                              { this->_speed = state; });
      // template_::Number *_ha_clock_speed;
      int _epoch_millis;
      int _last_log_millis;
      int _pause_millis;

      float _speed{1};
      bool _paused{false};

    public:
      int time();
      void reset();
      void setPaused(bool is_paused);

      void setup() override;
      void loop() override;
      float get_setup_priority() const override;
    };
  } // namespace debug
} // namespace esphome
