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
     * @note Left aligns to the left edge
     * @note Center centers horizontally
     * @note Right aligns to the right edge
     */
    enum class TextAlign {
        Left,
        Center,
        Right
    };

    /**
     * @brief Vertical alignment options for rendering
     * @note Top aligns to the top edge
     * @note Middle centers vertically
     * @note Bottom aligns to the bottom edge
     */
    enum class VerticalAlign {
        Top,
        Middle,
        Bottom
    };
}

namespace rtype::ecs {

    /**
     * @brief Forward declarations
     */
    struct TransformComponent;
    /**
     * @brief Rendering context passed to renderable components
     */
    struct RenderContext;

    /**
     * @brief Utility functions for component rendering
     */
    namespace RenderUtils {
        /**
         * @brief Convert UIColor to raylib Color
         * @param c UIColor instance
         * @return raylib Color representation
         */
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
         * @param x X position
         * @param y Y position
         * @param width Width of the glow area
         * @param height Height of the glow area
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @param intensity Glow intensity (0.0 to 1.0)
         * @param animTime Animation time for pulsing effect
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
         * @param x X position
         * @param y Y position
         * @param size Diameter of the circle
         * @param r Red component (0-255)
         * @param g Green component (0-255)
         * @param b Blue component (0-255)
         * @param a Alpha component (0-255)
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
         * @param texture Texture to draw
         * @param srcX Source rectangle X
         * @param srcY Source rectangle Y
         * @param srcW Source rectangle width
         * @param srcH Source rectangle height
         * @param destX Destination X position
         * @param destY Destination Y position
         * @param destW Destination width
         * @param destH Destination height
         * @param rotation Rotation angle in degrees
         * @param tint Color tint to apply
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

        /**
         * @brief Draw a UI rectangle
         * @param abs_t Absolute transform of the rectangle
         * @param color Fill color
         */
        inline void drawUiRect(const rtype::ui::UITransform& abs_t, const rtype::ui::UIColor& color) {
            DrawRectangle(
                (int)abs_t.x, (int)abs_t.y, 
                (int)abs_t.width, (int)abs_t.height, 
                toRaylibColor(color)
            );
        }

        /**
         * @brief Draw UI text with alignment options
         * @param text Text string to draw
         * @param abs_t Absolute transform for positioning
         * @param font_size Font size in points
         * @param color Text color
         * @param align Horizontal text alignment
         * @param verticalAlign Vertical text alignment
         * @param padding Padding from edges
         */
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

        /**
         * @brief Draw a UI rectangle outline
         * @param abs_t Absolute transform of the rectangle
         * @param border_width Width of the border
         * @param color Border color
         */
        inline void drawUiRectOutline(const rtype::ui::UITransform& abs_t, float border_width, const rtype::ui::UIColor& color) {
            DrawRectangleLinesEx(
                (Rectangle){abs_t.x, abs_t.y, abs_t.width, abs_t.height}, 
                border_width,
                toRaylibColor(color)
            );
        }

    } // namespace RenderUtils

} // namespace rtype::ecs
