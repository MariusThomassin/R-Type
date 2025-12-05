/*
** R-Type ECS - MovementSystem
** Handles entity movement and physics
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"

#include <cmath>

namespace rtype::ecs {

    /**
     * @brief System that updates entity positions based on velocity
     *
     * Processes all entities with Transform and Velocity components.
     * Optimized for high entity counts using cache-friendly iteration.
     */
    class MovementSystem : public ISystem {
        public:
            /**
             * @brief Construct a new Movement System object
             */
            MovementSystem() = default;
            /**
             * @brief Destroy the Movement System object
             */
            ~MovementSystem() override = default;

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
        };
} // namespace rtype::ecs
