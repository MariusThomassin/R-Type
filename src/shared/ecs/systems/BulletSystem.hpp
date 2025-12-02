/*
** R-Type ECS - BulletSystem
** Handles projectile spawning, movement, and lifetime
** Supports Touhou-style bullet spritesheets
** Subscribes to ShootEvent and DanmakuEvent
*/

#pragma once

#include "../ISystem.hpp"
#include "../Registry.hpp"
#include "../EventBus.hpp"
#include "../events/InputEvents.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/SpritesheetComponent.hpp"
#include "../components/WeaponComponent.hpp"
#include "../components/LifetimeComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/PlayerComponent.hpp"

#include <vector>
#include <cstdlib>

namespace rtype::ecs {

    /**
     * @brief System that manages projectiles
     *
     * Handles:
     * - Spawning projectiles from weapons (via ShootEvent)
     * - Spawning danmaku patterns (via DanmakuEvent)
     * - Updating projectile lifetimes
     * - Destroying expired projectiles
     */
    class BulletSystem : public ISystem {
    public:
        BulletSystem(EventBus& eventBus) : m_eventBus(eventBus) {
            // Subscribe to shoot events
            m_shootSubId = m_eventBus.subscribe<events::ShootEvent>(
                [this](const events::ShootEvent& e) {
                    spawnProjectile(e.shooterId);
                }
            );

            // Subscribe to danmaku events  
            m_danmakuSubId = m_eventBus.subscribe<events::DanmakuEvent>(
                [this](const events::DanmakuEvent& e) {
                    (void)e;  // e.x/e.y not used, we spawn at fixed location
                    spawnDanmakuPattern();
                }
            );
        }

        ~BulletSystem() override {
            m_eventBus.unsubscribe<events::ShootEvent>(m_shootSubId);
            m_eventBus.unsubscribe<events::DanmakuEvent>(m_danmakuSubId);
        }

        void update(float dt) override {
            if (!m_registry) return;

            m_gameTime += dt;

            updateLifetimes(dt);
            destroyExpiredProjectiles();
            destroyOffscreenProjectiles(m_screenWidth, m_screenHeight);
        }

        /**
         * @brief Set screen dimensions for off-screen detection
         */
        void setScreenSize(int width, int height) {
            m_screenWidth = width;
            m_screenHeight = height;
        }

        SystemPhase getPhase() const override {
            return SystemPhase::GameLogic;
        }

        /**
         * @brief Spawn a projectile from an entity with a weapon
         * @param shooterId The entity firing the weapon
         * @return EntityId of the created projectile, or NULL_ENTITY if failed
         */
        EntityId spawnProjectile(EntityId shooterId) {
            if (!m_registry) return NULL_ENTITY;
            if (!m_registry->hasComponent<WeaponComponent>(shooterId)) return NULL_ENTITY;
            if (!m_registry->hasComponent<TransformComponent>(shooterId)) return NULL_ENTITY;

            auto& weapon = m_registry->getComponent<WeaponComponent>(shooterId);
            const auto& transform = m_registry->getComponent<TransformComponent>(shooterId);

            if (!weapon.isReady(m_gameTime)) {
                return NULL_ENTITY;
            }

            weapon.lastFiredTime = m_gameTime;

            bool isPlayer = m_registry->hasComponent<PlayerComponent>(shooterId);

            Entity projectile = m_registry->createEntity();

            float offsetX = 40.0f * transform.scaleX;  // Spawn in front of ship
            m_registry->addComponent(projectile, TransformComponent(
                transform.x + offsetX,
                transform.y,
                0.0f,
                1.0f, 1.0f  // Scale controlled by SpritesheetComponent
            ));

            float direction = isPlayer ? 1.0f : -1.0f;
            m_registry->addComponent(projectile, VelocityComponent(
                weapon.projectileSpeed * direction,
                0.0f,
                weapon.projectileSpeed * 1.5f  // Max speed
            ));

            SpritesheetComponent bulletSprite;
            bulletSprite.textureId = "touhou_bullets";
            bulletSprite.frameWidth = 16;
            bulletSprite.frameHeight = 16;
            bulletSprite.hasGlow = true;
            bulletSprite.glowIntensity = 0.4f;

            if (isPlayer) {
                bulletSprite.layer = 100;
                bulletSprite.setBullet(BulletType::Rice, BulletColor::Cyan);
                bulletSprite.rotation = -90.0f;
                bulletSprite.tintR = 80;
                bulletSprite.tintG = 240;
                bulletSprite.tintB = 255;
            } else {
                bulletSprite.layer = 99 - (m_enemyBulletCounter % 100);
                m_enemyBulletCounter++;
                BulletType type = (std::rand() % 2 == 0) ? BulletType::Ball : BulletType::Outline;
                BulletColor color = static_cast<BulletColor>(std::rand() % static_cast<int>(BulletColor::COUNT));
                bulletSprite.setBullet(type, color);
                applyBulletColorTint(bulletSprite, color);
            }

            m_registry->addComponent(projectile, bulletSprite);

            m_registry->addComponent(projectile, ProjectileComponent(
                shooterId,
                weapon.damage,
                isPlayer
            ));

            if (isPlayer) {
                m_registry->addComponent(projectile, LifetimeComponent(3.0f));
            }

            return projectile.id;
        }

