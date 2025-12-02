/*
** R-Type ECS - BackgroundComponent
** Self-rendering parallax starfield background
** Implements IRenderable for the self-rendering pattern
*/

#pragma once

#include "../IComponent.hpp"
#include "../IRenderable.hpp"
#include "TransformComponent.hpp"

#include <raylib.h>
#include <vector>
#include <cstdint>

namespace rtype::ecs {

    /**
     * @brief Self-rendering scrolling starfield background
     * 
     * Creates a parallax effect with multiple star layers:
     * - Layer 0: Far stars (slow, dim, small)
     * - Layer 1: Mid stars (medium speed/brightness)
     * - Layer 2: Near stars (fast, bright, larger)
     */
    struct BackgroundComponent : public IComponent, public IRenderable {
        struct Star {
            float x, y;
            float speed;
            float brightness;
            float size;
            int layer;  // 0=far, 1=mid, 2=near
        };

        std::vector<Star> stars;
        int screenWidth = 1280;
        int screenHeight = 720;
        int layer = -100;
        mutable uint32_t m_randomSeed = 54321;  // Persistent random state

        Color farColor = {150, 150, 180, 255};   // Bluish distant stars
        Color midColor = {200, 200, 210, 255};   // Neutral
        Color nearColor = {255, 255, 255, 255};  // Bright white

        unsigned char bgR = 8, bgG = 8, bgB = 20;

        BackgroundComponent() = default;

        BackgroundComponent(int width, int height, int starCount = 200, float /*speed*/ = 100.0f)
            : screenWidth(width), screenHeight(height) {
            initStars(starCount);
        }

        std::string getTypeName() const override {
            return "BackgroundComponent";
        }

        /**
         * @brief Update animation (for IRenderable compatibility)
         */
        void updateAnimation(float dt) {
            update(dt);
        }

        // ==================== Random Number Generation ====================

        uint32_t fastRand() const {
            m_randomSeed ^= m_randomSeed << 13;
            m_randomSeed ^= m_randomSeed >> 17;
            m_randomSeed ^= m_randomSeed << 5;
            return m_randomSeed;
        }

        // ==================== Star Generation ====================

        void initStars(int count = 200) {
            stars.clear();
            stars.reserve(count);

            for (int i = 0; i < count; ++i) {
                int starLayer = fastRand() % 3;
                
                Star star;
                star.x = static_cast<float>(fastRand() % screenWidth);
                star.y = static_cast<float>(fastRand() % screenHeight);
                star.layer = starLayer;

                switch (starLayer) {
                    case 0:
                        star.speed = 20.0f + (fastRand() % 10);
                        star.brightness = 0.3f + (fastRand() % 20) * 0.01f;
                        star.size = 1.0f;
                        break;
                    case 1:
                        star.speed = 50.0f + (fastRand() % 20);
                        star.brightness = 0.6f + (fastRand() % 20) * 0.01f;
                        star.size = 1.5f;
                        break;
                    case 2:
                        star.speed = 90.0f + (fastRand() % 30);
                        star.brightness = 0.9f + (fastRand() % 10) * 0.01f;
                        star.size = 2.0f;
                        break;
                }
                stars.push_back(star);
            }
        }

        // ==================== IRenderable Implementation ====================

        bool isRenderable() const override { return true; }
        int getRenderLayer() const override { return layer; }

        void render([[maybe_unused]] const TransformComponent& transform, 
                   [[maybe_unused]] const RenderContext& ctx) const override {
            for (const Star& s : stars) {
                Color baseColor;
                switch (s.layer) {
                    case 0: baseColor = farColor; break;
                    case 1: baseColor = midColor; break;
                    default: baseColor = nearColor; break;
                }

                unsigned char alpha = static_cast<unsigned char>(255 * s.brightness);
                Color c = {
                    static_cast<unsigned char>(baseColor.r * s.brightness),
                    static_cast<unsigned char>(baseColor.g * s.brightness),
                    static_cast<unsigned char>(baseColor.b * s.brightness),
                    alpha
                };

                int x = static_cast<int>(s.x);
                int y = static_cast<int>(s.y);

                DrawPixel(x, y, c);
                if (s.size > 1.5f) {
                    DrawPixel(x - 1, y, c);
                    DrawPixel(x + 1, y, c);
                    DrawPixel(x, y - 1, c);
                    DrawPixel(x, y + 1, c);
                }
            }
        }

        // ==================== Update (call from a system) ====================

        void update(float dt) {
            for (Star& s : stars) {
                s.x -= s.speed * dt;

                if (s.x < -5) {
                    s.x = static_cast<float>(screenWidth + (fastRand() % 20));
                    s.y = static_cast<float>(fastRand() % screenHeight);
                }
            }
        }

        // ==================== Configuration ====================

        void setScreenSize(int width, int height) {
            screenWidth = width;
            screenHeight = height;
        }

        void setBackgroundColor(unsigned char r, unsigned char g, unsigned char b) {
            bgR = r; bgG = g; bgB = b;
        }

        Color getBackgroundColor() const {
            return {bgR, bgG, bgB, 255};
        }
    };

} // namespace rtype::ecs
