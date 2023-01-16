#pragma once

#define ON_BRIGHTNESS 0.8

namespace esphome
{
    namespace ripnetuk_lightshow_core
    {
        class RGB
        {
        public:
            float r;
            float g;
            float b;
            float brightness;
            void set(RGB* rgb);
        };

        const RGB OFF_PIXEL = {0, 0, 0, 1};

        const RGB rangeRGBs[] = {
            {0, 0, ON_BRIGHTNESS, ON_BRIGHTNESS},
            {0, ON_BRIGHTNESS, 0, ON_BRIGHTNESS},
            {ON_BRIGHTNESS, ON_BRIGHTNESS, 0, ON_BRIGHTNESS},
            {ON_BRIGHTNESS, 0, 0, ON_BRIGHTNESS},
            {ON_BRIGHTNESS, 0, ON_BRIGHTNESS, ON_BRIGHTNESS},
            {0, ON_BRIGHTNESS, ON_BRIGHTNESS, ON_BRIGHTNESS},
            {ON_BRIGHTNESS, ON_BRIGHTNESS, ON_BRIGHTNESS, ON_BRIGHTNESS}};
    }
}