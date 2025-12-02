/*
** R-Type ECS - LifetimeComponent
** Timed entity destruction data
*/

#pragma once

#include "../IComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Component for entities with limited lifetime
     *
     * Used by projectiles, particles, effects, etc. that should
     * automatically be destroyed after a certain time.
     */
    struct LifetimeComponent : public IComponent {
        float timeRemaining = 1.0f;   // Seconds until destruction
        float elapsedTime = 0.0f;     // Time since creation
        bool destroyOnExpire = true;  // Auto-destroy when expired?

        LifetimeComponent() = default;

        explicit LifetimeComponent(float lifetime)
            : timeRemaining(lifetime) {}

        LifetimeComponent(float lifetime, bool autoDestroy)
            : timeRemaining(lifetime), destroyOnExpire(autoDestroy) {}

        /**
         * @brief Update lifetime
         * @param dt Delta time
         * @return true if lifetime has expired
         */
        bool update(float dt) {
            elapsedTime += dt;
            timeRemaining -= dt;
            return isExpired();
        }

        /**
         * @brief Check if lifetime has expired
         */
        bool isExpired() const {
            return timeRemaining <= 0.0f;
        }

        /**
         * @brief Get normalized progress (0 to 1)
         */
        float getProgress() const {
            float totalLife = elapsedTime + timeRemaining;
            if (totalLife <= 0.0f) return 1.0f;
            return elapsedTime / totalLife;
        }

        std::string getTypeName() const override {
            return "LifetimeComponent";
        }
    };

} // namespace rtype::ecs
