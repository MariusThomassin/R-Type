/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Color
*/

#ifndef COLOR_HPP_
#define COLOR_HPP_

#include <cstdint>

// Forward declare raylib Color to avoid header dependency
struct Color;

namespace rtype::ui {
    class Color {
        public:
            Color(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
            Color();
            Color(const Color& other);

            ~Color() = default;

            void setRed(uint32_t r);
            void setGreen(uint32_t g);
            void setBlue(uint32_t b);
            void setAlpha(uint32_t a);

            uint32_t getRed() const;
            uint32_t getGreen() const;
            uint32_t getBlue() const;
            uint32_t getAlpha() const;

            Color& operator=(const Color &other);

            ::Color toRaylib() const;
            static Color fromRaylib(const ::Color& color);

            Color withAlpha(uint32_t a) const;
            static Color lerp(const Color& a, const Color& b, float t);

            static Color White() { return Color(255, 255, 255, 255); }
            static Color Black() { return Color(0, 0, 0, 255); }
            static Color Red() { return Color(255, 0, 0, 255); }
            static Color Green() { return Color(0, 255, 0, 255); }
            static Color Blue() { return Color(0, 0, 255, 255); }
            static Color Yellow() { return Color(255, 255, 0, 255); }
            static Color Transparent() { return Color(0, 0, 0, 0); }
            static Color Gray() { return Color(128, 128, 128, 255); }
            static Color DarkGray() { return Color(64, 64, 64, 255); }
            static Color LightGray() { return Color(192, 192, 192, 255); }

        private:
            uint32_t _r;
            uint32_t _g;
            uint32_t _b;
            uint32_t _a;
    };
} // namespace rtype::ui

#endif /* !COLOR_HPP_ */
