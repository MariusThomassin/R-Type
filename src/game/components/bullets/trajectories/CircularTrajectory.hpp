/*
** R-Type ECS - CircularTrajectory
** Orbital, spiral, and circular motion patterns
*/

#pragma once

#include "game/components/bullets/TrajectoryTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Circular/orbital trajectory
     * 
     * Bullet orbits around a center point. Can expand or contract.
     */
    struct CircularTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Circular;

        float centerX = 0.0f, centerY = 0.0f;  // Orbit center
        float radius = 100.0f;                  // Orbital radius
        float angularVelocity = 3.0f;          // Radians per second
        float currentAngle = 0.0f;              // Current angle (runtime state)
        float radiusChangeRate = 0.0f;          // Radius change per second

        CircularTrajectory() = default;

        CircularTrajectory(float cx, float cy, float r, float angVel = 3.0f)
            : centerX(cx), centerY(cy), radius(r), angularVelocity(angVel) {}

        CircularTrajectory& expanding(float rate) {
            radiusChangeRate = rate;
            return *this;
        }

        CircularTrajectory& contracting(float rate) {
            radiusChangeRate = -rate;
            return *this;
        }
    };

    /**
     * @brief Spiral outward trajectory
     * 
     * Bullet spirals outward from spawn point.
     */
    struct SpiralTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Spiral;

        float expansionRate = 50.0f;   // How fast the spiral expands
        float tightness = 2.0f;        // Rotations per expansion unit

        SpiralTrajectory() = default;

        SpiralTrajectory(float expansion, float tight = 2.0f)
            : expansionRate(expansion), tightness(tight) {}

        static SpiralTrajectory tight() {
            return SpiralTrajectory(30.0f, 4.0f);
        }

        static SpiralTrajectory loose() {
            return SpiralTrajectory(80.0f, 1.0f);
        }
    };

    /**
     * @brief Spiral inward trajectory
     * 
     * Bullet spirals inward toward a target point.
     */
    struct SpiralInwardTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::SpiralInward;

        float targetX = 0.0f, targetY = 0.0f;  // Target point
        float inwardRate = 30.0f;               // Rate of inward movement

        SpiralInwardTrajectory() = default;

        SpiralInwardTrajectory(float tx, float ty, float rate = 30.0f)
            : targetX(tx), targetY(ty), inwardRate(rate) {}
    };

} // namespace rtype::ecs
