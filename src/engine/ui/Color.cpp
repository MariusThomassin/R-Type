/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Color
*/

#include "Color.hpp"

namespace rtype::ui {
    Color::Color(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha)
    {
        this->_r = red;
        this->_g = green;
        this->_b = blue;
        this->_a = alpha;
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

} // namespace rtype::ui
