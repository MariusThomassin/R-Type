/*
** R-Type ECS - Color Palette Utilities
** Color tinting for sprites (bullets, effects, particles, etc.)
*/

#pragma once

#include "game/components/SpritesheetComponent.hpp"
#include "game/components/BulletTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Utility for applying color tints to sprites
     * Can be used for bullets, effects, particles, UI elements, etc.
     */
    struct ColorPalette {
        static void applyTint(SpritesheetComponent& sprite, BulletColor color);

        // Aliases for backward compatibility
        static void applyBulletColorTint(SpritesheetComponent& sprite, BulletColor color);

        static void getRGB(BulletColor color, uint8_t& r, uint8_t& g, uint8_t& b);

        /**
         * @brief Apply a custom RGB tint
         */
        static void applyCustomTint(SpritesheetComponent& sprite, uint8_t r, uint8_t g, uint8_t b);

        /**
         * @brief Blend two colors
         */
        static void blendColors(uint8_t r1, uint8_t g1, uint8_t b1,
                                uint8_t r2, uint8_t g2, uint8_t b2,
                                float t,
                                uint8_t& outR, uint8_t& outG, uint8_t& outB);
    };

    // Backward compatibility alias
    using BulletColors = ColorPalette;

} // namespace rtype::ecs
