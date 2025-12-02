/*
** R-Type ECS - MovementSystem
** Handles entity movement and physics
*/

#pragma once

#include "../ISystem.hpp"
#include "../Registry.hpp"
#include "../components/TransformComponent.hpp"
#include "../components/VelocityComponent.hpp"

#include <cmath>

namespace rtype::ecs {

    /**
     * @brief System that updates entity positions based on velocity
     *
     * Processes all entities with Transform and Velocity components.
     */
    class MovementSystem : public ISystem {
    public:
        MovementSystem() = default;
        ~MovementSystem() override = default;

        void update(float dt) override {
            if (!m_registry) return;

            auto entities = m_registry->getEntitiesWith<TransformComponent, VelocityComponent>();

            for (EntityId entity : entities) {
                auto& transform = m_registry->getComponent<TransformComponent>(entity);
                auto& velocity = m_registry->getComponent<VelocityComponent>(entity);

                velocity.vx += velocity.ax * dt;
                velocity.vy += velocity.ay * dt;

                float speed = std::sqrt(velocity.vx * velocity.vx + velocity.vy * velocity.vy);
                if (speed > velocity.maxSpeed && speed > 0.0f) {
                    float scale = velocity.maxSpeed / speed;
                    velocity.vx *= scale;
                    velocity.vy *= scale;
                }

                transform.x += velocity.vx * dt;
                transform.y += velocity.vy * dt;
            }
        }

        SystemPhase getPhase() const override {
            return SystemPhase::Physics;
        }
    };

} // namespace rtype::ecs
