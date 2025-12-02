/*
** R-Type ECS - SpritesheetComponent
** Lightweight component for spritesheet-based rendering
** Uses RenderUtils for shared rendering logic
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include "engine/graphics/IRenderable.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "BulletTypes.hpp"
#include "BulletSprites.hpp"
#include "engine/graphics/RenderUtils.hpp"
#include <string>

namespace rtype::ecs {

    /**
     * @brief Component for spritesheet-based sprites (bullets, effects, etc.)
     */
    struct SpritesheetComponent : public IComponent, public IRenderable {
        std::string textureId = "touhou_bullets";
        int frameWidth = 16, frameHeight = 16;
        int frameX = 0, frameY = 0;
        int layer = 5;
        bool isVisible = true;
        float rotation = 0.0f;

        unsigned char tintR = 255, tintG = 255, tintB = 255, tintA = 255;
        bool hasGlow = true;
        float glowIntensity = 0.3f;

        // Animation (optional)
        int animationFrames = 1, currentFrame = 0;
        float animationSpeed = 0.0f, animationTimer = 0.0f;

        SpritesheetComponent() = default;
        SpritesheetComponent(int fx, int fy, int fw = 16, int fh = 16)
            : frameWidth(fw), frameHeight(fh), frameX(fx), frameY(fy) {}
        SpritesheetComponent(BulletType type, BulletColor color) { setBullet(type, color); }

        void setBullet(BulletType type, BulletColor color) {
            m_bulletType = type;
            m_bulletColor = color;
            m_useBulletMapping = true;
            int fx, fy;
            if (getBulletFrame(type, color, fx, fy)) {
                frameX = fx; frameY = fy;
            }
        }

        bool updateAnimation(float dt) {
            if (animationSpeed <= 0.0f || animationFrames <= 1) return false;
            animationTimer += dt;
            if (animationTimer >= 1.0f / animationSpeed) {
                animationTimer -= 1.0f / animationSpeed;
                currentFrame = (currentFrame + 1) % animationFrames;
                return true;
            }
            return false;
        }

        void getSourceRect(float& x, float& y, float& w, float& h) const {
            if (m_useBulletMapping) {
                int sx, sy, sw, sh;
                if (getBulletSourceRect(m_bulletType, m_bulletColor, sx, sy, sw, sh)) {
                    x = static_cast<float>(sx); y = static_cast<float>(sy);
                    w = static_cast<float>(sw); h = static_cast<float>(sh);
                    return;
                }
            }
            x = static_cast<float>(BULLET_SHEET_OFFSET_X + (frameX + currentFrame) * frameWidth);
            y = static_cast<float>(BULLET_SHEET_OFFSET_Y + frameY * frameHeight);
            w = static_cast<float>(frameWidth);
            h = static_cast<float>(frameHeight);
        }

        std::string getTypeName() const override { return "SpritesheetComponent"; }
        bool isRenderable() const override { return isVisible; }
        int getRenderLayer() const override { return layer; }
        
        BulletType getBulletType() const { return m_bulletType; }
        BulletColor getBulletColor() const { return m_bulletColor; }
        bool usesBulletMapping() const { return m_useBulletMapping; }

        void render(const TransformComponent& transform, const RenderContext& ctx) const override {
            const Texture2D* texture = ctx.getTexture(textureId);
            float srcX, srcY, srcW, srcH;
            getSourceRect(srcX, srcY, srcW, srcH);

            float scale = transform.scaleX * 2.0f;
            float destW = srcW * scale, destH = srcH * scale;

            if (!texture || texture->id == 0) {
                RenderUtils::drawFallbackCircle(transform.x, transform.y,
                    static_cast<float>(frameWidth) * transform.scaleX, tintR, tintG, tintB, tintA);
                return;
            }

            // Glow effect using Outline texture (only for Pellet type)
            if (hasGlow && glowIntensity > 0.0f && m_useBulletMapping && m_bulletType == BulletType::Pellet) {
                int olSrcX, olSrcY, olSrcW, olSrcH;
                if (getBulletSourceRect(BulletType::Outline, m_bulletColor, olSrcX, olSrcY, olSrcW, olSrcH)) {
                    float pulse = 1.0f + 0.05f * std::sin(ctx.animTime * 5.0f);
                    float glowScale = 1.25f * pulse;
                    unsigned char glowAlpha = static_cast<unsigned char>(220 * glowIntensity);
                    
                    RenderUtils::drawSprite(*texture,
                        static_cast<float>(olSrcX), static_cast<float>(olSrcY),
                        static_cast<float>(olSrcW), static_cast<float>(olSrcH),
                        transform.x, transform.y,
                        destW * glowScale, destH * glowScale,
                        transform.rotation + rotation,
                        {tintR, tintG, tintB, glowAlpha});
                }
            }

            RenderUtils::drawSprite(*texture, srcX, srcY, srcW, srcH,
                transform.x, transform.y, destW, destH,
                transform.rotation + rotation, {tintR, tintG, tintB, tintA});
        }

    private:
        BulletType m_bulletType = BulletType::Pellet;
        BulletColor m_bulletColor = BulletColor::Red;
        bool m_useBulletMapping = false;
    };

} // namespace rtype::ecs
