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

            Color getColor() const;

            UIColor& operator=(const UIColor &other);

        private:
            uint32_t _r;
            uint32_t _g;
            uint32_t _b;
            uint32_t _a;
    };
} // namespace rtype::ui

#endif /* !UICOLOR_HPP_ */
