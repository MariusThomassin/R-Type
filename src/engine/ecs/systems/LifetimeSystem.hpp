/*
** R-Type ECS - Lifetime System
** Handles entity lifetime updates and cleanup
** Used for projectiles, effects, particles, temporary entities
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "engine/ecs/components/TransformComponent.hpp"

#include <vector>
#include <functional>

namespace rtype::ecs {

    /**
     * @brief Configuration for the LifetimeSystem
     */
    struct LifetimeSystemConfig {
        int screenWidth = 1920;
        int screenHeight = 1080;
        float offscreenMargin = 50.0f;
        bool destroyOffscreen = true;
    };

    /**
     * @brief System that manages entity lifetimes and cleanup
     * 
     * Handles:
     * - Time-based lifetime expiration (LifetimeComponent)
     * - Off-screen entity destruction
     * - Optional callback for entity destruction events
     */
    class LifetimeSystem : public ISystem {
        public:
            /**
             * @brief Callback type for entity expiration events
             */
            using OnEntityExpired = std::function<void(EntityId)>;

            /**
             * @brief Construct a new Lifetime System object
             * @param config Configuration options
             */
            explicit LifetimeSystem(LifetimeSystemConfig config = {}) : m_config(config) {}

            /**
             * @brief Destroy the Lifetime System object
             */
            ~LifetimeSystem() override = default;

            /**
             * @brief Update the system
             * @param dt Delta time since last update
             */
            void update(float dt) override;

            /**
             * @brief Get the system's execution phase
             * @return SystemPhase The phase in which this system runs
             */
            SystemPhase getPhase() const override;

            /**
             * @brief Set callback for when entities expire
             * @param callback The callback function
             */
            void setOnExpired(OnEntityExpired callback);

            /**
             * @brief Update screen dimensions for off-screen detection
             * @param width Screen width in pixels
             * @param height Screen height in pixels
             */
            void setScreenSize(int width, int height);

            /**
             * @brief Set the margin beyond screen edges before destruction
             * @param margin Margin in pixels
             */
            void setOffscreenMargin(float margin);

            /**
             * @brief Enable/disable off-screen destruction
             * @param enabled true to enable, false to disable
             */
            void setDestroyOffscreen(bool enabled);

            /**
             * @brief Get entities that expired in the last update
             * Useful for spawning effects, playing sounds, etc.
             * @return Vector of expired entity IDs
             */
            const std::vector<EntityId>& getExpiredEntities() const;

            /**
             * @brief Get entities destroyed for being off-screen last frame
             * @return Vector of off-screen destroyed entity IDs
             */
            const std::vector<EntityId>& getOffscreenEntities() const;

        private:
            /**
             * @brief Update lifetimes of entities with LifetimeComponent
             * @param dt Delta time since last update
             */
            void updateLifetimes(float dt);
            /**
             * @brief Destroy entities that moved off-screen
             */
            void destroyOffscreen();
            /**
             * @brief Destroy entities that have expired
             */
            void destroyExpired();

            /**
             * @brief Configuration options for the system
             */
            LifetimeSystemConfig m_config;
            /**
             * @brief Callback for entity expiration events
             */
            OnEntityExpired m_onExpired;

            /**
             * @brief Entities that expired in the last update
             */
            std::vector<EntityId> m_expiredLastFrame;
            /**
             * @brief Entities destroyed for being off-screen in the last update
             */
            std::vector<EntityId> m_offscreenLastFrame;
        };
} // namespace rtype::ecs
