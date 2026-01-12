/*
** R-Type ECS - PowerupSystem
** Handles powerup spawning, animation, and pickup logic
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "game/components/PowerupComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "engine/ecs/events/definitions/GameEvents.hpp"

#include <random>
#include <cmath>

namespace rtype::ecs {

    /**
     * @brief System that manages powerup behavior
     * 
     * Handles:
     * - Powerup spawning (from enemy deaths or random)
     * - Floating/bobbing animation
     * - Collision with players and effect application
     * - Cleanup of collected powerups
     */
    class PowerupSystem : public ISystem {
    public:
        /**
         * @brief Construct PowerupSystem
         * @param eventBus Reference to EventBus
         * @param spawnChance Chance to spawn powerup on enemy death (0.0-1.0)
         */
        PowerupSystem(EventBus& eventBus, float spawnChance = 0.15f)
            : m_eventBus(eventBus)
            , m_spawnChance(spawnChance)
            , m_rng(std::random_device{}()) {
            
            // Subscribe to enemy death events to spawn powerups
            m_enemyDeathSubId = m_eventBus.subscribe<events::EnemyDestroyed>(
                [this](const events::EnemyDestroyed& e) {
                    trySpawnPowerup(e.x, e.y);
                }
            );
        }

        ~PowerupSystem() override {
            m_eventBus.unsubscribe<events::EnemyDestroyed>(m_enemyDeathSubId);
        }

        /**
         * @brief Update powerup animations and check for pickups
         */
        void update(float dt) override {
            if (!m_registry) return;

            // Update powerup animations (bobbing/glowing)
            m_registry->forEach<PowerupComponent, TransformComponent>(
                [this, dt](EntityId e) {
                    auto& powerup = m_registry->getComponent<PowerupComponent>(e);
                    auto& transform = m_registry->getComponent<TransformComponent>(e);

                    // Skip if already collected
                    if (powerup.isCollected) return;

                    // Bobbing animation
                    powerup.bobOffset += dt * 3.0f;
                    float bobAmount = std::sin(powerup.bobOffset) * 5.0f;
                    
                    // Glowing animation
                    powerup.glowIntensity = 0.7f + 0.3f * std::sin(powerup.bobOffset * 2.0f);

                    // Apply slight downward drift
                    if (m_registry->hasComponent<VelocityComponent>(e)) {
                        auto& vel = m_registry->getComponent<VelocityComponent>(e);
                        vel.vy = 30.0f + bobAmount;  // Slow drift down with bob
                    }
                }
            );

            // Check for player-powerup collisions
            checkPickups();

            // Update active powerup effects on players
            updateActivePowerups(dt);

            // Cleanup collected powerups
            cleanupCollected();
        }

        /**
         * @brief Spawn a powerup at the given position
         * @param x X position
         * @param y Y position
         * @param type Optional specific type (random if not specified)
         * @return Entity ID of spawned powerup
         */
        Entity spawnPowerup(float x, float y, PowerupType type) {
            Entity powerup = m_registry->createEntity();

            m_registry->addComponent(powerup, TransformComponent(x, y, 0.0f, 1.0f, 1.0f));
            m_registry->addComponent(powerup, VelocityComponent(0.0f, 30.0f, 100.0f));
            m_registry->addComponent(powerup, PowerupComponent(type, getDuration(type), getValue(type)));
            
            // Add collision for pickup detection
            ColliderComponent collision(24.0f, 24.0f, CollisionLayer::Powerup);
            collision.mask = CollisionLayer::Player;
            collision.isTrigger = true;
            m_registry->addComponent(powerup, collision);

            // Auto-despawn after 15 seconds
            m_registry->addComponent(powerup, LifetimeComponent(15.0f));

            return powerup;
        }

        /**
         * @brief Set spawn chance for powerups
         */
        void setSpawnChance(float chance) {
            m_spawnChance = std::clamp(chance, 0.0f, 1.0f);
        }

        SystemPhase getPhase() const override { return SystemPhase::GameLogic; }

    private:
        EventBus& m_eventBus;
        float m_spawnChance;
        std::mt19937 m_rng;
        size_t m_enemyDeathSubId = 0;

        /**
         * @brief Try to spawn a powerup with configured probability
         */
        void trySpawnPowerup(float x, float y) {
            std::uniform_real_distribution<float> dist(0.0f, 1.0f);
            if (dist(m_rng) < m_spawnChance) {
                PowerupType type = getRandomType();
                spawnPowerup(x, y, type);
            }
        }

        /**
         * @brief Get a random powerup type with weighted probabilities
         */
        PowerupType getRandomType() {
            std::uniform_int_distribution<int> dist(0, 99);
            int roll = dist(m_rng);

            // Weighted distribution:
            // Health Up: 30%
            // Speed Boost: 20%
            // Spread Shot: 15%
            // Shield: 12%
            // Weapon Upgrade: 10%
            // Force Orb: 8%
            // Bomb: 5%
            if (roll < 30) return PowerupType::HEALTH_UP;
            if (roll < 50) return PowerupType::SPEED_BOOST;
            if (roll < 65) return PowerupType::SPREAD_SHOT;
            if (roll < 77) return PowerupType::SHIELD;
            if (roll < 87) return PowerupType::WEAPON_UPGRADE;
            if (roll < 95) return PowerupType::FORCE_ORB;
            return PowerupType::BOMB;
        }

        /**
         * @brief Get default duration for powerup type
         */
        float getDuration(PowerupType type) {
            switch (type) {
                case PowerupType::SPREAD_SHOT:    return 10.0f;
                case PowerupType::SPEED_BOOST:    return 8.0f;
                case PowerupType::SHIELD:         return 5.0f;
                default:                          return 0.0f;  // Instant effects
            }
        }

        /**
         * @brief Get default value for powerup type
         */
        float getValue(PowerupType type) {
            switch (type) {
                case PowerupType::HEALTH_UP:      return 1.0f;   // Restore 1 HP
                case PowerupType::SPEED_BOOST:    return 1.5f;   // 50% speed increase
                case PowerupType::WEAPON_UPGRADE: return 1.0f;   // +1 power level
                default:                          return 1.0f;
            }
        }

        /**
         * @brief Check for powerup-player collisions and apply effects
         */
        void checkPickups() {
            std::vector<std::pair<EntityId, EntityId>> pickups;

            m_registry->forEach<PowerupComponent, TransformComponent, ColliderComponent>(
                [this, &pickups](EntityId powerupEntity) {
                    auto& powerup = m_registry->getComponent<PowerupComponent>(powerupEntity);
                    if (powerup.isCollected) return;

                    const auto& powerupTransform = m_registry->getComponent<TransformComponent>(powerupEntity);
                    const auto& powerupCollision = m_registry->getComponent<ColliderComponent>(powerupEntity);

                    // Check against all players
                    m_registry->forEach<PlayerComponent, TransformComponent, ColliderComponent>(
                        [&](EntityId playerEntity) {
                            const auto& playerTransform = m_registry->getComponent<TransformComponent>(playerEntity);
                            const auto& playerCollision = m_registry->getComponent<ColliderComponent>(playerEntity);

                            if (checkCollision(powerupTransform, powerupCollision, playerTransform, playerCollision)) {
                                pickups.emplace_back(powerupEntity, playerEntity);
                            }
                        }
                    );
                }
            );

            // Apply pickups
            for (const auto& [powerupEntity, playerEntity] : pickups) {
                applyPowerup(powerupEntity, playerEntity);
            }
        }

        /**
         * @brief Check AABB collision between two entities
         */
        bool checkCollision(const TransformComponent& t1, const ColliderComponent& c1,
                           const TransformComponent& t2, const ColliderComponent& c2) {
            float left1 = t1.x + c1.offsetX - c1.width / 2.0f;
            float right1 = t1.x + c1.offsetX + c1.width / 2.0f;
            float top1 = t1.y + c1.offsetY - c1.height / 2.0f;
            float bottom1 = t1.y + c1.offsetY + c1.height / 2.0f;

            float left2 = t2.x + c2.offsetX - c2.width / 2.0f;
            float right2 = t2.x + c2.offsetX + c2.width / 2.0f;
            float top2 = t2.y + c2.offsetY - c2.height / 2.0f;
            float bottom2 = t2.y + c2.offsetY + c2.height / 2.0f;

            return !(left1 > right2 || right1 < left2 || top1 > bottom2 || bottom1 < top2);
        }

        /**
         * @brief Apply powerup effect to player
         */
        void applyPowerup(EntityId powerupEntity, EntityId playerEntity) {
            auto& powerup = m_registry->getComponent<PowerupComponent>(powerupEntity);
            auto& player = m_registry->getComponent<PlayerComponent>(playerEntity);

            powerup.isCollected = true;

            // Ensure player has ActivePowerupsComponent
            if (!m_registry->hasComponent<ActivePowerupsComponent>(playerEntity)) {
                m_registry->addComponent(playerEntity, ActivePowerupsComponent());
            }
            auto& active = m_registry->getComponent<ActivePowerupsComponent>(playerEntity);

            switch (powerup.type) {
                case PowerupType::HEALTH_UP:
                    // Restore health (handled by HealthComponent if exists)
                    player.lives = std::min(player.lives + 1, 5);  // Cap at 5 lives
                    break;

                case PowerupType::SPREAD_SHOT:
                    active.hasSpreadShot = true;
                    active.spreadShotTimer = powerup.duration;
                    if (m_registry->hasComponent<WeaponComponent>(playerEntity)) {
                        auto& weapon = m_registry->getComponent<WeaponComponent>(playerEntity);
                        weapon.projectileCount = 3;
                    }
                    break;

                case PowerupType::SPEED_BOOST:
                    active.hasSpeedBoost = true;
                    active.speedBoostTimer = powerup.duration;
                    if (m_registry->hasComponent<VelocityComponent>(playerEntity)) {
                        auto& vel = m_registry->getComponent<VelocityComponent>(playerEntity);
                        if (active.originalMaxSpeed == 0.0f) {
                            active.originalMaxSpeed = vel.maxSpeed;
                        }
                        vel.maxSpeed = active.originalMaxSpeed * powerup.value;
                    }
                    break;

                case PowerupType::SHIELD:
                    active.hasShield = true;
                    active.shieldTimer = powerup.duration;
                    break;

                case PowerupType::WEAPON_UPGRADE:
                    if (m_registry->hasComponent<WeaponComponent>(playerEntity)) {
                        auto& weapon = m_registry->getComponent<WeaponComponent>(playerEntity);
                        weapon.powerLevel = std::min(weapon.powerLevel + 1, 5);
                    }
                    break;

                case PowerupType::FORCE_ORB:
                    // Emit event for ForceOrbSystem to handle
                    m_eventBus.emit(events::SpawnForceOrb{playerEntity, 1});
                    break;

                case PowerupType::BOMB:
                    // Trigger screen-clearing bomb effect
                    activateBomb(playerEntity);
                    break;
            }

            // Emit pickup event
            m_eventBus.emit(events::PowerupCollected{
                static_cast<int>(powerupEntity),
                player.playerId,
                static_cast<int>(powerup.type)
            });

            // Add score for pickup
            player.addScore(100);
            m_eventBus.emit(events::ScoreChanged{
                player.playerId,
                player.score - 100,
                player.score,
                100,
                "powerup"
            });
        }

        /**
         * @brief Update active powerup timers and revert expired effects
         */
        void updateActivePowerups(float dt) {
            m_registry->forEach<ActivePowerupsComponent, PlayerComponent>(
                [this, dt](EntityId e) {
                    auto& active = m_registry->getComponent<ActivePowerupsComponent>(e);
                    
                    bool wasSpreadShot = active.hasSpreadShot;
                    bool wasSpeedBoost = active.hasSpeedBoost;

                    active.update(dt);

                    // Revert spread shot
                    if (wasSpreadShot && !active.hasSpreadShot) {
                        if (m_registry->hasComponent<WeaponComponent>(e)) {
                            auto& weapon = m_registry->getComponent<WeaponComponent>(e);
                            weapon.projectileCount = 1;
                        }
                    }

                    // Revert speed boost
                    if (wasSpeedBoost && !active.hasSpeedBoost) {
                        if (m_registry->hasComponent<VelocityComponent>(e)) {
                            auto& vel = m_registry->getComponent<VelocityComponent>(e);
                            if (active.originalMaxSpeed > 0) {
                                vel.maxSpeed = active.originalMaxSpeed;
                            }
                        }
                    }
                }
            );
        }

        /**
         * @brief Remove collected powerup entities
         */
        void cleanupCollected() {
            std::vector<EntityId> toRemove;

            m_registry->forEach<PowerupComponent>(
                [this, &toRemove](EntityId e) {
                    const auto& powerup = m_registry->getComponent<PowerupComponent>(e);
                    if (powerup.isCollected) {
                        toRemove.push_back(e);
                    }
                }
            );

            for (EntityId e : toRemove) {
                m_registry->destroyEntity(Entity{e});
            }
        }

        /**
         * @brief Activate bomb effect - destroy enemy projectiles and weak enemies
         */
        void activateBomb(EntityId playerEntity) {
            if (!m_registry->hasComponent<TransformComponent>(playerEntity)) return;

            const auto& playerTransform = m_registry->getComponent<TransformComponent>(playerEntity);
            
            int enemiesDestroyed = 0;
            int projectilesDestroyed = 0;

            std::vector<EntityId> toDestroy;

            // Destroy all enemy projectiles
            m_registry->forEach<ProjectileComponent, TransformComponent>(
                [this, &toDestroy, &projectilesDestroyed](EntityId projEntity) {
                    const auto& proj = m_registry->getComponent<ProjectileComponent>(projEntity);
                    if (!proj.isPlayerProjectile) {
                        toDestroy.push_back(projEntity);
                        projectilesDestroyed++;
                    }
                }
            );

            // Destroy weak enemies (health <= 2)
            m_registry->forEach<EnemyComponent, TransformComponent>(
                [this, &toDestroy, &enemiesDestroyed](EntityId enemyEntity) {
                    // Check if enemy has low health
                    if (m_registry->hasComponent<HealthComponent>(enemyEntity)) {
                        const auto& health = m_registry->getComponent<HealthComponent>(enemyEntity);
                        if (health.currentHealth <= 2) {
                            toDestroy.push_back(enemyEntity);
                            enemiesDestroyed++;
                        }
                    } else {
                        // No health component = instantly destroyable
                        toDestroy.push_back(enemyEntity);
                        enemiesDestroyed++;
                    }
                }
            );

            // Destroy entities
            for (EntityId e : toDestroy) {
                if (m_registry->entityExists(e)) {
                    // Award points for enemies
                    if (m_registry->hasComponent<EnemyComponent>(e)) {
                        if (m_registry->hasComponent<PlayerComponent>(playerEntity)) {
                            auto& player = m_registry->getComponent<PlayerComponent>(playerEntity);
                            player.addScore(50); // Reduced points for bomb kills
                        }
                        
                        // Get position for death effects
                        if (m_registry->hasComponent<TransformComponent>(e)) {
                            const auto& t = m_registry->getComponent<TransformComponent>(e);
                            m_eventBus.emit(events::EnemyDestroyed{e, playerEntity, 50, t.x, t.y});
                        }
                    }
                    m_registry->destroyEntity(Entity{e});
                }
            }

            // Emit bomb complete event
            m_eventBus.emit(events::BombComplete{
                playerEntity,
                enemiesDestroyed,
                projectilesDestroyed
            });

            // TODO: Add visual bomb effect (screen flash, particles)
        }
    };

} // namespace rtype::ecs
