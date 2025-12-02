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
        MovementSystem() = default;
        ~MovementSystem() override = default;

        void update(float dt) override {
            if (!m_registry) return;

            // Use forEachWith for direct component access (avoids repeated lookups)
            m_registry->forEachWith<TransformComponent, VelocityComponent>(
                [dt](EntityId entity, TransformComponent& transform, VelocityComponent& velocity) {
                    (void)entity;  // Unused but required for interface
                    
                    // Apply acceleration
                    velocity.vx += velocity.ax * dt;
                    velocity.vy += velocity.ay * dt;

                    // Clamp to max speed
                    float speedSq = velocity.vx * velocity.vx + velocity.vy * velocity.vy;
                    float maxSpeedSq = velocity.maxSpeed * velocity.maxSpeed;
                    if (speedSq > maxSpeedSq && speedSq > 0.0f) {
                        float invSpeed = velocity.maxSpeed / std::sqrt(speedSq);
                        velocity.vx *= invSpeed;
                        velocity.vy *= invSpeed;
                    }

                    // Apply velocity to position
                    transform.x += velocity.vx * dt;
                    transform.y += velocity.vy * dt;
                }
            );
        }

        SystemPhase getPhase() const override {
            return SystemPhase::Physics;
        }
    };

} // namespace rtype::ecs
