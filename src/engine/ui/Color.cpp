/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Color
*/

#include "Color.hpp"
#include <raylib.h>
#include <algorithm>

namespace rtype::ui {
    Color::Color(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
    {
        this->_r = red;
        this->_g = green;
        this->_b = blue;
        this->_a = alpha;
    }

    Color::Color() : _r(0), _g(0), _b(0), _a(255)
    {
    }

    Color::Color(const Color &color)
    {
        *this = color;
    }

    void Color::setRed(uint32_t r)
    {
        this->_r = r;
    }

    void Color::setGreen(uint32_t g)
    {
        this->_g = g;
    }

    void Color::setBlue(uint32_t b)
    {
        this->_b = b;
    }

    void Color::setAlpha(uint32_t a)
    {
        this->_a = a;
    }

    uint32_t Color::getRed() const
    {
        return this->_r;
    }

    uint32_t Color::getGreen() const
    {
        return this->_g;
    }

    uint32_t Color::getBlue() const
    {
        return this->_b;
    }

    uint32_t Color::getAlpha() const
    {
        return this->_a;
    }

    Color& Color::operator=(const Color &other)
    {
        if (this != &other) {
            this->_r = other._r;
            this->_g = other._g;
            this->_b = other._b;
            this->_a = other._a;
        }
        return *this;
    }

    ::Color Color::toRaylib() const
    {
        return ::Color{
            static_cast<unsigned char>(_r),
            static_cast<unsigned char>(_g),
            static_cast<unsigned char>(_b),
            static_cast<unsigned char>(_a)
        };
    }

    Color Color::fromRaylib(const ::Color& color)
    {
        return Color(color.r, color.g, color.b, color.a);
    }

    Color Color::withAlpha(uint32_t a) const
    {
        return Color(_r, _g, _b, a);
    }

    Color Color::lerp(const Color& a, const Color& b, float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        return Color(
            static_cast<uint32_t>(a._r + (b._r - a._r) * t),
            static_cast<uint32_t>(a._g + (b._g - a._g) * t),
            static_cast<uint32_t>(a._b + (b._b - a._b) * t),
            static_cast<uint32_t>(a._a + (b._a - a._a) * t)
        );
    }

} // namespace rtype::ui
