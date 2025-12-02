/*
** R-Type ECS - RenderSystem
** Handles rendering entities with Raylib
** Galaga-inspired pixel art style
*/

#pragma once

#include "../ISystem.hpp"
#include "../Registry.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/ProjectileComponent.hpp"

#include <raylib.h>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <cmath>

namespace rtype::ecs {

    /**
     * @brief System that renders entities using Raylib
     *
     * Galaga-inspired pixel art style with animated starfield.
     */
    class RenderSystem : public ISystem {
    public:
        RenderSystem(int screenWidth = 1280, int screenHeight = 720)
            : m_screenWidth(screenWidth)
            , m_screenHeight(screenHeight)
            , m_randomState(12345)
            , m_animTime(0.0f) {
            initStarfield();
            initNebula();
        }

        ~RenderSystem() override = default;

        void update(float dt) override {
            if (!m_registry) return;

            m_animTime += dt;

            updateStarfield(dt);

            BeginDrawing();

            drawSpaceBackground();

            drawStarfield();

            auto entities = m_registry->getEntitiesWith<TransformComponent, SpriteComponent>();

            std::vector<EntityId> sortedEntities(entities.begin(), entities.end());
            std::sort(sortedEntities.begin(), sortedEntities.end(),
                [this](EntityId a, EntityId b) {
                    return m_registry->getComponent<SpriteComponent>(a).layer <
                           m_registry->getComponent<SpriteComponent>(b).layer;
                });

            for (EntityId entity : sortedEntities) {
                const auto& transform = m_registry->getComponent<TransformComponent>(entity);
                const auto& sprite = m_registry->getComponent<SpriteComponent>(entity);

                if (!sprite.isVisible) continue;

                if (m_registry->hasComponent<PlayerComponent>(entity)) {
                    drawGalagaShip(transform, sprite);
                } else if (m_registry->hasComponent<ProjectileComponent>(entity)) {
                    drawGalagaBullet(transform, sprite);
                } else {
                    drawDefaultSprite(transform, sprite);
                }
            }

            drawUI();

            EndDrawing();
        }

        SystemPhase getPhase() const override {
            return SystemPhase::Render;
        }

        int getScreenWidth() const { return m_screenWidth; }
        int getScreenHeight() const { return m_screenHeight; }

    private:
        // ==================== Fast Random (xorshift) ====================

        uint32_t fastRandom() {
            m_randomState ^= m_randomState << 13;
            m_randomState ^= m_randomState >> 17;
            m_randomState ^= m_randomState << 5;
            return m_randomState;
        }

        int fastRandomRange(int min, int max) {
            return min + static_cast<int>(fastRandom() % static_cast<uint32_t>(max - min + 1));
        }

        // ==================== Galaga-style Space Background ====================

        struct Star {
            float x, y;
            float speed;
            float brightness;
            float size;
            int colorType;
        };

        struct NebulaCloud {
            float x, y;
            float radius;
            float speed;
            Color color;
        };

        void initNebula() {
            m_nebulae.clear();
            m_nebulae.reserve(8);

            Color nebulaColors[] = {
                {40, 20, 80, 30},   // Purple
                {20, 40, 80, 25},   // Blue
                {80, 20, 40, 20},   // Magenta
                {20, 60, 60, 25},   // Teal
            };

            for (int i = 0; i < 8; ++i) {
                NebulaCloud cloud;
                cloud.x = static_cast<float>(fastRandomRange(0, m_screenWidth + 200));
                cloud.y = static_cast<float>(fastRandomRange(50, m_screenHeight - 50));
                cloud.radius = static_cast<float>(fastRandomRange(100, 300));
                cloud.speed = static_cast<float>(fastRandomRange(5, 15));
                cloud.color = nebulaColors[i % 4];
                m_nebulae.push_back(cloud);
            }
        }

        void initStarfield() {
            m_stars.clear();
            m_stars.reserve(STAR_COUNT);

            for (int i = 0; i < STAR_COUNT; ++i) {
                Star star;
                star.x = static_cast<float>(fastRandomRange(0, m_screenWidth));
                star.y = static_cast<float>(fastRandomRange(0, m_screenHeight));
                star.colorType = fastRandomRange(0, 10) < 7 ? 0 : fastRandomRange(1, 3);

                int layer = fastRandomRange(0, 2);
                switch (layer) {
                    case 0:
                        star.speed = 20.0f;
                        star.brightness = 0.4f;
                        star.size = 1.0f;
                        break;
                    case 1:
                        star.speed = 50.0f;
                        star.brightness = 0.7f;
                        star.size = 1.5f;
                        break;
                    case 2:
                        star.speed = 90.0f;
                        star.brightness = 1.0f;
                        star.size = 2.0f;
                        break;
                }
                m_stars.push_back(star);
            }
        }

