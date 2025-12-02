/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Color
*/

#ifndef COLOR_HPP_
#define COLOR_HPP_

#include <cstdint>

namespace rtype::ui {
    class Color {
        public:
            Color(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
            Color();

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

        private:
            uint32_t _r;
            uint32_t _g;
            uint32_t _b;
            uint32_t _a;
    };
} // namespace rtype::ui

#endif /* !COLOR_HPP_ */
