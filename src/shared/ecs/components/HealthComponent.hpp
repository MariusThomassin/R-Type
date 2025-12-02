/*
** R-Type ECS - HealthComponent
** Health and damage state data
*/

#pragma once

#include "../IComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Component holding health and damage-related data
     *
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
         * @brief Check if entity is dead
         */
        bool isDead() const {
            return currentHealth <= 0;
        }

        /**
         * @brief Apply damage (respects invincibility)
         * @param amount Damage amount
         * @return Actual damage dealt
         */
        int takeDamage(int amount) {
            if (isInvincible || amount <= 0) return 0;

            int actualDamage = std::min(amount, currentHealth);
            currentHealth -= actualDamage;
            return actualDamage;
        }

        /**
         * @brief Heal the entity
         * @param amount Heal amount
         * @return Actual healing done
         */
        int heal(int amount) {
            if (amount <= 0) return 0;

            int actualHeal = std::min(amount, maxHealth - currentHealth);
            currentHealth += actualHeal;
            return actualHeal;
        }

        std::string getTypeName() const override {
            return "HealthComponent";
        }
    };

} // namespace rtype::ecs
