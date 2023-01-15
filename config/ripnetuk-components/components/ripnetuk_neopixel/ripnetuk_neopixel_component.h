#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/hal.h"

namespace esphome
{
  namespace ripnetuk_neopixel
  {

    class RipnetUkNeopixelComponent : public Component
    {
    public:
      void set_pin(GPIOPin *pin) { _pin = pin; }
      void set_pixel_count(int pixel_count) { _pixel_count = pixel_count; }
      void set_power_sensor(sensor::Sensor *power_sensor) { _power_sensor = power_sensor; }
      void loop() override;
      float get_setup_priority() const override;

    private:
      GPIOPin *_pin;
      int _pixel_count;
      sensor::Sensor *_power_sensor;
    };

  } // namespace debug
} // namespace esphome
