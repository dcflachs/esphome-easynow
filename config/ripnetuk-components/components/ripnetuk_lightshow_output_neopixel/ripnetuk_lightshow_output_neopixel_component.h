#pragma once

#include "esphome/core/component.h"
#include "../ripnetuk_ui/ripnetuk_number_component.h"
#include "../ripnetuk_clock/ripnetuk_clock_component.h"
#include <Adafruit_NeoPixel.h>
#include "../ripnetuk_lightshow_core/RGB.h"
#include "../ripnetuk_lightshow_core/ripnetuk_lightshow_core_component.h"
#include "../ripnetuk_lightshow_core/base_ripnetuk_lightshow_output_component.h"
#include "esphome/core/hal.h"

namespace esphome
{
  namespace ripnetuk_lightshow_output_neopixel
  {
    class RipnetUkLightshowOutputNeopixelComponent : public ripnetuk_lightshow_core::BaseRipnetUkLightshowOutputComponent
    {
    public:
      void setup() override;
      void loop() override;
      float get_setup_priority() const override;
      void set_lightshow_core(ripnetuk_lightshow_core::RipnetUkLightshowCoreComponent *core) { _core = core; }
      void set_pin(GPIOPin *pin) { _pin = pin; }

    private:
      GPIOPin *_pin;
      ripnetuk_lightshow_core::RipnetUkLightshowCoreComponent *_core;

      ripnetuk_ui::RipnetUkNumberComponent *_ha_brightness = new ripnetuk_ui::RipnetUkNumberComponent("Brightness", number::NUMBER_MODE_BOX, 0, 1, 0.001);

      Adafruit_NeoPixel *_neoPixel;
      int scaleToByte(double value, double brightness);
      void showPixels();
    };

  } // namespace debug
} // namespace esphome
