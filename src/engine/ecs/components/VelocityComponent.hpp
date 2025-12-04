/*
** R-Type ECS - VelocityComponent
** Movement velocity and speed data
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Component holding velocity and movement constraints
     *
     * Used by entities that can move.
     */
    struct VelocityComponent : public IComponent {
        float vx = 0.0f;          // Velocity X
        float vy = 0.0f;          // Velocity Y
        float maxSpeed = 500.0f;  // Maximum speed limit
        float ax = 0.0f;          // Acceleration X
        float ay = 0.0f;          // Acceleration Y

        VelocityComponent() = default;

        VelocityComponent(float velX, float velY)
            : vx(velX), vy(velY) {}

        VelocityComponent(float velX, float velY, float max)
            : vx(velX), vy(velY), maxSpeed(max) {}

        std::string getTypeName() const override {
            return "VelocityComponent";
        }
    };

} // namespace rtype::ecs