        void updateStarfield(float dt) {
            for (Star& star : m_stars) {
                star.x -= star.speed * dt;
                if (star.x < 0) {
                    star.x = static_cast<float>(m_screenWidth);
                    star.y = static_cast<float>(fastRandomRange(0, m_screenHeight));
                }
            }
        }

        void drawSpaceBackground() {
            ClearBackground({8, 8, 20, 255});
        }

        void drawStarfield() {
            for (const Star& star : m_stars) {
                unsigned char brightness = static_cast<unsigned char>(255 * star.brightness);

                Color starColor;
                switch (star.colorType) {
                    case 1:
                        starColor = {static_cast<unsigned char>(brightness * 0.7f),
                                    static_cast<unsigned char>(brightness * 0.8f),
                                    brightness, 255};
                        break;
                    case 2:
                        starColor = {brightness,
                                    static_cast<unsigned char>(brightness * 0.9f),
                                    static_cast<unsigned char>(brightness * 0.5f), 255};
                        break;
                    case 3:
                        starColor = {brightness,
                                    static_cast<unsigned char>(brightness * 0.5f),
                                    static_cast<unsigned char>(brightness * 0.5f), 255};
                        break;
                    default:
                        starColor = {brightness, brightness, brightness, 255};
                        break;
                }

                int sx = static_cast<int>(star.x);
                int sy = static_cast<int>(star.y);

                if (star.size <= 1.0f) {
                    DrawPixel(sx, sy, starColor);
                } else if (star.size <= 1.5f) {
                    DrawPixel(sx, sy, starColor);
                    DrawPixel(sx + 1, sy, starColor);
                } else {
                    DrawPixel(sx, sy, starColor);
                    DrawPixel(sx - 1, sy, starColor);
                    DrawPixel(sx + 1, sy, starColor);
                    DrawPixel(sx, sy - 1, starColor);
                    DrawPixel(sx, sy + 1, starColor);
                }
            }
        }

        // ==================== Galaga-style Ship Rendering ====================

        void drawGalagaShip(const TransformComponent& transform, const SpriteComponent& sprite) {
            float x = transform.x;
            float y = transform.y;
            float s = transform.scaleX * 2.0f;  // Pixel scale

            Color mainColor = {sprite.tintR, sprite.tintG, sprite.tintB, 255};
            Color darkColor = {
                static_cast<unsigned char>(sprite.tintR * 0.5f),
                static_cast<unsigned char>(sprite.tintG * 0.5f),
                static_cast<unsigned char>(sprite.tintB * 0.5f), 255
            };
            Color cockpitColor = {200, 230, 255, 255};
            Color engineColor = {255, 200, 100, 255};

            DrawRectangle(static_cast<int>(x + 14*s), static_cast<int>(y - 1*s), static_cast<int>(4*s), static_cast<int>(2*s), mainColor);
            DrawRectangle(static_cast<int>(x + 10*s), static_cast<int>(y - 2*s), static_cast<int>(6*s), static_cast<int>(4*s), mainColor);

            DrawRectangle(static_cast<int>(x + 2*s), static_cast<int>(y - 4*s), static_cast<int>(10*s), static_cast<int>(8*s), mainColor);
            DrawRectangle(static_cast<int>(x - 4*s), static_cast<int>(y - 3*s), static_cast<int>(8*s), static_cast<int>(6*s), mainColor);

            DrawRectangle(static_cast<int>(x - 2*s), static_cast<int>(y - 8*s), static_cast<int>(6*s), static_cast<int>(4*s), mainColor);
            DrawRectangle(static_cast<int>(x - 2*s), static_cast<int>(y + 4*s), static_cast<int>(6*s), static_cast<int>(4*s), mainColor);

            DrawRectangle(static_cast<int>(x + 2*s), static_cast<int>(y - 10*s), static_cast<int>(3*s), static_cast<int>(2*s), darkColor);
            DrawRectangle(static_cast<int>(x + 2*s), static_cast<int>(y + 8*s), static_cast<int>(3*s), static_cast<int>(2*s), darkColor);

            DrawRectangle(static_cast<int>(x - 8*s), static_cast<int>(y - 2*s), static_cast<int>(4*s), static_cast<int>(4*s), darkColor);

            DrawRectangle(static_cast<int>(x + 6*s), static_cast<int>(y - 1*s), static_cast<int>(4*s), static_cast<int>(2*s), cockpitColor);

            int animFrame = static_cast<int>(m_animTime * 15.0f) % 10;
            float enginePulse = 0.7f + 0.3f * (animFrame < 5 ? animFrame * 0.2f : (10 - animFrame) * 0.2f);
            Color glowColor = {
                static_cast<unsigned char>(255 * enginePulse),
                static_cast<unsigned char>(150 * enginePulse),
                static_cast<unsigned char>(50 * enginePulse),
                255
            };

            // Engine flames (simple flicker)
            int flameLen = static_cast<int>(6*s + 4*s * ((animFrame % 3) * 0.5f));
            DrawRectangle(static_cast<int>(x - 8*s - flameLen), static_cast<int>(y - 1*s), flameLen, static_cast<int>(2*s), glowColor);

            // Engine core
            DrawRectangle(static_cast<int>(x - 10*s), static_cast<int>(y - 1*s), static_cast<int>(3*s), static_cast<int>(2*s), engineColor);
        }

