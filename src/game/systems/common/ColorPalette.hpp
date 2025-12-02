/*
** R-Type ECS - Color Palette Utilities
** Color tinting for sprites (bullets, effects, particles, etc.)
*/

#pragma once

#include "../../components/SpritesheetComponent.hpp"
#include "../../components/BulletTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Utility for applying color tints to sprites
     * Can be used for bullets, effects, particles, UI elements, etc.
     */
    struct ColorPalette {
        static void applyTint(SpritesheetComponent& sprite, BulletColor color) {
            switch (color) {
                case BulletColor::Red:
                    sprite.tintR = 255; sprite.tintG = 80; sprite.tintB = 80;
                    break;
                case BulletColor::Orange:
                    sprite.tintR = 255; sprite.tintG = 160; sprite.tintB = 50;
                    break;
                case BulletColor::Yellow:
                    sprite.tintR = 255; sprite.tintG = 240; sprite.tintB = 80;
                    break;
                case BulletColor::Green:
                    sprite.tintR = 80; sprite.tintG = 255; sprite.tintB = 80;
                    break;
                case BulletColor::Cyan:
                    sprite.tintR = 80; sprite.tintG = 240; sprite.tintB = 255;
                    break;
                case BulletColor::Blue:
                    sprite.tintR = 80; sprite.tintG = 120; sprite.tintB = 255;
                    break;
                case BulletColor::Purple:
                    sprite.tintR = 180; sprite.tintG = 80; sprite.tintB = 255;
                    break;
                case BulletColor::Magenta:
                    sprite.tintR = 255; sprite.tintG = 80; sprite.tintB = 200;
                    break;
                case BulletColor::White:
                    sprite.tintR = 255; sprite.tintG = 255; sprite.tintB = 255;
                    break;
                case BulletColor::Black:
                    sprite.tintR = 60; sprite.tintG = 60; sprite.tintB = 80;
                    break;
                default:
                    sprite.tintR = 255; sprite.tintG = 255; sprite.tintB = 255;
                    break;
            }
        }

        // Aliases for backward compatibility
        static void applyBulletColorTint(SpritesheetComponent& sprite, BulletColor color) {
            applyTint(sprite, color);
        }

        static void getRGB(BulletColor color, uint8_t& r, uint8_t& g, uint8_t& b) {
            switch (color) {
                case BulletColor::Red:     r = 255; g = 80;  b = 80;  break;
                case BulletColor::Orange:  r = 255; g = 160; b = 50;  break;
                case BulletColor::Yellow:  r = 255; g = 240; b = 80;  break;
                case BulletColor::Green:   r = 80;  g = 255; b = 80;  break;
                case BulletColor::Cyan:    r = 80;  g = 240; b = 255; break;
                case BulletColor::Blue:    r = 80;  g = 120; b = 255; break;
                case BulletColor::Purple:  r = 180; g = 80;  b = 255; break;
                case BulletColor::Magenta: r = 255; g = 80;  b = 200; break;
                case BulletColor::White:   r = 255; g = 255; b = 255; break;
                case BulletColor::Black:   r = 60;  g = 60;  b = 80;  break;
                default:                   r = 255; g = 255; b = 255; break;
            }
        }

        /**
         * @brief Apply a custom RGB tint
         */
        static void applyCustomTint(SpritesheetComponent& sprite, uint8_t r, uint8_t g, uint8_t b) {
            sprite.tintR = r;
            sprite.tintG = g;
            sprite.tintB = b;
        }

        /**
         * @brief Blend two colors
         */
        static void blendColors(uint8_t r1, uint8_t g1, uint8_t b1,
                                uint8_t r2, uint8_t g2, uint8_t b2,
                                float t,
                                uint8_t& outR, uint8_t& outG, uint8_t& outB) {
            outR = static_cast<uint8_t>(r1 + (r2 - r1) * t);
            outG = static_cast<uint8_t>(g1 + (g2 - g1) * t);
            outB = static_cast<uint8_t>(b1 + (b2 - b1) * t);
        }
    };

    // Backward compatibility alias
    using BulletColors = ColorPalette;

} // namespace rtype::ecs
