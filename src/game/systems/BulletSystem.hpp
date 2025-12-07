/*
** R-Type ECS - BulletSystem
** Handles projectile spawning, movement, and lifetime
** Supports Touhou-style bullet spritesheets
** Subscribes to ShootEvent and DanmakuEvent
*/

#pragma once

#include "../../engine/ecs/core/ISystem.hpp"
#include "../../engine/ecs/core/Registry.hpp"
#include "../../engine/ecs/core/EventBus.hpp"
#include "../../engine/ecs/events/InputEvents.hpp"
#include "../../engine/ecs/components/TransformComponent.hpp"
#include "../../engine/ecs/components/VelocityComponent.hpp"
#include "../../engine/ecs/components/SpriteComponent.hpp"
#include "../../engine/ecs/components/LifetimeComponent.hpp"
#include "../components/WeaponComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/SpritesheetComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/bullets/TrajectoryComponent.hpp"
#include "../components/bullets/SpinComponent.hpp"
#include "common/ColorPalette.hpp"
#include "common/EntityLifetimeManager.hpp"

#include <vector>
#include <cstdlib>
#include <cmath>

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
            /**
             * @brief Construct a new Bullet System object
             * @param eventBus Reference to EventBus for event handling
             */
            BulletSystem(EventBus& eventBus);
            /**
             * @brief Destroy the Bullet System object
             */
            ~BulletSystem() override;

            /**
             * @brief Update all projectiles and handle spawning events
             * @param dt Delta time since last update
             */
            void update(float dt) override;
            /**
             * @brief Set the screen dimensions for spawning calculations
             * @param width Screen width
             * @param height Screen height
             */
            void setScreenSize(int width, int height);
            /**
             * @brief Get the system phase (GameLogic)
             * @return SystemPhase
             */
            SystemPhase getPhase() const override;

            /**
             * @brief Spawn a projectile from an entity with a weapon
             * @param shooterId The entity firing the weapon
             * @return EntityId of the created projectile, or NULL_ENTITY if failed
             */
            EntityId spawnProjectile(EntityId shooterId);

            /**
             * @brief Spawn a Touhou-style bullet with specific type and color
             * @param x Starting X position
             * @param y Starting Y position
             * @param velX Velocity X
             * @param velY Velocity Y
             * @param bulletType Type of bullet from spritesheet
             * @param bulletColor Color variant
             * @param isPlayerBullet Whether this is a player bullet
             * @param trajectory Optional trajectory for complex movement
             * @return EntityId of the created projectile
             */
            EntityId spawnTouhouBullet(float x, float y, float velX, float velY,
                                    BulletType bulletType, BulletColor bulletColor,
                                    bool isPlayerBullet = false,
                                    const TrajectoryComponent* trajectory = nullptr);

            /**
             * @brief Spawn a batch of bullets for pattern systems
             * @param bullets Vector of bullet definitions to spawn
             * @return Vector of created bullet entity IDs
             */
            struct BulletSpawnData {
                float x, y;
                float velX, velY;
                BulletType type;
                BulletColor color;
                bool isPlayer;
                TrajectoryType trajectoryType = TrajectoryType::Linear;
                float trajParam1 = 0.0f;
                float trajParam2 = 0.0f;
                float trajParam3 = 0.0f;
            };

            /**
             * @brief Spawn a batch of bullets for pattern systems
             * @param bullets Vector of bullet definitions to spawn
             * @return Vector of created bullet entity IDs
             */
            std::vector<EntityId> spawnBulletBatch(const std::vector<BulletSpawnData>& bullets);

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
                                BulletType bulletType, BulletColor bulletColor);

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
                                float speed, BulletType bulletType, BulletColor bulletColor);

            /**
             * @brief Get current game time
             */
            float getGameTime() const;

        private:
            /**
             * @brief Spawn a danmaku pattern (called via DanmakuEvent)
             */
            void spawnDanmakuPattern();

        private:
            /**
             * @brief Reference to EventBus for event handling
             */
            EventBus& m_eventBus;
            /**
             * @brief Subscriber IDs for events
             */
            EventBus::SubscriberId m_shootSubId;
            /**
             * @brief Subscriber ID for danmaku spawn event
             */
            EventBus::SubscriberId m_danmakuSubId;

            /**
             * @brief Game time accumulator
             */
            float m_gameTime = 0.0f;
            /**
             * @brief Expired projectiles to be destroyed
             */
            std::vector<EntityId> m_expiredProjectiles;
            /**
             * @brief Screen dimensions for spawning calculations
             */
            int m_screenWidth = 1280;
            /**
             * @brief Screen height for spawning calculations
             */
            int m_screenHeight = 720;
            /**
             * @brief Counter for enemy bullets spawned, used for layering
             */
            int m_enemyBulletCounter = 0;
        };
} // namespace rtype::ecs