        // ==================== Galaga-style Bullet Rendering ====================

        void drawGalagaBullet(const TransformComponent& transform, const SpriteComponent& sprite) {
            float x = transform.x;
            float y = transform.y;
            float s = transform.scaleX * 2.0f;

            Color color = {sprite.tintR, sprite.tintG, sprite.tintB, 255};
            Color coreColor = {255, 255, 255, 255};

            DrawRectangle(static_cast<int>(x - 2*s), static_cast<int>(y - 2*s), static_cast<int>(12*s), static_cast<int>(4*s), color);

            DrawRectangle(static_cast<int>(x + 10*s), static_cast<int>(y - 1*s), static_cast<int>(4*s), static_cast<int>(2*s), color);
            DrawRectangle(static_cast<int>(x + 14*s), static_cast<int>(y), static_cast<int>(2*s), static_cast<int>(1*s), color);

            DrawRectangle(static_cast<int>(x + 4*s), static_cast<int>(y - 1*s), static_cast<int>(6*s), static_cast<int>(2*s), coreColor);

            for (int i = 1; i <= 4; ++i) {
                unsigned char alpha = static_cast<unsigned char>(200 - i * 45);
                Color trailColor = {sprite.tintR, sprite.tintG, sprite.tintB, alpha};
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

        void drawDefaultSprite(const TransformComponent& transform, const SpriteComponent& sprite) {
            float width = sprite.srcWidth > 0 ? sprite.srcWidth : 32.0f;
            float height = sprite.srcHeight > 0 ? sprite.srcHeight : 32.0f;

            Color color = {
                sprite.tintR,
                sprite.tintG,
                sprite.tintB,
                sprite.tintA
            };

            Rectangle rect = {
                transform.x - (width * transform.scaleX) / 2,
                transform.y - (height * transform.scaleY) / 2,
                width * transform.scaleX,
                height * transform.scaleY
            };

            DrawRectanglePro(rect, {0, 0}, transform.rotation, color);
        }

        // ==================== UI ====================

        void drawUI() {
            DrawText("SCORE", 20, 15, 20, {150, 150, 150, 255});
            DrawText("00000", 20, 40, 28, WHITE);

            DrawText("HI-SCORE", m_screenWidth/2 - 60, 15, 20, {150, 150, 150, 255});
            DrawText("50000", m_screenWidth/2 - 40, 40, 28, {255, 255, 100, 255});

            DrawFPS(m_screenWidth - 80, m_screenHeight - 25);

            DrawText("R-TYPE", m_screenWidth - 90, 15, 20, {100, 100, 255, 255});
        }

    private:
        int m_screenWidth;
        int m_screenHeight;
        uint32_t m_randomState;
        float m_animTime;

        static constexpr int STAR_COUNT = 250;
        std::vector<Star> m_stars;
        std::vector<NebulaCloud> m_nebulae;
    };

} // namespace rtype::ecs
