/*
** R-Type ECS - BulletSystem
** Handles projectile spawning, movement, and lifetime
*/

#pragma once

#include "../ISystem.hpp"
#include "../Registry.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"
#include "../components/SpriteComponent.hpp"
#include "../components/WeaponComponent.hpp"
#include "../components/LifetimeComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../components/PlayerComponent.hpp"

#include <vector>

namespace rtype::ecs {

    /**
     * @brief System that manages projectiles
     *
     * Handles:
     * - Spawning projectiles from weapons
     * - Updating projectile lifetimes
     * - Destroying expired projectiles
     */
    class BulletSystem : public ISystem {
    public:
        BulletSystem() = default;
        ~BulletSystem() override = default;

        void update(float dt) override {
            if (!m_registry) return;

            m_gameTime += dt;

            updateLifetimes(dt);

            destroyExpiredProjectiles();
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
                0.8f, 0.8f  // Slightly smaller scale
            ));

            float direction = isPlayer ? 1.0f : -1.0f;
            m_registry->addComponent(projectile, VelocityComponent(
                weapon.projectileSpeed * direction,
                0.0f,
                weapon.projectileSpeed * 1.5f  // Max speed
            ));

            SpriteComponent sprite("projectile");
            sprite.layer = 5;
            if (isPlayer) {
                sprite.tintR = 100;
                sprite.tintG = 200;
                sprite.tintB = 255;
            } else {
                sprite.tintR = 255;
                sprite.tintG = 100;
                sprite.tintB = 50;
            }
            sprite.srcWidth = 20.0f;
            sprite.srcHeight = 8.0f;
            m_registry->addComponent(projectile, sprite);

            m_registry->addComponent(projectile, ProjectileComponent(
                shooterId,
                weapon.damage,
                isPlayer
            ));

            m_registry->addComponent(projectile, LifetimeComponent(3.0f));

            return projectile.id;
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

    private:
        float m_gameTime = 0.0f;
        std::vector<EntityId> m_expiredProjectiles;
    };

} // namespace rtype::ecs
