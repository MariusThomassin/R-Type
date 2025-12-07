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
    /**
     * @brief RGBA color representation
     * 
     * UIColor encapsulates color data with red, green, blue, and alpha
     * components. It provides utility methods for conversion to/from raylib's
     * Color struct, color interpolation, and common color constants.
     */
    class UIColor {
        public:
            /**
             * @brief Construct a new UIColor object
             * @param r Red component (0-255)
             * @param g Green component (0-255)
             * @param b Blue component (0-255)
             * @param a Alpha component (0-255)
             */
            UIColor(uint32_t r, uint32_t g, uint32_t b, uint32_t a);
            /**
             * @brief Construct a new default UIColor (black, opaque)
             * @note Defaults to (0, 0, 0, 255)
             */
            UIColor();

            ~UIColor() = default;

            /**
             * @brief Set the Red component
             * @param r Red value (0-255)
             */
            void setRed(uint32_t r);
            /**
             * @brief Set the Green component
             * @param g Green value (0-255)
             */
            void setGreen(uint32_t g);
            /**
             * @brief Set the Blue component
             * @param b Blue value (0-255)
             */
            void setBlue(uint32_t b);
            /**
             * @brief Set the Alpha component
             * @param a Alpha value (0-255)
             */
            void setAlpha(uint32_t a);

            /**
             * @brief Get the Red component
             * @return Red value (0-255)
             */
            uint32_t getRed() const;
            /**
             * @brief Get the Green component
             * @return Green value (0-255)
             */
            uint32_t getGreen() const;
            /**
             * @brief Get the Blue component
             * @return Blue value (0-255)
             */
            uint32_t getBlue() const;
            /**
             * @brief Get the Alpha component
             * @return Alpha value (0-255)
             */
            uint32_t getAlpha() const;

            /**
             * @brief Get a copy of this color
             * @return A new UIColor instance with the same RGBA values
             */
            UIColor getColor() const;

            /**
             * @brief Assignment operator
             * @param other The other UIColor to copy from
             * @return Reference to this UIColor
             */
            UIColor& operator=(const UIColor &other);

            /**
             * @brief Convert to raylib Color struct
             * @return raylib Color representation
             */
            ::Color toRaylib() const;
            static UIColor fromRaylib(const ::Color& color);

            /**
             * @brief Create a new UIColor with modified alpha
             * @param a New alpha value (0-255)
             * @return New UIColor with updated alpha
             */
            UIColor withAlpha(uint32_t a) const;
            /**
             * @brief Linearly interpolate between two colors
             * @param a Starting color
             * @param b Ending color
             * @param t Interpolation factor (0.0 to 1.0)
             * @return Interpolated UIColor
             */
            static UIColor lerp(const UIColor& a, const UIColor& b, float t);

            // Common color constants
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
            /**
             * @brief Color components R = red
             */
            uint32_t _r;
            /**
             * @brief Color components G = green
             */
            uint32_t _g;
            /**
             * @brief Color components B = blue
             */
            uint32_t _b;
            /**
             * @brief Color components A = alpha
             */
            uint32_t _a;
    };
} // namespace rtype::ui

#endif /* !UICOLOR_HPP_ */
