/*
** Flappy Bird - BirdComponent
** Self-rendering pixel-art bird with flapping animation
** Implements IRenderable for the self-rendering pattern
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include "engine/graphics/IRenderable.hpp"
#include "engine/ecs/components/TransformComponent.hpp"

#include <raylib.h>
#include <cmath>
#include <string>

namespace flappy {

    /**
     * @brief Self-rendering bird component for Flappy Bird game
     * 
     * Renders a cute pixel-art bird with:
     * - Animated wings based on velocity
     * - Rotation based on vertical velocity
     * - Color customization
     */
    struct BirdComponent : public rtype::ecs::IComponent, public rtype::ecs::IRenderable {
        // Physics properties
        float gravity = 800.0f;
        float flapForce = -350.0f;
        float maxFallSpeed = 600.0f;
        
        // State
        bool isAlive = true;
        bool hasStarted = false;  // Game hasn't started until first flap
        
        // Visual properties
        unsigned char bodyR = 255, bodyG = 220, bodyB = 50;   // Yellow body
        unsigned char wingR = 255, wingG = 180, wingB = 20;   // Darker wing
        unsigned char beakR = 255, beakG = 120, beakB = 50;   // Orange beak
        
        int layer = 10;
        float pixelScale = 3.0f;
        bool isVisible = true;
        
        // Animation
        float flapAnimTime = 0.0f;
        
        BirdComponent() = default;
        
        BirdComponent(unsigned char r, unsigned char g, unsigned char b)
            : bodyR(r), bodyG(g), bodyB(b) {}
        
        std::string getTypeName() const override {
            return "BirdComponent";
        }
        
        /**
         * @brief Update animation state
         */
        void updateAnimation(float dt, float velocityY) {
            flapAnimTime += dt;
            // Wing flaps faster when going up
            if (velocityY < 0) {
                flapAnimTime += dt * 2.0f;
            }
        }
        
        // ==================== IRenderable Implementation ====================
        
        bool isRenderable() const override { return isVisible && isAlive; }
        int getRenderLayer() const override { return layer; }
        
        void render(const rtype::ecs::TransformComponent& transform,
                   const rtype::ecs::RenderContext& ctx) const override {
            if (!isAlive && static_cast<int>(ctx.animTime * 10.0f) % 2 == 0) {
                return;  // Blink when dead
            }
            
            float x = transform.x;
            float y = transform.y;
            float s = pixelScale * transform.scaleX;
            
            // Rotation could be used for tilting the bird based on velocity
            (void)transform.rotation;  // Currently unused
            
            // Colors
            Color bodyColor = {bodyR, bodyG, bodyB, 255};
            Color wingColor = {wingR, wingG, wingB, 255};
            Color beakColor = {beakR, beakG, beakB, 255};
            Color eyeWhite = {255, 255, 255, 255};
            Color eyeBlack = {0, 0, 0, 255};
            
            // Draw bird body (circle-ish shape)
            // Main body
            DrawCircle(static_cast<int>(x), static_cast<int>(y), 12.0f * s / 3.0f, bodyColor);
            
            // Wing (animates up and down)
            int wingOffset = static_cast<int>(std::sin(flapAnimTime * 15.0f) * 3.0f * s / 3.0f);
            DrawEllipse(
                static_cast<int>(x - 4 * s / 3.0f),
                static_cast<int>(y + wingOffset),
                8.0f * s / 3.0f,
                5.0f * s / 3.0f,
                wingColor
            );
            
            // Eye white
            DrawCircle(
                static_cast<int>(x + 6 * s / 3.0f),
                static_cast<int>(y - 3 * s / 3.0f),
                4.0f * s / 3.0f,
                eyeWhite
            );
            
            // Eye pupil
            DrawCircle(
                static_cast<int>(x + 7 * s / 3.0f),
                static_cast<int>(y - 3 * s / 3.0f),
                2.0f * s / 3.0f,
                eyeBlack
            );
            
            // Beak
            Vector2 beakPoints[3] = {
                {x + 10 * s / 3.0f, y},
                {x + 18 * s / 3.0f, y + 2 * s / 3.0f},
                {x + 10 * s / 3.0f, y + 4 * s / 3.0f}
            };
            DrawTriangle(beakPoints[0], beakPoints[1], beakPoints[2], beakColor);
        }
    };

} // namespace flappy
