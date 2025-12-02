/*
** R-Type ECS - SpriteComponent
** Visual representation data for rendering
** Now implements IRenderable for self-rendering capability
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include "engine/graphics/IRenderable.hpp"
#include "TransformComponent.hpp"
#include <string>
#include <cmath>

namespace rtype::ecs {

    /**
     * @brief Component holding sprite/texture rendering data
     *
     * Used by entities that need to be rendered visually.
     * The textureId refers to a texture managed by the asset system.
     * 
     * Implements IRenderable to handle its own rendering logic,
     * keeping rendering code close to the data it operates on.
     */
    struct SpriteComponent : public IComponent, public IRenderable {
        std::string textureId;   // Asset manager texture key
        int layer = 0;           // Render layer (higher = on top)
        bool isVisible = true;   // Visibility toggle

        float srcX = 0.0f;
        float srcY = 0.0f;
        float srcWidth = 0.0f;
        float srcHeight = 0.0f;

        unsigned char tintR = 255;
        unsigned char tintG = 255;
        unsigned char tintB = 255;
        unsigned char tintA = 255;

        enum class RenderStyle {
            Default,      // Simple rectangle/texture
            GalagaShip,   // Pixel-art ship with engine effects
            GalagaBullet  // Bullet with trail effects
        };
        RenderStyle renderStyle = RenderStyle::Default;

        SpriteComponent() = default;

        explicit SpriteComponent(const std::string& texId)
            : textureId(texId) {}

        SpriteComponent(const std::string& texId, int renderLayer)
            : textureId(texId), layer(renderLayer) {}

        std::string getTypeName() const override {
            return "SpriteComponent";
        }

        // ==================== IRenderable Implementation ====================

        bool isRenderable() const override { return isVisible; }
        int getRenderLayer() const override { return layer; }

        void render(const TransformComponent& transform, 
                   const RenderContext& ctx) const override {
            switch (renderStyle) {
                case RenderStyle::GalagaShip:
                    renderGalagaShip(transform, ctx);
                    break;
                case RenderStyle::GalagaBullet:
                    renderGalagaBullet(transform, ctx);
                    break;
                default:
                    renderDefault(transform, ctx);
                    break;
            }
        }

    private:
        // ==================== Galaga-style Ship Rendering ====================

        void renderGalagaShip(const TransformComponent& transform, 
                              const RenderContext& ctx) const {
            float x = transform.x;
            float y = transform.y;
            float s = transform.scaleX * 2.0f;  // Pixel scale

            Color mainColor = {tintR, tintG, tintB, 255};
            Color darkColor = {
                static_cast<unsigned char>(tintR * 0.5f),
                static_cast<unsigned char>(tintG * 0.5f),
                static_cast<unsigned char>(tintB * 0.5f), 255
            };
            Color cockpitColor = {200, 230, 255, 255};
            Color engineColor = {255, 200, 100, 255};

            // Main body
            DrawRectangle(static_cast<int>(x + 14*s), static_cast<int>(y - 1*s), 
                         static_cast<int>(4*s), static_cast<int>(2*s), mainColor);
            DrawRectangle(static_cast<int>(x + 10*s), static_cast<int>(y - 2*s), 
                         static_cast<int>(6*s), static_cast<int>(4*s), mainColor);
            DrawRectangle(static_cast<int>(x + 2*s), static_cast<int>(y - 4*s), 
                         static_cast<int>(10*s), static_cast<int>(8*s), mainColor);
            DrawRectangle(static_cast<int>(x - 4*s), static_cast<int>(y - 3*s), 
                         static_cast<int>(8*s), static_cast<int>(6*s), mainColor);

            // Wings
            DrawRectangle(static_cast<int>(x - 2*s), static_cast<int>(y - 8*s), 
                         static_cast<int>(6*s), static_cast<int>(4*s), mainColor);
            DrawRectangle(static_cast<int>(x - 2*s), static_cast<int>(y + 4*s), 
                         static_cast<int>(6*s), static_cast<int>(4*s), mainColor);

            // Wing tips
            DrawRectangle(static_cast<int>(x + 2*s), static_cast<int>(y - 10*s), 
                         static_cast<int>(3*s), static_cast<int>(2*s), darkColor);
            DrawRectangle(static_cast<int>(x + 2*s), static_cast<int>(y + 8*s), 
                         static_cast<int>(3*s), static_cast<int>(2*s), darkColor);

            // Engine housing
            DrawRectangle(static_cast<int>(x - 8*s), static_cast<int>(y - 2*s), 
                         static_cast<int>(4*s), static_cast<int>(4*s), darkColor);

            // Cockpit
            DrawRectangle(static_cast<int>(x + 6*s), static_cast<int>(y - 1*s), 
                         static_cast<int>(4*s), static_cast<int>(2*s), cockpitColor);

            // Animated engine flame
            int animFrame = static_cast<int>(ctx.animTime * 15.0f) % 10;
            float enginePulse = 0.7f + 0.3f * (animFrame < 5 ? animFrame * 0.2f : (10 - animFrame) * 0.2f);
            Color glowColor = {
                static_cast<unsigned char>(255 * enginePulse),
                static_cast<unsigned char>(150 * enginePulse),
                static_cast<unsigned char>(50 * enginePulse),
                255
            };

            int flameLen = static_cast<int>(6*s + 4*s * ((animFrame % 3) * 0.5f));
            DrawRectangle(static_cast<int>(x - 8*s - flameLen), static_cast<int>(y - 1*s), 
                         flameLen, static_cast<int>(2*s), glowColor);

            DrawRectangle(static_cast<int>(x - 10*s), static_cast<int>(y - 1*s), 
                         static_cast<int>(3*s), static_cast<int>(2*s), engineColor);
        }

        // ==================== Galaga-style Bullet Rendering ====================

        void renderGalagaBullet(const TransformComponent& transform, 
                                [[maybe_unused]] const RenderContext& ctx) const {
            float x = transform.x;
            float y = transform.y;
            float s = transform.scaleX * 2.0f;

            Color color = {tintR, tintG, tintB, 255};
            Color coreColor = {255, 255, 255, 255};

            DrawRectangle(static_cast<int>(x - 2*s), static_cast<int>(y - 2*s), 
                         static_cast<int>(12*s), static_cast<int>(4*s), color);

            DrawRectangle(static_cast<int>(x + 10*s), static_cast<int>(y - 1*s), 
                         static_cast<int>(4*s), static_cast<int>(2*s), color);
            DrawRectangle(static_cast<int>(x + 14*s), static_cast<int>(y), 
                         static_cast<int>(2*s), static_cast<int>(1*s), color);

            DrawRectangle(static_cast<int>(x + 4*s), static_cast<int>(y - 1*s), 
                         static_cast<int>(6*s), static_cast<int>(2*s), coreColor);

            for (int i = 1; i <= 4; ++i) {
                unsigned char alpha = static_cast<unsigned char>(200 - i * 45);
                Color trailColor = {tintR, tintG, tintB, alpha};
                float trailX = x - i * 6*s;
                int trailHeight = static_cast<int>((4 - i) * s);
                if (trailHeight > 0) {
                    DrawRectangle(
                        static_cast<int>(trailX - 2*s),
                        static_cast<int>(y - trailHeight/2),
                        static_cast<int>(4*s),
                        trailHeight,
                        trailColor
                    );
                }
            }
        }

        // ==================== Default Sprite Rendering ====================

        void renderDefault(const TransformComponent& transform, 
                          [[maybe_unused]] const RenderContext& ctx) const {
            float width = srcWidth > 0 ? srcWidth : 32.0f;
            float height = srcHeight > 0 ? srcHeight : 32.0f;

            Color color = {tintR, tintG, tintB, tintA};

            Rectangle rect = {
                transform.x - (width * transform.scaleX) / 2,
                transform.y - (height * transform.scaleY) / 2,
                width * transform.scaleX,
                height * transform.scaleY
            };

            DrawRectanglePro(rect, {0, 0}, transform.rotation, color);
        }
    };

} // namespace rtype::ecs
