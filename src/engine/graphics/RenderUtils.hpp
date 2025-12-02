/*
** R-Type ECS - RenderUtils
** Shared rendering utilities for self-rendering components
*/

#pragma once

#include <raylib.h>
#include <cmath>
#include <algorithm>

namespace rtype::ecs {

    // Forward declarations
    struct TransformComponent;
    struct RenderContext;

    /**
     * @brief Utility functions for component rendering
     */
    namespace RenderUtils {

        /**
         * @brief Draw a pulsing glow effect
         */
        inline void drawGlow(float x, float y, float width, float height,
                            unsigned char r, unsigned char g, unsigned char b,
                            float intensity, float animTime) {
            Color glowColor = {r, g, b, static_cast<unsigned char>(80 * intensity)};

            float pulse = 1.0f + 0.2f * std::sin(animTime * 8.0f);
            float glowRadius = std::max(width, height) * 0.8f * pulse;

            DrawCircle(static_cast<int>(x), static_cast<int>(y), glowRadius, glowColor);

            // Inner glow
            Color innerGlow = {
                static_cast<unsigned char>(std::min(255, r + 50)),
                static_cast<unsigned char>(std::min(255, g + 50)),
                static_cast<unsigned char>(std::min(255, b + 50)),
                static_cast<unsigned char>(60 * intensity)
            };
            DrawCircle(static_cast<int>(x), static_cast<int>(y), glowRadius * 0.5f, innerGlow);
        }

        /**
         * @brief Draw a fallback circle when texture is unavailable
         */
        inline void drawFallbackCircle(float x, float y, float size,
                                       unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
            Color color = {r, g, b, a};
            Color coreColor = {255, 255, 255, 200};

            DrawCircle(static_cast<int>(x), static_cast<int>(y), size * 0.5f, color);
            DrawCircle(static_cast<int>(x), static_cast<int>(y), size * 0.25f, coreColor);
        }

        /**
         * @brief Draw a textured sprite with rotation
         */
        inline void drawSprite(const Texture2D& texture,
                              float srcX, float srcY, float srcW, float srcH,
                              float destX, float destY, float destW, float destH,
                              float rotation, Color tint) {
            Rectangle source = {srcX, srcY, srcW, srcH};
            Rectangle dest = {destX, destY, destW, destH};
            Vector2 origin = {destW / 2.0f, destH / 2.0f};
            DrawTexturePro(texture, source, dest, origin, rotation, tint);
        }

    } // namespace RenderUtils

} // namespace rtype::ecs
