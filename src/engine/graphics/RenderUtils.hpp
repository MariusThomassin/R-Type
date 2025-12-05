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
     * @brief Utility class for component rendering
     * 
     * @note RenderUtils provides static helper functions for common rendering tasks
     * @note such as drawing UI elements, sprites, and effects.
     */
    class RenderUtils {
        public:
            /**
             * @brief Convert UIColor to raylib Color
             * @param c UIColor instance
             * @return raylib Color representation
             */
            static Color toRaylibColor(const rtype::ui::UIColor& c);

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
            static void drawGlow(float x, float y, float width, float height, unsigned char r, unsigned char g,
                unsigned char b, float intensity, float animTime);

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
            static void drawFallbackCircle(float x, float y, float size, unsigned char r, unsigned char g, unsigned char b, unsigned char a);

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
            static void drawSprite(const Texture2D& texture, float srcX, float srcY, float srcW, float srcH,
                float destX, float destY, float destW, float destH, float rotation, Color tint);

            /**
             * @brief Draw a UI rectangle
             * @param abs_t Absolute transform of the rectangle
             * @param color Fill color
             */
            static void drawUiRect(const rtype::ui::UITransform& abs_t, const rtype::ui::UIColor& color);

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
            static void drawUiText(const std::string& text, const rtype::ui::UITransform& abs_t, size_t font_size, const rtype::ui::UIColor& color,
                TextAlign align = TextAlign::Left, VerticalAlign verticalAlign = VerticalAlign::Top, float padding = 0.0f);

            /**
             * @brief Draw UI rectangle outline
             * @param abs_t Absolute transform for positioning
             * @param border_width Width of the border
             * @param color Border color
             */
            static void drawUiRectOutline(const rtype::ui::UITransform& abs_t, float border_width, const rtype::ui::UIColor& color);
    };

} // namespace rtype::ecs
