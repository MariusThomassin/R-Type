/*
** R-Type ECS - HealthComponent
** Health and damage state data (pure data - logic moved to HealthSystem)
*/

#pragma once

#include "../core/IComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Component holding health and damage-related data
     *
     * This is a pure data component. Logic is handled by HealthSystem.
     * Used by entities that can take damage and be destroyed.
     */
    struct HealthComponent : public IComponent {
        int currentHealth = 100;
        int maxHealth = 100;
        bool isInvincible = false;
        float invincibilityTimer = 0.0f;

        HealthComponent() = default;

        explicit HealthComponent(int health)
            : currentHealth(health), maxHealth(health) {}

        HealthComponent(int current, int max)
            : currentHealth(current), maxHealth(max) {}

        /**
         * @brief Check if entity is dead (data query, not logic)
         */
        bool isDead() const {
            return currentHealth <= 0;
        }

        /**
         * @brief Get health percentage (data query)
         */
        float getHealthPercent() const {
            if (maxHealth <= 0) return 0.0f;
            return static_cast<float>(currentHealth) / static_cast<float>(maxHealth);
        }

        std::string getTypeName() const override {
            return "HealthComponent";
        }
    };

} // namespace rtype::ecs
