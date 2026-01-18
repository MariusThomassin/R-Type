/*
** Flappy Bird - PipeComponent
** Self-rendering pipe obstacle
** Implements IRenderable for the self-rendering pattern
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include "engine/graphics/IRenderable.hpp"
#include "engine/ecs/components/TransformComponent.hpp"

#include <raylib.h>
#include <string>

namespace flappy {

    /**
     * @brief Self-rendering pipe component for Flappy Bird obstacles
     * 
     * Renders classic green pipes with:
     * - Top or bottom orientation
     * - Decorative cap
     * - Scoring detection when passed
     */
    struct PipeComponent : public rtype::ecs::IComponent, public rtype::ecs::IRenderable {
        // Pipe properties
        float pipeWidth = 80.0f;
        float gapCenterY = 0.0f;     // Y position of gap center (only for score tracking)
        bool isTopPipe = false;       // Is this the top or bottom pipe
        bool scored = false;          // Has player passed this pipe (for scoring)
        bool isActive = true;         // For pooling
        
        // Visual properties
        unsigned char pipeR = 80, pipeG = 200, pipeB = 80;     // Green pipe
        unsigned char capR = 60, capG = 180, capB = 60;        // Darker cap
        unsigned char highlightR = 120, highlightG = 230, highlightB = 120; // Light edge
        
        int layer = 5;
        bool isVisible = true;
        
        // Dimensions (set during creation based on gap position)
        float pipeHeight = 0.0f;  // Height of this pipe section
        
        PipeComponent() = default;
        
        PipeComponent(bool top, float width, float height)
            : pipeWidth(width), isTopPipe(top), pipeHeight(height) {}
        
        std::string getTypeName() const override {
            return "PipeComponent";
        }
        
        // ==================== IRenderable Implementation ====================
        
        bool isRenderable() const override { return isVisible && isActive; }
        int getRenderLayer() const override { return layer; }
        
        void render(const rtype::ecs::TransformComponent& transform,
                   const rtype::ecs::RenderContext& /* ctx */) const override {
            float x = transform.x;
            float y = transform.y;
            
            Color pipeColor = {pipeR, pipeG, pipeB, 255};
            Color capColor = {capR, capG, capB, 255};
            Color highlightColor = {highlightR, highlightG, highlightB, 255};
            Color shadowColor = {
                static_cast<unsigned char>(pipeR * 0.6f),
                static_cast<unsigned char>(pipeG * 0.6f),
                static_cast<unsigned char>(pipeB * 0.6f),
                255
            };
            
            float capHeight = 30.0f;
            float capExtraWidth = 10.0f;
            
            if (isTopPipe) {
                // Top pipe: y is at 0, extends down to pipeHeight
                // Main pipe body
                DrawRectangle(
                    static_cast<int>(x),
                    static_cast<int>(y),
                    static_cast<int>(pipeWidth),
                    static_cast<int>(pipeHeight - capHeight),
                    pipeColor
                );
                
                // Cap at bottom of top pipe
                DrawRectangle(
                    static_cast<int>(x - capExtraWidth / 2),
                    static_cast<int>(y + pipeHeight - capHeight),
                    static_cast<int>(pipeWidth + capExtraWidth),
                    static_cast<int>(capHeight),
                    capColor
                );
                
                // Highlight on left edge
                DrawRectangle(
                    static_cast<int>(x + 5),
                    static_cast<int>(y),
                    static_cast<int>(10),
                    static_cast<int>(pipeHeight - capHeight),
                    highlightColor
                );
                
                // Shadow on right edge
                DrawRectangle(
                    static_cast<int>(x + pipeWidth - 15),
                    static_cast<int>(y),
                    static_cast<int>(10),
                    static_cast<int>(pipeHeight - capHeight),
                    shadowColor
                );
            } else {
                // Bottom pipe: y is the top of the pipe, extends down to screen bottom
                // Cap at top of bottom pipe
                DrawRectangle(
                    static_cast<int>(x - capExtraWidth / 2),
                    static_cast<int>(y),
                    static_cast<int>(pipeWidth + capExtraWidth),
                    static_cast<int>(capHeight),
                    capColor
                );
                
                // Main pipe body
                DrawRectangle(
                    static_cast<int>(x),
                    static_cast<int>(y + capHeight),
                    static_cast<int>(pipeWidth),
                    static_cast<int>(pipeHeight - capHeight),
                    pipeColor
                );
                
                // Highlight on left edge
                DrawRectangle(
                    static_cast<int>(x + 5),
                    static_cast<int>(y + capHeight),
                    static_cast<int>(10),
                    static_cast<int>(pipeHeight - capHeight),
                    highlightColor
                );
                
                // Shadow on right edge
                DrawRectangle(
                    static_cast<int>(x + pipeWidth - 15),
                    static_cast<int>(y + capHeight),
                    static_cast<int>(10),
                    static_cast<int>(pipeHeight - capHeight),
                    shadowColor
                );
            }
        }
    };

} // namespace flappy
