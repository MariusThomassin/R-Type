/*
** R-Type ECS - PlayerShipComponent
** Self-rendering pixel-art player ship with engine effects
** Implements IRenderable for the self-rendering pattern
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include "engine/graphics/IRenderable.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/graphics/RenderUtils.hpp"

#include <raylib.h>
#include <cmath>

namespace rtype::ecs {

    /**
     * @brief Self-rendering player ship component
     * 
     * Renders a Galaga/R-Type style pixel-art ship with:
     * - Animated engine flames
     * - Customizable colors
     * - Shield effects (when active)
     */
    struct PlayerShipComponent : public IComponent, public IRenderable {
        enum class ShipStyle { Classic, Stealth, Heavy, Scout };
        
        unsigned char mainR = 100, mainG = 150, mainB = 255;
        unsigned char cockpitR = 200, cockpitG = 230, cockpitB = 255;
        unsigned char engineR = 255, engineG = 200, engineB = 100;

        int layer = 10;
        float pixelScale = 2.0f;
        bool isVisible = true;

        bool shieldActive = false;
        float shieldPulse = 0.0f;
        bool isInvincible = false;
        float invincibleTimer = 0.0f;

        PlayerShipComponent() = default;

        PlayerShipComponent(ShipStyle style) {
            switch (style) {
                case ShipStyle::Classic:
                    mainR = 100; mainG = 150; mainB = 255;
                    break;
                case ShipStyle::Stealth:
                    mainR = 80; mainG = 80; mainB = 100;
                    break;
                case ShipStyle::Heavy:
                    mainR = 200; mainG = 100; mainB = 50;
                    pixelScale = 2.5f;
                    break;
                case ShipStyle::Scout:
                    mainR = 100; mainG = 200; mainB = 100;
                    pixelScale = 1.5f;
                    break;
            }
        }

        PlayerShipComponent(unsigned char r, unsigned char g, unsigned char b)
            : mainR(r), mainG(g), mainB(b) {}

        std::string getTypeName() const override {
            return "PlayerShipComponent";
        }

        /**
         * @brief Update animation state (IRenderable compatibility)
         */
        void updateAnimation(float dt) {
            updateEffects(dt);
        }

        // ==================== IRenderable Implementation ====================

        bool isRenderable() const override { return isVisible; }
        int getRenderLayer() const override { return layer; }

        void render(const TransformComponent& transform, 
                   const RenderContext& ctx) const override {
            if (isInvincible) {
                int flash = static_cast<int>(ctx.animTime * 20.0f) % 2;
                if (flash == 0) return;
            }

            float x = transform.x;
            float y = transform.y;
            float s = pixelScale * transform.scaleX;

            Color mainColor = {mainR, mainG, mainB, 255};
            Color darkColor = {
                static_cast<unsigned char>(mainR * 0.5f),
                static_cast<unsigned char>(mainG * 0.5f),
                static_cast<unsigned char>(mainB * 0.5f), 255
            };
            Color cockpitColor = {cockpitR, cockpitG, cockpitB, 255};
            Color engineColor = {engineR, engineG, engineB, 255};

            DrawRectangle(int(x + 14*s), int(y - 1*s), int(4*s), int(2*s), mainColor);
            DrawRectangle(int(x + 10*s), int(y - 2*s), int(6*s), int(4*s), mainColor);
            DrawRectangle(int(x + 2*s), int(y - 4*s), int(10*s), int(8*s), mainColor);
            DrawRectangle(int(x - 4*s), int(y - 3*s), int(8*s), int(6*s), mainColor);

            DrawRectangle(int(x - 2*s), int(y - 8*s), int(6*s), int(4*s), mainColor);
            DrawRectangle(int(x - 2*s), int(y + 4*s), int(6*s), int(4*s), mainColor);

            DrawRectangle(int(x + 2*s), int(y - 10*s), int(3*s), int(2*s), darkColor);
            DrawRectangle(int(x + 2*s), int(y + 8*s), int(3*s), int(2*s), darkColor);

            DrawRectangle(int(x - 8*s), int(y - 2*s), int(4*s), int(4*s), darkColor);

            DrawRectangle(int(x + 6*s), int(y - 1*s), int(4*s), int(2*s), cockpitColor);

            int animFrame = static_cast<int>(ctx.animTime * 15.0f) % 10;
            float pulse = 0.7f + 0.3f * (animFrame < 5 ? animFrame * 0.2f : (10 - animFrame) * 0.2f);
            Color glowColor = {
                static_cast<unsigned char>(255 * pulse),
                static_cast<unsigned char>(150 * pulse),
                static_cast<unsigned char>(50 * pulse), 255
            };

            int flameLen = int(6*s + 4*s * ((animFrame % 3) * 0.5f));
            DrawRectangle(int(x - 8*s - flameLen), int(y - 1*s), flameLen, int(2*s), glowColor);
            DrawRectangle(int(x - 10*s), int(y - 1*s), int(3*s), int(2*s), engineColor);

            if (shieldActive) {
                float shieldAlpha = 0.3f + 0.2f * std::sin(ctx.animTime * 5.0f);
                Color shieldColor = {100, 200, 255, static_cast<unsigned char>(255 * shieldAlpha)};
                DrawCircleLines(int(x), int(y), 25.0f * s, shieldColor);
                DrawCircleLines(int(x), int(y), 28.0f * s, shieldColor);
            }
        }

        // ==================== State Updates ====================

        void updateEffects(float dt) {
            if (isInvincible) {
                invincibleTimer -= dt;
                if (invincibleTimer <= 0) {
                    isInvincible = false;
                }
            }
            if (shieldActive) {
                shieldPulse += dt * 5.0f;
            }
        }

        void setInvincible(float duration) {
            isInvincible = true;
            invincibleTimer = duration;
        }
    };

} // namespace rtype::ecs
