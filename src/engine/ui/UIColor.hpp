/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UIColor
*/

#ifndef UICOLOR_HPP_
#define UICOLOR_HPP_

#include <cstdint>
#include <raylib.h>
#include <algorithm>

// Forward declare raylib Color to avoid header dependency
struct Color;

namespace rtype::ui {
    class UIColor {
        public:
            UIColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
            UIColor();

            ~UIColor() = default;

            void setRed(uint32_t r);
            void setGreen(uint32_t g);
            void setBlue(uint32_t b);
            void setAlpha(uint32_t a);

            uint32_t getRed() const;
            uint32_t getGreen() const;
            uint32_t getBlue() const;
            uint32_t getAlpha() const;

            UIColor getColor() const;

            UIColor& operator=(const UIColor &other);

            ::Color toRaylib() const;
            static UIColor fromRaylib(const ::Color& color);

            UIColor withAlpha(uint32_t a) const;
            static UIColor lerp(const UIColor& a, const UIColor& b, float t);

            static UIColor White() { return UIColor(255, 255, 255, 255); }
            static UIColor Black() { return UIColor(0, 0, 0, 255); }
            static UIColor Red() { return UIColor(255, 0, 0, 255); }
            static UIColor Green() { return UIColor(0, 255, 0, 255); }
            static UIColor Blue() { return UIColor(0, 0, 255, 255); }
            static UIColor Yellow() { return UIColor(255, 255, 0, 255); }
            static UIColor Transparent() { return UIColor(0, 0, 0, 0); }
            static UIColor Gray() { return UIColor(128, 128, 128, 255); }
            static UIColor DarkGray() { return UIColor(64, 64, 64, 255); }
            static UIColor LightGray() { return UIColor(192, 192, 192, 255); }

        private:
            uint32_t _r;
            uint32_t _g;
            uint32_t _b;
            uint32_t _a;
    };
} // namespace rtype::ui

#endif /* !UICOLOR_HPP_ */