        /**
         * @brief Spawn a Touhou-style bullet with specific type and color
         * @param x Starting X position
         * @param y Starting Y position
         * @param velX Velocity X
         * @param velY Velocity Y
         * @param bulletType Type of bullet from spritesheet
         * @param bulletColor Color variant
         * @param isPlayerBullet Whether this is a player bullet
         * @return EntityId of the created projectile
         */
        EntityId spawnTouhouBullet(float x, float y, float velX, float velY,
                                   BulletType bulletType, BulletColor bulletColor,
                                   bool isPlayerBullet = false) {
            if (!m_registry) return NULL_ENTITY;

            Entity projectile = m_registry->createEntity();

            m_registry->addComponent(projectile, TransformComponent(x, y, 0.0f, 1.5f, 1.5f));
            m_registry->addComponent(projectile, VelocityComponent(velX, velY, 1000.0f));

            SpritesheetComponent bulletSprite(bulletType, bulletColor);
            bulletSprite.hasGlow = true;
            bulletSprite.glowIntensity = 0.5f;
            
            if (isPlayerBullet) {
                bulletSprite.layer = 100;
            } else {
                bulletSprite.layer = 99 - (m_enemyBulletCounter % 100);
                m_enemyBulletCounter++;
            }

            applyBulletColorTint(bulletSprite, bulletColor);

            m_registry->addComponent(projectile, bulletSprite);
            m_registry->addComponent(projectile, ProjectileComponent(NULL_ENTITY, 10, isPlayerBullet));

            return projectile.id;
        }

        /**
         * @brief Spawn a circular pattern of bullets (Touhou danmaku style)
         * @param x Center X position
         * @param y Center Y position
         * @param bulletCount Number of bullets in the circle
         * @param speed Speed of bullets
         * @param bulletType Type of bullet
         * @param bulletColor Color of bullets
         */
        void spawnCirclePattern(float x, float y, int bulletCount, float speed,
                               BulletType bulletType, BulletColor bulletColor) {
            float angleStep = 360.0f / static_cast<float>(bulletCount);

            for (int i = 0; i < bulletCount; ++i) {
                float angle = i * angleStep * (3.14159f / 180.0f);
                float velX = std::cos(angle) * speed;
                float velY = std::sin(angle) * speed;

                spawnTouhouBullet(x, y, velX, velY, bulletType, bulletColor, false);
            }
        }

        /**
         * @brief Spawn a spiral pattern of bullets
         * @param x Center X position
         * @param y Center Y position
         * @param arms Number of spiral arms
         * @param bulletsPerArm Bullets per arm
         * @param speed Speed of bullets
         * @param bulletType Type of bullet
         * @param bulletColor Color of bullets
         */
        void spawnSpiralPattern(float x, float y, int arms, int bulletsPerArm,
                               float speed, BulletType bulletType, BulletColor bulletColor) {
            float armAngleStep = 360.0f / static_cast<float>(arms);

            for (int arm = 0; arm < arms; ++arm) {
                float baseAngle = arm * armAngleStep;

                for (int i = 0; i < bulletsPerArm; ++i) {
                    float spiralOffset = i * 15.0f;  // Spiral twist
                    float angle = (baseAngle + spiralOffset) * (3.14159f / 180.0f);
                    float bulletSpeed = speed + i * 20.0f;  // Accelerating spiral

                    float velX = std::cos(angle) * bulletSpeed;
                    float velY = std::sin(angle) * bulletSpeed;

                    spawnTouhouBullet(x, y, velX, velY, bulletType, bulletColor, false);
                }
            }
        }

