/*
** R-Type ECS - MovementSystem Implementation
** Handles entity movement and physics
*/

#include "MovementSystem.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include <cmath>

namespace rtype::ecs {

    void MovementSystem::update(float dt) {
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

    SystemPhase MovementSystem::getPhase() const {
        return SystemPhase::Physics;
    }

} // namespace rtype::ecs