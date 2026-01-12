/*
** R-Type ECS - ForceOrbSystem
** Handles Force orb positioning, targeting, and auto-firing
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "game/components/ForceOrbComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/components/SpritesheetComponent.hpp"
#include "game/components/BulletTypes.hpp"
#include "engine/ecs/events/InputEvents.hpp"
#include "engine/ecs/events/definitions/GameEvents.hpp"

#include <cmath>
#include <limits>

namespace rtype::ecs {

    /**
     * @brief System that manages Force orb behavior
     * 
     * Handles:
     * - Positioning orbs relative to their owner player
     * - Finding and tracking nearest enemy targets
     * - Auto-firing lasers at targets
     * - Orb side switching on player input
     * - Visual effects (rotation, glow)
     */
    class ForceOrbSystem : public ISystem {
    public:
        /**
         * @brief Construct ForceOrbSystem
         * @param eventBus Reference to EventBus for input events
         */
        explicit ForceOrbSystem(EventBus& eventBus)
            : m_eventBus(eventBus) {
            
            // Subscribe to orb switch input
            m_switchSubId = m_eventBus.subscribe<events::KeyPressedEvent>(
                [this](const events::KeyPressedEvent& e) {
                    // Tab key switches orb side
                    if (e.key == events::KeyCode::Tab) {
                        switchAllOrbSides();
                    }
                }
            );
            
            // Subscribe to gamepad button for orb switch (RB)
            m_gamepadSwitchSubId = m_eventBus.subscribe<events::GamepadButtonPressedEvent>(
                [this](const events::GamepadButtonPressedEvent& e) {
                    if (e.button == events::GamepadButton::RightBumper) {
                        switchAllOrbSides();
                    }
                }
            );

            // Subscribe to SpawnForceOrb event from PowerupSystem
            m_spawnOrbSubId = m_eventBus.subscribe<events::SpawnForceOrb>(
                [this](const events::SpawnForceOrb& e) {
                    spawnOrb(e.playerId, e.level);
                }
            );
        }

        ~ForceOrbSystem() override {
            m_eventBus.unsubscribe<events::KeyPressedEvent>(m_switchSubId);
            m_eventBus.unsubscribe<events::GamepadButtonPressedEvent>(m_gamepadSwitchSubId);
            m_eventBus.unsubscribe<events::SpawnForceOrb>(m_spawnOrbSubId);
        }

        /**
         * @brief Update all Force orbs
         */
        void update(float dt) override {
            if (!m_registry) return;

            m_registry->forEach<ForceOrbComponent, TransformComponent>(
                [this, dt](EntityId orbEntity) {
                    auto& orb = m_registry->getComponent<ForceOrbComponent>(orbEntity);
                    auto& orbTransform = m_registry->getComponent<TransformComponent>(orbEntity);

                    if (!orb.isActive) return;

                    // Update visual effects
                    updateVisualEffects(orb, dt);

                    // Position orb relative to owner
                    if (!positionOrbRelativeToOwner(orb, orbTransform)) {
                        // Owner doesn't exist, destroy orb
                        m_orbsToDestroy.push_back(orbEntity);
                        return;
                    }

                    // Find and track target
                    updateTarget(orb, orbTransform);

                    // Auto-fire at target
                    updateFiring(orbEntity, orb, orbTransform, dt);
                }
            );

            // Cleanup destroyed orbs
            for (EntityId orb : m_orbsToDestroy) {
                if (m_registry->entityExists(orb)) {
                    m_registry->destroyEntity(orb);
                }
            }
            m_orbsToDestroy.clear();
        }

        /**
         * @brief Spawn a Force orb for a player
         * @param playerId The player entity to attach orb to
         * @param level Initial orb level (1-3)
         * @return Entity ID of the spawned orb
         */
        Entity spawnOrb(EntityId playerId, int level = 1) {
            if (!m_registry || !m_registry->entityExists(playerId)) {
                return Entity{NULL_ENTITY};
            }

            // Check if player already has an orb
            bool hasOrb = false;
            m_registry->forEach<ForceOrbComponent>([&](EntityId existing) {
                auto& existingOrb = m_registry->getComponent<ForceOrbComponent>(existing);
                if (existingOrb.ownerId == playerId) {
                    // Upgrade existing orb instead
                    existingOrb.upgrade();
                    hasOrb = true;
                }
            });

            if (hasOrb) {
                return Entity{NULL_ENTITY}; // Upgraded existing orb
            }

            // Get player position for initial spawn
            const auto& playerTransform = m_registry->getComponent<TransformComponent>(playerId);

            Entity orb = m_registry->createEntity();

            // Add ForceOrbComponent
            m_registry->addComponent(orb, ForceOrbComponent(playerId, OrbDockSide::Left, level));

            // Add Transform at player position (will be adjusted in update)
            m_registry->addComponent(orb, TransformComponent(
                playerTransform.x - 40.0f,
                playerTransform.y,
                0.0f, 1.0f, 1.0f
            ));

            // Add collider for potential projectile absorption
            ColliderComponent collider(24.0f, 24.0f, CollisionLayer::Player);
            collider.isTrigger = true;
            m_registry->addComponent(orb, collider);

            // Add visual component (using spritesheet for now)
            SpritesheetComponent sprite;
            sprite.bulletType = BulletType::Orb;
            sprite.bulletColor = static_cast<BulletColor>(level - 1); // Color based on level
            sprite.glowEnabled = true;
            sprite.glowIntensity = 1.5f;
            m_registry->addComponent(orb, sprite);

            return orb;
        }

        /**
         * @brief Switch orb side for all orbs owned by a player
         * @param playerId Player to switch orbs for (NULL_ENTITY = all players)
         */
        void switchOrbSide(EntityId playerId = NULL_ENTITY) {
            if (!m_registry) return;

            m_registry->forEach<ForceOrbComponent>([&](EntityId orbEntity) {
                auto& orb = m_registry->getComponent<ForceOrbComponent>(orbEntity);
                if (playerId == NULL_ENTITY || orb.ownerId == playerId) {
                    orb.toggleSide();
                }
            });
        }

        SystemPhase getPhase() const override { return SystemPhase::GameLogic; }

    private:
        EventBus& m_eventBus;
        EventBus::SubscriberId m_switchSubId = 0;
        EventBus::SubscriberId m_gamepadSwitchSubId = 0;
        EventBus::SubscriberId m_spawnOrbSubId = 0;
        std::vector<EntityId> m_orbsToDestroy;

        /**
         * @brief Switch all orbs to opposite side (called on input)
         */
        void switchAllOrbSides() {
            switchOrbSide(NULL_ENTITY);
        }

        /**
         * @brief Update orb visual effects (rotation, glow)
         */
        void updateVisualEffects(ForceOrbComponent& orb, float dt) {
            // Rotate the orb
            orb.currentRotation += orb.rotationSpeed * dt;
            if (orb.currentRotation >= 360.0f) {
                orb.currentRotation -= 360.0f;
            }

            // Pulsing glow effect
            orb.glowTimer += dt * 3.0f;
            orb.glowIntensity = 0.8f + 0.4f * std::sin(orb.glowTimer);
        }

        /**
         * @brief Position orb relative to its owner
         * @return false if owner doesn't exist
         */
        bool positionOrbRelativeToOwner(ForceOrbComponent& orb, TransformComponent& orbTransform) {
            if (!m_registry->entityExists(orb.ownerId)) {
                return false;
            }

            if (!m_registry->hasComponent<TransformComponent>(orb.ownerId)) {
                return false;
            }

            const auto& ownerTransform = m_registry->getComponent<TransformComponent>(orb.ownerId);

            // Position orb to left or right of owner
            orbTransform.x = ownerTransform.x + orb.getDockOffsetX();
            orbTransform.y = ownerTransform.y;
            orbTransform.rotation = orb.currentRotation;

            return true;
        }

        /**
         * @brief Find and update current target
         */
        void updateTarget(ForceOrbComponent& orb, const TransformComponent& orbTransform) {
            EntityId bestTarget = NULL_ENTITY;
            float bestDistance = std::numeric_limits<float>::max();

            // Find nearest enemy within range
            m_registry->forEach<EnemyComponent, TransformComponent>([&](EntityId enemyEntity) {
                const auto& enemyTransform = m_registry->getComponent<TransformComponent>(enemyEntity);

                float dx = enemyTransform.x - orbTransform.x;
                float dy = enemyTransform.y - orbTransform.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < orb.targetRange && distance < bestDistance) {
                    bestDistance = distance;
                    bestTarget = enemyEntity;
                }
            });

            orb.currentTarget = bestTarget;
        }

        /**
         * @brief Fire lasers at current target
         */
        void updateFiring(EntityId orbEntity, ForceOrbComponent& orb, 
                         const TransformComponent& orbTransform, float dt) {
            // Update fire timer
            orb.fireTimer -= dt;

            if (orb.fireTimer > 0.0f) {
                return; // Still on cooldown
            }

            // Fire if we have a target OR fire straight ahead if no target
            orb.fireTimer = orb.fireRate;

            float targetX, targetY;
            if (orb.currentTarget != NULL_ENTITY && 
                m_registry->entityExists(orb.currentTarget) &&
                m_registry->hasComponent<TransformComponent>(orb.currentTarget)) {
                
                const auto& targetTransform = m_registry->getComponent<TransformComponent>(orb.currentTarget);
                targetX = targetTransform.x;
                targetY = targetTransform.y;
            } else {
                // No target, fire in direction orb is facing
                float fireDirection = (orb.dockSide == OrbDockSide::Left) ? -1.0f : 1.0f;
                targetX = orbTransform.x + fireDirection * 500.0f;
                targetY = orbTransform.y;
            }

            // Calculate direction to target
            float dx = targetX - orbTransform.x;
            float dy = targetY - orbTransform.y;
            float length = std::sqrt(dx * dx + dy * dy);

            if (length > 0.0f) {
                dx /= length;
                dy /= length;
            } else {
                dx = (orb.dockSide == OrbDockSide::Left) ? -1.0f : 1.0f;
                dy = 0.0f;
            }

            // Spawn laser projectile
            spawnOrbLaser(orbTransform.x, orbTransform.y, 
                         dx * orb.laserSpeed, dy * orb.laserSpeed,
                         orb.damage, orb.level, orb.ownerId);
        }

        /**
         * @brief Spawn a laser projectile from the orb
         */
        void spawnOrbLaser(float x, float y, float velX, float velY, 
                          float damage, int level, EntityId ownerId) {
            Entity laser = m_registry->createEntity();

            m_registry->addComponent(laser, TransformComponent(x, y, 0.0f, 0.5f, 0.5f));
            m_registry->addComponent(laser, VelocityComponent(velX, velY, 800.0f));

            ProjectileComponent proj;
            proj.damage = static_cast<int>(damage);
            proj.isPlayerProjectile = true;
            proj.ownerId = ownerId;
            m_registry->addComponent(laser, proj);

            // Small collision box for laser
            ColliderComponent collider(12.0f, 4.0f, CollisionLayer::PlayerProjectile);
            collider.mask = CollisionLayer::Enemy;
            m_registry->addComponent(laser, collider);

            // Auto-destroy after 2 seconds
            m_registry->addComponent(laser, LifetimeComponent(2.0f));

            // Visual - use laser bullet type
            SpritesheetComponent sprite;
            sprite.bulletType = BulletType::Laser;
            sprite.bulletColor = static_cast<BulletColor>(level - 1);
            sprite.glowEnabled = true;
            sprite.glowIntensity = 1.2f;
            m_registry->addComponent(laser, sprite);
        }
    };

} // namespace rtype::ecs