        /**
         * @brief Get current game time
         */
        float getGameTime() const { return m_gameTime; }

    private:
        void updateLifetimes(float dt) {
            auto entities = m_registry->getEntitiesWith<ProjectileComponent, LifetimeComponent>();

            m_expiredProjectiles.clear();

            for (EntityId entity : entities) {
                auto& lifetime = m_registry->getComponent<LifetimeComponent>(entity);

                if (lifetime.update(dt)) {
                    m_expiredProjectiles.push_back(entity);
                }
            }
        }

        void destroyExpiredProjectiles() {
            for (EntityId entity : m_expiredProjectiles) {
                if (m_registry->entityExists(entity)) {
                    m_registry->destroyEntity(entity);
                }
            }
        }

        /**
         * @brief Destroy projectiles that go off-screen
         * @param screenWidth Width of the screen
         * @param screenHeight Height of the screen
         */
        void destroyOffscreenProjectiles(int screenWidth, int screenHeight) {
            auto entities = m_registry->getEntitiesWith<ProjectileComponent, TransformComponent>();

            std::vector<EntityId> toDestroy;

            for (EntityId entity : entities) {
                const auto& transform = m_registry->getComponent<TransformComponent>(entity);

                constexpr float MARGIN = 50.0f;
                if (transform.x < -MARGIN || transform.x > screenWidth + MARGIN ||
                    transform.y < -MARGIN || transform.y > screenHeight + MARGIN) {
                    toDestroy.push_back(entity);
                }
            }

            for (EntityId entity : toDestroy) {
                if (m_registry->entityExists(entity)) {
                    m_registry->destroyEntity(entity);
                }
            }
        }

        /**
         * @brief Apply color tint based on bullet color enum
         */
        void applyBulletColorTint(SpritesheetComponent& sprite, BulletColor color) {
            switch (color) {
                case BulletColor::Red:
                    sprite.tintR = 255; sprite.tintG = 80; sprite.tintB = 80;
                    break;
                case BulletColor::Orange:
                    sprite.tintR = 255; sprite.tintG = 160; sprite.tintB = 50;
                    break;
                case BulletColor::Yellow:
                    sprite.tintR = 255; sprite.tintG = 240; sprite.tintB = 80;
                    break;
                case BulletColor::Green:
                    sprite.tintR = 80; sprite.tintG = 255; sprite.tintB = 80;
                    break;
                case BulletColor::Cyan:
                    sprite.tintR = 80; sprite.tintG = 240; sprite.tintB = 255;
                    break;
                case BulletColor::Blue:
                    sprite.tintR = 80; sprite.tintG = 120; sprite.tintB = 255;
                    break;
                case BulletColor::Purple:
                    sprite.tintR = 180; sprite.tintG = 80; sprite.tintB = 255;
                    break;
                case BulletColor::Magenta:
                    sprite.tintR = 255; sprite.tintG = 80; sprite.tintB = 200;
                    break;
                case BulletColor::White:
                    sprite.tintR = 255; sprite.tintG = 255; sprite.tintB = 255;
                    break;
                case BulletColor::Black:
                    sprite.tintR = 60; sprite.tintG = 60; sprite.tintB = 80;
                    break;
                default:
                    sprite.tintR = 255; sprite.tintG = 255; sprite.tintB = 255;
                    break;
            }
        }

        /**
         * @brief Spawn a danmaku pattern (called via DanmakuEvent)
         */
        void spawnDanmakuPattern() {
            float spawnX = m_screenWidth * 0.75f;
            float spawnY = m_screenHeight / 2.0f;

            BulletColor color1 = static_cast<BulletColor>(std::rand() % static_cast<int>(BulletColor::COUNT));
            BulletColor color2 = static_cast<BulletColor>(std::rand() % static_cast<int>(BulletColor::COUNT));

            spawnCirclePattern(spawnX, spawnY, 24, 200.0f, BulletType::Ball, color1);
            spawnCirclePattern(spawnX, spawnY, 16, 150.0f, BulletType::Outline, color2);
        }

    private:
        EventBus& m_eventBus;
        EventBus::SubscriberId m_shootSubId;
        EventBus::SubscriberId m_danmakuSubId;

        float m_gameTime = 0.0f;
        std::vector<EntityId> m_expiredProjectiles;
        int m_screenWidth = 1280;
        int m_screenHeight = 720;
        int m_enemyBulletCounter = 0;  // For stacking enemy bullets (later = lower layer)
    };

} // namespace rtype::ecs
