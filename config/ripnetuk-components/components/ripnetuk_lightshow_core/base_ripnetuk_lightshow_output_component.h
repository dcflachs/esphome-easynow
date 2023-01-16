#pragma once

#include "esphome/core/component.h"
#include "frame.h"

namespace esphome
{
  namespace ripnetuk_lightshow_core
  {
    class BaseRipnetUkLightshowOutputComponent : public Component
    {
    public:
      virtual void output_frame(ripnetuk_lightshow_core::Frame *frame) = 0;

    private:
    };

  } // namespace ripnetuk_lightshow_core
} // namespace esphome
