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

    Color UIColor::getColor() const
    {
        Color color;
        color.r = static_cast<unsigned char>(this->_r);
        color.g = static_cast<unsigned char>(this->_g);
        color.b = static_cast<unsigned char>(this->_b);
        color.a = static_cast<unsigned char>(this->_a);
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

} // namespace rtype::ui
