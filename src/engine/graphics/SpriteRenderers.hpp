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
    class SpriteRenderers {
        public:
            /**
             * @brief Render a default rectangular sprite
             * @param renderer The rendering interface
             * @param transform The transform component data
             * @param data The sprite render data
             */
            static void renderDefault(IRenderer& renderer, const TransformComponent& transform, const SpriteRenderData& data);

            /**
             * @brief Render a Galaga-style ship with pixel art and engine effects
             * @param renderer The rendering interface
             * @param transform The transform component data
             * @param data The sprite render data
             * @param animTime Animation time for engine effects
             */
            static void renderGalagaShip(IRenderer& renderer, const TransformComponent& transform, const SpriteRenderData& data, float animTime);

            /**
             * @brief Render a Galaga-style bullet with trail effects
             * @param renderer The rendering interface
             * @param transform The transform component data
             * @param data The sprite render data
             */
            static void renderGalagaBullet(IRenderer& renderer, const TransformComponent& transform, const SpriteRenderData& data);

            /**
             * @brief Render an enemy with pulsing glow effect
             * @param renderer The rendering interface
             * @param transform The transform component data
             * @param data The sprite render data
             * @param animTime Animation time for glow pulsing
             */
            static void renderEnemy(IRenderer& renderer, const TransformComponent& transform, const SpriteRenderData& data, float animTime);

            /**
             * @brief Render a simple particle
             * @param renderer The rendering interface
             * @param transform The transform component data
             * @param color The particle color
             * @param size The particle size
             */
            static void renderParticle(IRenderer& renderer, const TransformComponent& transform, const RenderColor& color, float size);
    };
} // namespace rtype::ecs
