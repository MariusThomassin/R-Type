/*
** R-Type ECS - EnemyAISystem
** Handles enemy AI behaviors based on enemy type
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "engine/ecs/events/definitions/GameEvents.hpp"

#include <cmath>
#include <vector>
#include <limits>

namespace rtype::ecs {

    /**
     * @brief System that controls enemy AI behaviors
     * 
     * Handles different AI patterns based on EnemyType:
     * - Basic: Move in straight line
     * - Chaser: Follow nearest player
     * - Shooter: Move and aim at player
     * - Turret: Stationary, rotate to aim
     * - Boss: Complex multi-phase behavior
     */
    class EnemyAISystem : public ISystem {
    public:
        /**
         * @brief Construct EnemyAISystem
         * @param eventBus Reference to EventBus
         * @param screenWidth Screen width for boundary checks
         * @param screenHeight Screen height for boundary checks
         */
        EnemyAISystem(EventBus& eventBus, int screenWidth = 1280, int screenHeight = 720)
            : m_eventBus(eventBus)
            , m_screenWidth(screenWidth)
            , m_screenHeight(screenHeight) {}

        ~EnemyAISystem() override = default;

        /**
         * @brief Update all enemy AI behaviors
         */
        void update(float dt) override {
            if (!m_registry) return;

            m_totalTime += dt;

            // Cache player positions for targeting
            cachePlayerPositions();

            // Process each enemy type
            m_registry->forEach<EnemyComponent, TransformComponent, VelocityComponent>(
                [this, dt](EntityId e) {
                    auto& enemy = m_registry->getComponent<EnemyComponent>(e);
                    auto& transform = m_registry->getComponent<TransformComponent>(e);
                    auto& velocity = m_registry->getComponent<VelocityComponent>(e);

                    switch (enemy.type) {
                        case EnemyType::Basic:
                            updateBasicAI(e, transform, velocity, dt);
                            break;
                        case EnemyType::Chaser:
                            updateChaserAI(e, transform, velocity, dt);
                            break;
                        case EnemyType::Shooter:
                            updateShooterAI(e, transform, velocity, dt);
                            break;
                        case EnemyType::Turret:
                            updateTurretAI(e, transform, velocity, dt);
                            break;
                        case EnemyType::Boss:
                            updateBossAI(e, enemy, transform, velocity, dt);
                            break;
                    }

                    // Check for off-screen cleanup
                    checkBoundaries(e, transform);
                }
            );

            // Handle enemy shooting
            updateEnemyShooting(dt);
        }

        SystemPhase getPhase() const override { return SystemPhase::GameLogic; }

    private:
        EventBus& m_eventBus;
        int m_screenWidth;
        int m_screenHeight;
        float m_totalTime = 0.0f;

        // Cached player positions for targeting
        struct PlayerPos {
            EntityId id;
            float x, y;
        };
        std::vector<PlayerPos> m_playerPositions;

        /**
         * @brief Cache current player positions for AI targeting
         */
        void cachePlayerPositions() {
            m_playerPositions.clear();

            m_registry->forEach<PlayerComponent, TransformComponent>(
                [this](EntityId e) {
                    const auto& transform = m_registry->getComponent<TransformComponent>(e);
                    m_playerPositions.push_back({e, transform.x, transform.y});
                }
            );
        }

        /**
         * @brief Find nearest player to a position
         * @return Pointer to nearest player position, or nullptr if none
         */
        const PlayerPos* findNearestPlayer(float x, float y) {
            const PlayerPos* nearest = nullptr;
            float minDistSq = std::numeric_limits<float>::max();

            for (const auto& player : m_playerPositions) {
                float dx = player.x - x;
                float dy = player.y - y;
                float distSq = dx * dx + dy * dy;
                if (distSq < minDistSq) {
                    minDistSq = distSq;
                    nearest = &player;
                }
            }

            return nearest;
        }

        /**
         * @brief Basic AI: Move left in straight line
         */
        void updateBasicAI(EntityId e, TransformComponent& transform, VelocityComponent& velocity, float dt) {
            (void)e;
            (void)transform;
            (void)dt;
            
            // Simple: maintain leftward velocity
            velocity.vx = -150.0f;
            velocity.vy = 0.0f;
        }

        /**
         * @brief Chaser AI: Follow nearest player
         */
        void updateChaserAI(EntityId e, TransformComponent& transform, VelocityComponent& velocity, float dt) {
            (void)e;
            (void)dt;

            const PlayerPos* target = findNearestPlayer(transform.x, transform.y);
            if (!target) {
                // No player - move left
                velocity.vx = -100.0f;
                velocity.vy = 0.0f;
                return;
            }

            // Calculate direction to player
            float dx = target->x - transform.x;
            float dy = target->y - transform.y;
            float dist = std::sqrt(dx * dx + dy * dy);

            if (dist > 10.0f) {
                float speed = velocity.maxSpeed;
                velocity.vx = (dx / dist) * speed * 0.6f;  // Move toward player at 60% max speed
                velocity.vy = (dy / dist) * speed * 0.6f;
            }

            // Always drift left somewhat
            velocity.vx -= 50.0f;
        }

        /**
         * @brief Shooter AI: Move in sinusoidal pattern, aim at player
         */
        void updateShooterAI(EntityId e, TransformComponent& transform, VelocityComponent& velocity, float dt) {
            (void)dt;

            // Sinusoidal vertical movement
            float waveOffset = static_cast<float>(e) * 0.5f;  // Different phase per enemy
            velocity.vy = std::sin(m_totalTime * 2.0f + waveOffset) * 100.0f;
            
            // Constant leftward movement
            velocity.vx = -80.0f;

            // Update rotation to face player
            const PlayerPos* target = findNearestPlayer(transform.x, transform.y);
            if (target) {
                float dx = target->x - transform.x;
                float dy = target->y - transform.y;
                transform.rotation = std::atan2(dy, dx) * (180.0f / 3.14159f);
            }
        }

        /**
         * @brief Turret AI: Stationary, rotate to track player
         */
        void updateTurretAI(EntityId e, TransformComponent& transform, VelocityComponent& velocity, float dt) {
            (void)e;
            (void)dt;

            // Stationary
            velocity.vx = 0.0f;
            velocity.vy = 0.0f;

            // Rotate to face nearest player
            const PlayerPos* target = findNearestPlayer(transform.x, transform.y);
            if (target) {
                float dx = target->x - transform.x;
                float dy = target->y - transform.y;
                float targetAngle = std::atan2(dy, dx) * (180.0f / 3.14159f);
                
                // Smooth rotation
                float angleDiff = targetAngle - transform.rotation;
                while (angleDiff > 180.0f) angleDiff -= 360.0f;
                while (angleDiff < -180.0f) angleDiff += 360.0f;
                
                transform.rotation += angleDiff * dt * 3.0f;  // Rotation speed
            }
        }

        /**
         * @brief Boss AI: Complex multi-phase behavior
         */
        void updateBossAI(EntityId e, EnemyComponent& enemy, TransformComponent& transform, 
                         VelocityComponent& velocity, float dt) {
            (void)dt;

            // Phase based on health
            float healthPercent = 1.0f;
            if (m_registry->hasComponent<HealthComponent>(e)) {
                const auto& health = m_registry->getComponent<HealthComponent>(e);
                healthPercent = health.currentHealth / health.maxHealth;
            }

            int phase = (healthPercent > 0.66f) ? 1 :
                       (healthPercent > 0.33f) ? 2 : 3;

            // Movement pattern based on phase
            switch (phase) {
                case 1:
                    // Phase 1: Slow vertical movement
                    velocity.vx = 0.0f;
                    velocity.vy = std::sin(m_totalTime) * 50.0f;
                    break;
                case 2:
                    // Phase 2: Faster, more erratic
                    velocity.vx = std::sin(m_totalTime * 0.5f) * 30.0f;
                    velocity.vy = std::sin(m_totalTime * 2.0f) * 100.0f;
                    break;
                case 3:
                    // Phase 3: Aggressive chasing
                    {
                        const PlayerPos* target = findNearestPlayer(transform.x, transform.y);
                        if (target) {
                            float dx = target->x - transform.x;
                            float dy = target->y - transform.y;
                            float dist = std::sqrt(dx * dx + dy * dy);
                            if (dist > 200.0f) {
                                velocity.vx = (dx / dist) * 80.0f;
                                velocity.vy = (dy / dist) * 80.0f;
                            }
                        }
                    }
                    break;
            }

            // Emit phase change event if phase changed
            static int lastPhase = 1;
            if (phase != lastPhase) {
                m_eventBus.emit(events::BossPhaseChanged{e, lastPhase, phase});
                lastPhase = phase;
            }

            (void)enemy;  // Could use difficulty for behavior scaling
        }

        /**
         * @brief Handle enemy shooting logic
         */
        void updateEnemyShooting(float dt) {
            m_registry->forEach<EnemyComponent, WeaponComponent, TransformComponent>(
                [this, dt](EntityId e) {
                    auto& weapon = m_registry->getComponent<WeaponComponent>(e);
                    const auto& transform = m_registry->getComponent<TransformComponent>(e);
                    const auto& enemy = m_registry->getComponent<EnemyComponent>(e);

                    // Update cooldown
                    weapon.lastFiredTime += dt;

                    // Check if can fire
                    if (weapon.lastFiredTime >= weapon.fireRate) {
                        // Only shoot if player is in front (left of enemy)
                        const PlayerPos* target = findNearestPlayer(transform.x, transform.y);
                        if (target && target->x < transform.x) {
                            // Calculate direction to player
                            float dx = target->x - transform.x;
                            float dy = target->y - transform.y;
                            float dist = std::sqrt(dx * dx + dy * dy);
                            
                            if (dist > 0 && dist < 800.0f) {  // Max shooting range
                                // Emit shoot event
                                m_eventBus.emit(events::SpawnProjectile{
                                    e,
                                    transform.x - 20.0f,  // Spawn in front
                                    transform.y,
                                    dx / dist,  // Direction
                                    dy / dist,
                                    weapon.projectileSpeed,
                                    weapon.damage,
                                    false  // Enemy projectile
                                });

                                weapon.lastFiredTime = 0.0f;
                            }
                        }
                    }

                    (void)enemy;  // Could modify behavior based on enemy type
                }
            );
        }

        /**
         * @brief Check if enemy is off-screen and should be destroyed
         */
        void checkBoundaries(EntityId e, const TransformComponent& transform) {
            const float MARGIN = 100.0f;

            if (transform.x < -MARGIN || 
                transform.x > m_screenWidth + MARGIN * 2 ||
                transform.y < -MARGIN || 
                transform.y > m_screenHeight + MARGIN) {
                
                // Queue for destruction
                m_registry->destroyEntity(Entity{e});
            }
        }
    };

} // namespace rtype::ecs
