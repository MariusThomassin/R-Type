/*
** R-Type ECS - Sprite Renderers
** Modular rendering functions for different sprite styles
** Decoupled from components - pure functions operating on data
*/

#pragma once

#include "IRenderer.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include <cmath>

namespace rtype::ecs {

    /**
     * @brief Render parameters for sprites
     * Pure data - no behavior
     */
    struct SpriteRenderData {
        RenderColor tint = RenderColor::White();
        float srcX = 0.0f;
        float srcY = 0.0f;
        float srcWidth = 32.0f;
        float srcHeight = 32.0f;
    };

    /**
     * @brief Collection of pure rendering functions
     * 
     * These are stateless functions that take render data and produce
     * draw calls. They don't know about ECS, components, or entities.
     */
    namespace SpriteRenderers {

        /**
         * @brief Render a default rectangular sprite
         */
        inline void renderDefault(IRenderer& renderer,
                                   const TransformComponent& transform,
                                   const SpriteRenderData& data) {
            float width = data.srcWidth > 0 ? data.srcWidth : 32.0f;
            float height = data.srcHeight > 0 ? data.srcHeight : 32.0f;

            RenderRect rect = {
                transform.x - (width * transform.scaleX) / 2,
                transform.y - (height * transform.scaleY) / 2,
                width * transform.scaleX,
                height * transform.scaleY
            };

            renderer.drawRectRotated(rect, 0, 0, transform.rotation, data.tint);
        }

        /**
         * @brief Render a Galaga-style ship with pixel art and engine effects
         */
        inline void renderGalagaShip(IRenderer& renderer,
                                      const TransformComponent& transform,
                                      const SpriteRenderData& data,
                                      float animTime) {
            float x = transform.x;
            float y = transform.y;
            float s = transform.scaleX * 2.0f;  // Pixel scale

            RenderColor mainColor = data.tint;
            RenderColor darkColor = data.tint.scaled(0.5f);
            RenderColor cockpitColor = {200, 230, 255, 255};
            RenderColor engineColor = {255, 200, 100, 255};

            // Main body
            renderer.drawRect(x + 14*s, y - 1*s, 4*s, 2*s, mainColor);
            renderer.drawRect(x + 10*s, y - 2*s, 6*s, 4*s, mainColor);
            renderer.drawRect(x + 2*s, y - 4*s, 10*s, 8*s, mainColor);
            renderer.drawRect(x - 4*s, y - 3*s, 8*s, 6*s, mainColor);

            // Wings
            renderer.drawRect(x - 2*s, y - 8*s, 6*s, 4*s, mainColor);
            renderer.drawRect(x - 2*s, y + 4*s, 6*s, 4*s, mainColor);

            // Wing tips
            renderer.drawRect(x + 2*s, y - 10*s, 3*s, 2*s, darkColor);
            renderer.drawRect(x + 2*s, y + 8*s, 3*s, 2*s, darkColor);

            // Engine housing
            renderer.drawRect(x - 8*s, y - 2*s, 4*s, 4*s, darkColor);

            // Cockpit
            renderer.drawRect(x + 6*s, y - 1*s, 4*s, 2*s, cockpitColor);

            // Animated engine flame
            int animFrame = static_cast<int>(animTime * 15.0f) % 10;
            float enginePulse = 0.7f + 0.3f * (animFrame < 5 ? animFrame * 0.2f : (10 - animFrame) * 0.2f);
            RenderColor glowColor = {
                static_cast<uint8_t>(255 * enginePulse),
                static_cast<uint8_t>(150 * enginePulse),
                static_cast<uint8_t>(50 * enginePulse),
                255
            };

            float flameLen = 6*s + 4*s * ((animFrame % 3) * 0.5f);
            renderer.drawRect(x - 8*s - flameLen, y - 1*s, flameLen, 2*s, glowColor);
            renderer.drawRect(x - 10*s, y - 1*s, 3*s, 2*s, engineColor);
        }

        /**
         * @brief Render a Galaga-style bullet with trail effects
         */
        inline void renderGalagaBullet(IRenderer& renderer,
                                        const TransformComponent& transform,
                                        const SpriteRenderData& data) {
            float x = transform.x;
            float y = transform.y;
            float s = transform.scaleX * 2.0f;

            RenderColor color = data.tint;
            RenderColor coreColor = RenderColor::White();

            // Main bullet body
            renderer.drawRect(x - 2*s, y - 2*s, 12*s, 4*s, color);

            // Bullet tip
            renderer.drawRect(x + 10*s, y - 1*s, 4*s, 2*s, color);
            renderer.drawRect(x + 14*s, y, 2*s, 1*s, color);

            // Bright core
            renderer.drawRect(x + 4*s, y - 1*s, 6*s, 2*s, coreColor);

            // Trail particles
            for (int i = 1; i <= 4; ++i) {
                uint8_t alpha = static_cast<uint8_t>(200 - i * 45);
                RenderColor trailColor = color.withAlpha(alpha);
                float trailX = x - i * 6*s;
                float trailHeight = (4.0f - i) * s;
                if (trailHeight > 0) {
                    renderer.drawRect(trailX - 2*s, y - trailHeight/2, 4*s, trailHeight, trailColor);
                }
            }
        }

        /**
         * @brief Render an enemy with pulsing glow effect
         */
        inline void renderEnemy(IRenderer& renderer,
                                 const TransformComponent& transform,
                                 const SpriteRenderData& data,
                                 float animTime) {
            float x = transform.x;
            float y = transform.y;
            float s = transform.scaleX;

            // Pulsing glow
            float pulse = 0.5f + 0.5f * std::sin(animTime * 4.0f);
            RenderColor glowColor = data.tint.withAlpha(static_cast<uint8_t>(100 * pulse));

            // Glow circle (larger)
            renderer.drawCircle(x, y, 24.0f * s, glowColor);

            // Main body
            renderer.drawCircle(x, y, 16.0f * s, data.tint);

            // Inner highlight
            RenderColor highlight = RenderColor::White().withAlpha(150);
            renderer.drawCircle(x - 4*s, y - 4*s, 6.0f * s, highlight);
        }

        /**
         * @brief Render a simple particle
         */
        inline void renderParticle(IRenderer& renderer,
                                    const TransformComponent& transform,
                                    const RenderColor& color,
                                    float size) {
            renderer.drawCircle(transform.x, transform.y, size * transform.scaleX, color);
        }

    } // namespace SpriteRenderers

} // namespace rtype::ecs
