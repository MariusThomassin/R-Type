/*
** R-Type ECS - HealthSystem
** System for processing health-related logic
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/components/HealthComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Event fired when an entity is healed
     */
    struct HealEvent {
        EntityId entity;
        int amount;
        int actualHealing;
    };

    /**
     * @brief System that processes health-related logic
     * 
     * Responsibilities:
     * - Process invincibility timers
     * - Apply damage and healing through events
     * - Detect and notify entity deaths
     * 
     * This moves logic out of HealthComponent, keeping components as pure data.
     */
    class HealthSystem : public ISystem {
        public:
            /**
             * @brief Construct a new Health System object
             */
            HealthSystem() = default;
            /**
             * @brief Destroy the Health System object
             */
            ~HealthSystem() override = default;

            /**
             * @brief Update all HealthComponent entities
             * @param dt Delta time since last update
             */
            void update(float dt) override {
                if (!m_registry) return;

                m_registry->forEach<HealthComponent>(
                    [this, dt](EntityId entity) {
                        auto& health = m_registry->getComponent<HealthComponent>(entity);
                        updateInvincibility(health, dt);
                    }
                );

                processDeathQueue();
            }

            /**
             * @brief Get the system phase (GameLogic)
             * @return SystemPhase
             */
            SystemPhase getPhase() const override {
                return SystemPhase::GameLogic;
            }

            /**
             * @brief Apply damage to an entity
             * @param entity Target entity
             * @param amount Damage amount
             * @param source Entity that caused the damage (optional)
             * @return Actual damage dealt
             */
            int applyDamage(EntityId entity, int amount, EntityId source = NULL_ENTITY) {
                if (!m_registry || !m_registry->hasComponent<HealthComponent>(entity)) {
                    return 0;
                }

                auto& health = m_registry->getComponent<HealthComponent>(entity);

                if (health.isInvincible || amount <= 0) {
                    return 0;
                }

                int actualDamage = std::min(amount, health.currentHealth);
                health.currentHealth -= actualDamage;

                if (m_eventBus) {
                    DamageEvent evt;
                    evt.targetId = entity;
                    evt.sourceId = source;
                    evt.damage = actualDamage;
                    evt.remainingHealth = health.currentHealth;
                    m_eventBus->emit(evt);
                }

                if (health.currentHealth <= 0) {
                    m_deathQueue.push_back({entity, source});
                }

                return actualDamage;
            }

            /**
             * @brief Apply healing to an entity
             * @param entity Target entity
             * @param amount Heal amount
             * @return Actual healing done
             */
            int applyHealing(EntityId entity, int amount) {
                if (!m_registry || !m_registry->hasComponent<HealthComponent>(entity)) {
                    return 0;
                }

                auto& health = m_registry->getComponent<HealthComponent>(entity);

                if (amount <= 0) {
                    return 0;
                }

                int actualHeal = std::min(amount, health.maxHealth - health.currentHealth);
                health.currentHealth += actualHeal;

                if (m_eventBus) {
                    HealEvent evt{entity, amount, actualHeal};
                    m_eventBus->emit(evt);
                }

                return actualHeal;
            }

            /**
             * @brief Grant temporary invincibility
             * @param entity Target entity
             * @param duration Duration in seconds
             */
            void grantInvincibility(EntityId entity, float duration) {
                if (!m_registry || !m_registry->hasComponent<HealthComponent>(entity)) {
                    return;
                }

                auto& health = m_registry->getComponent<HealthComponent>(entity);
                health.isInvincible = true;
                health.invincibilityTimer = duration;
            }

            /**
             * @brief Check if an entity is dead
             * @param entity Target entity
             * @return true if dead
             */
            bool isDead(EntityId entity) const {
                if (!m_registry || !m_registry->hasComponent<HealthComponent>(entity)) {
                    return true;
                }
                const auto& health = m_registry->getComponent<HealthComponent>(entity);
                return health.currentHealth <= 0;
            }

            /**
             * @brief Set event bus for health events
             * @param eventBus Pointer to EventBus
             */
            void setEventBus(EventBus* eventBus) {
                m_eventBus = eventBus;
            }

        private:
            /**
             * @brief Update invincibility timer for a HealthComponent
             * @param health Reference to HealthComponent
             * @param dt Delta time since last update
             */
            void updateInvincibility(HealthComponent& health, float dt) {
                if (health.isInvincible && health.invincibilityTimer > 0.0f) {
                    health.invincibilityTimer -= dt;
                    if (health.invincibilityTimer <= 0.0f) {
                        health.isInvincible = false;
                        health.invincibilityTimer = 0.0f;
                    }
                }
            }

            /**
             * @brief Process the death queue and emit DeathEvents
             */
            void processDeathQueue() {
                for (const auto& death : m_deathQueue) {
                    if (m_eventBus) {
                        DeathEvent evt{death.entity, death.killer};
                        m_eventBus->emit(evt);
                    }
                }
                m_deathQueue.clear();
            }

            /**
             * @brief Struct for pending death notifications
             */
            struct PendingDeath {
                EntityId entity;
                EntityId killer;
            };

            /**
             * @brief Queue of entities that have died this update
             */
            std::vector<PendingDeath> m_deathQueue;
            /**
             * @brief Pointer to EventBus for emitting events
             */
            EventBus* m_eventBus = nullptr;
        };
} // namespace rtype::ecs
