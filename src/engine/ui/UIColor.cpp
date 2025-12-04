/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** UIColor
*/

#include "UIColor.hpp"

namespace rtype::ui {
    UIColor::UIColor(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
    {
        this->_r = red;
        this->_g = green;
        this->_b = blue;
        this->_a = alpha;
    }

    UIColor::UIColor() : _r(0), _g(0), _b(0), _a(255)
    {
    }

    void UIColor::setRed(uint32_t r)
    {
        this->_r = r;
    }

    void UIColor::setGreen(uint32_t g)
    {
        this->_g = g;
    }

    void UIColor::setBlue(uint32_t b)
    {
        this->_b = b;
    }

    void UIColor::setAlpha(uint32_t a)
    {
        this->_a = a;
    }

    uint32_t UIColor::getRed() const
    {
        return this->_r;
    }

    uint32_t UIColor::getGreen() const
    {
        return this->_g;
    }

    uint32_t UIColor::getBlue() const
    {
        return this->_b;
    }

    uint32_t UIColor::getAlpha() const
    {
        return this->_a;
    }

    UIColor UIColor::getColor() const
    {
        UIColor color;
        color._r = static_cast<uint32_t>(this->_r);
        color._g = static_cast<uint32_t>(this->_g);
        color._b = static_cast<uint32_t>(this->_b);
        color._a = static_cast<uint32_t>(this->_a);
        return color;
    }

    UIColor& UIColor::operator=(const UIColor &other)
    {
        if (this != &other) {
            this->_r = other._r;
            this->_g = other._g;
            this->_b = other._b;
            this->_a = other._a;
        }
        return *this;
    }

    ::Color UIColor::toRaylib() const
    {
        return ::Color{
            static_cast<unsigned char>(_r),
            static_cast<unsigned char>(_g),
            static_cast<unsigned char>(_b),
            static_cast<unsigned char>(_a)
        };
    }

    UIColor UIColor::fromRaylib(const ::Color& color)
    {
        return UIColor(color.r, color.g, color.b, color.a);
    }

    UIColor UIColor::withAlpha(uint32_t a) const
    {
        return UIColor(_r, _g, _b, a);
    }

    UIColor UIColor::lerp(const UIColor& a, const UIColor& b, float t)
    {
        t = std::clamp(t, 0.0f, 1.0f);
        return UIColor(
            static_cast<uint32_t>(a._r + (b._r - a._r) * t),
            static_cast<uint32_t>(a._g + (b._g - a._g) * t),
            static_cast<uint32_t>(a._b + (b._b - a._b) * t),
            static_cast<uint32_t>(a._a + (b._a - a._a) * t)
        );
    }

} // namespace rtype::ui
