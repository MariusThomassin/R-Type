/*
** R-Type ECS - RenderUtils
** Shared rendering utilities for self-rendering components
*/

#pragma once

#include <raylib.h>
#include <cmath>
#include <algorithm>
#include "../ui/UIColor.hpp"
#include "../ui/Widget.hpp"

namespace rtype::ecs {

    /**
     * @brief Text alignment options for rendering
     */
    enum class TextAlign {
        Left,
        Center,
        Right
    };

    /**
     * @brief Vertical alignment options for rendering
     */
    enum class VerticalAlign {
        Top,
        Middle,
        Bottom
    };
}

namespace rtype::ecs {

    // Forward declarations
    struct TransformComponent;
    struct RenderContext;

    /**
     * @brief Utility functions for component rendering
     */
    namespace RenderUtils {
        inline Color toRaylibColor(const rtype::ui::UIColor& c) {
            return {
                (unsigned char)c.getRed(), 
                (unsigned char)c.getGreen(), 
                (unsigned char)c.getBlue(), 
                (unsigned char)c.getAlpha()
            };
        }

        /**
         * @brief Draw a pulsing glow effect
         */
        inline void drawGlow(float x, float y, float width, float height,
                            unsigned char r, unsigned char g, unsigned char b,
                            float intensity, float animTime) {
            rtype::ui::UIColor glowColor = {r, g, b, static_cast<unsigned char>(80 * intensity)};

            float pulse = 1.0f + 0.2f * std::sin(animTime * 8.0f);
            float glowRadius = std::max(width, height) * 0.8f * pulse;

            DrawCircle(static_cast<int>(x), static_cast<int>(y), glowRadius, glowColor.toRaylib());

            // Inner glow
            rtype::ui::UIColor innerGlow = {
                static_cast<unsigned char>(std::min(255, r + 50)),
                static_cast<unsigned char>(std::min(255, g + 50)),
                static_cast<unsigned char>(std::min(255, b + 50)),
                static_cast<unsigned char>(60 * intensity)
            };
            DrawCircle(static_cast<int>(x), static_cast<int>(y), glowRadius * 0.5f, innerGlow.toRaylib());
        }

        /**
         * @brief Draw a fallback circle when texture is unavailable
         */
        inline void drawFallbackCircle(float x, float y, float size,
                                       unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
            rtype::ui::UIColor color = {r, g, b, a};
            rtype::ui::UIColor coreColor = {255, 255, 255, 200};

            DrawCircle(static_cast<int>(x), static_cast<int>(y), size * 0.5f, color.toRaylib());
            DrawCircle(static_cast<int>(x), static_cast<int>(y), size * 0.25f, coreColor.toRaylib());
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

        inline void drawUiRect(const rtype::ui::UITransform& abs_t, const rtype::ui::UIColor& color) {
            DrawRectangle(
                (int)abs_t.x, (int)abs_t.y, 
                (int)abs_t.width, (int)abs_t.height, 
                toRaylibColor(color)
            );
        }

        inline void drawUiText(const std::string& text, const rtype::ui::UITransform& abs_t, size_t font_size, const rtype::ui::UIColor& color, TextAlign align = TextAlign::Left, VerticalAlign verticalAlign = VerticalAlign::Top, float padding = 0.0f) {

            int textWidth = MeasureText(text.c_str(), static_cast<int>(font_size));
            int textHeight = static_cast<int>(font_size);

            // Calculate X position based on alignment
            float textX = abs_t.x + padding;
            float availableWidth = abs_t.width - 2 * padding;

            switch (align) {
                case TextAlign::Center:
                    textX = abs_t.x + (abs_t.width - textWidth) / 2.0f;
                    break;
                case TextAlign::Right:
                    textX = abs_t.x + abs_t.width - textWidth - padding;
                    break;
                case TextAlign::Left:
                default:
                    break;
            }

            // Calculate Y position based on vertical alignment
            float textY = abs_t.y + padding;
            float availableHeight = abs_t.height - 2 * padding;

            switch (verticalAlign) {
                case VerticalAlign::Middle:
                    textY = abs_t.y + (abs_t.height - textHeight) / 2.0f;
                    break;
                case VerticalAlign::Bottom:
                    textY = abs_t.y + abs_t.height - textHeight - padding;
                    break;
                case VerticalAlign::Top:
                default:
                    break;
            }

            DrawText(text.c_str(), static_cast<int>(textX), static_cast<int>(textY), static_cast<int>(font_size), toRaylibColor(color));
        }

        inline void drawUiRectOutline(const rtype::ui::UITransform& abs_t, float border_width, const rtype::ui::UIColor& color) {
            DrawRectangleLinesEx(
                (Rectangle){abs_t.x, abs_t.y, abs_t.width, abs_t.height}, 
                border_width,
                toRaylibColor(color)
            );
        }

    } // namespace RenderUtils

} // namespace rtype::ecs
