/*
** Flappy Bird - GroundComponent
** Self-rendering scrolling ground
** Implements IRenderable for the self-rendering pattern
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include "engine/graphics/IRenderable.hpp"
#include "engine/ecs/components/TransformComponent.hpp"

#include <raylib.h>
#include <string>
#include <cmath>

namespace flappy {

    /**
     * @brief Self-rendering ground component for Flappy Bird
     * 
     * Renders a scrolling ground with:
     * - Grass top layer
     * - Dirt body
     * - Scrolling animation
     */
    struct GroundComponent : public rtype::ecs::IComponent, public rtype::ecs::IRenderable {
        float scrollOffset = 0.0f;
        float scrollSpeed = 200.0f;
        float groundHeight = 100.0f;
        int screenWidth = 1280;
        
        // Visual properties
        unsigned char grassR = 80, grassG = 180, grassB = 80;
        unsigned char dirtR = 180, dirtG = 140, dirtB = 80;
        
        int layer = 15;  // In front of pipes
        bool isVisible = true;
        
        GroundComponent() = default;
        
        GroundComponent(int width, float height, float speed)
            : scrollSpeed(speed), groundHeight(height), screenWidth(width) {}
        
        std::string getTypeName() const override {
            return "GroundComponent";
        }
        
        void updateScroll(float dt) {
            scrollOffset += scrollSpeed * dt;
            // Reset scroll when one "tile" has passed (assume 64px tiles)
            if (scrollOffset >= 64.0f) {
                scrollOffset -= 64.0f;
            }
        }
        
        // ==================== IRenderable Implementation ====================
        
        bool isRenderable() const override { return isVisible; }
        int getRenderLayer() const override { return layer; }
        
        void render(const rtype::ecs::TransformComponent& transform,
                   const rtype::ecs::RenderContext& /* ctx */) const override {
            float y = transform.y;
            
            Color grassColor = {grassR, grassG, grassB, 255};
            Color dirtColor = {dirtR, dirtG, dirtB, 255};
            Color grassDark = {
                static_cast<unsigned char>(grassR * 0.7f),
                static_cast<unsigned char>(grassG * 0.7f),
                static_cast<unsigned char>(grassB * 0.7f),
                255
            };
            
            float grassHeight = 15.0f;
            
            // Draw dirt
            DrawRectangle(
                0,
                static_cast<int>(y + grassHeight),
                screenWidth,
                static_cast<int>(groundHeight - grassHeight),
                dirtColor
            );
            
            // Draw grass
            DrawRectangle(
                0,
                static_cast<int>(y),
                screenWidth,
                static_cast<int>(grassHeight),
                grassColor
            );
            
            // Draw grass pattern (stripes that scroll)
            float stripeWidth = 64.0f;
            int numStripes = static_cast<int>(screenWidth / stripeWidth) + 2;
            for (int i = 0; i < numStripes; i++) {
                float stripeX = i * stripeWidth - scrollOffset;
                if (i % 2 == 0) {
                    DrawRectangle(
                        static_cast<int>(stripeX),
                        static_cast<int>(y),
                        static_cast<int>(stripeWidth / 2),
                        static_cast<int>(grassHeight),
                        grassDark
                    );
                }
            }
        }
    };

} // namespace flappy
