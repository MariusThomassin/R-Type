/*
** R-Type ECS - Trajectory Parameters
** Data structures for trajectory configuration
*/

#pragma once

#include "engine/ecs/core/Types.hpp"

namespace rtype::ecs {

    /**
     * @brief Parameters for homing trajectory
     */
    struct HomingParams {
        EntityId targetId = NULL_ENTITY;    // Target entity to home toward
        float strength = 5.0f;              // Turn rate (radians/sec)
        float duration = 0.0f;              // 0 = infinite homing
        bool predictTarget = false;         // Lead target based on velocity
    };

    /**
     * @brief Parameters for sinusoidal wave trajectory
     */
    struct SinusoidalParams {
        float amplitude = 50.0f;            // Pixels of wave displacement
        float frequency = 3.0f;             // Waves per second
        float phase = 0.0f;                 // Starting phase offset (radians)
        bool perpendicular = true;          // Wave perpendicular to travel direction
    };

    /**
     * @brief Parameters for bezier curve trajectory
     */
    struct BezierParams {
        float startX = 0.0f, startY = 0.0f;           // P0: Start point
        float control1X = 0.0f, control1Y = 0.0f;     // P1: First control point
        float control2X = 0.0f, control2Y = 0.0f;     // P2: Second control point
        float endX = 0.0f, endY = 0.0f;               // P3: End point
        float duration = 2.0f;                         // Time to complete curve
    };

    /**
     * @brief Parameters for circular/orbital trajectory
     */
    struct CircularParams {
        float centerX = 0.0f, centerY = 0.0f;  // Center of orbit
        float radius = 100.0f;                  // Current radius
        float angularVelocity = 3.0f;           // Radians per second
        float currentAngle = 0.0f;              // Current angle in orbit
        float radiusChangeRate = 0.0f;          // Spiral: + = expand, - = contract
    };

    /**
     * @brief Parameters for accelerating trajectory
     */
    struct AcceleratingParams {
        float targetSpeed = 500.0f;         // Speed to accelerate/decelerate toward
        float acceleration = 200.0f;        // Speed change per second
        float delay = 0.5f;                 // Delay before speed change starts
    };

    /**
     * @brief Parameters for boomerang trajectory
     */
    struct BoomerangParams {
        float distance = 300.0f;            // Distance before returning
        float phase = 0.0f;                 // 0 = going out, 1 = returning
        float originX = 0.0f, originY = 0.0f;
    };

    /**
     * @brief Parameters for spiral trajectory
     */
    struct SpiralParams {
        float expansionRate = 50.0f;        // Radius increase per second
        float tightness = 2.0f;             // Rotations per second
        float originX = 0.0f, originY = 0.0f;
    };

    /**
     * @brief Parameters for random trajectory
     */
    struct RandomParams {
        float interval = 0.5f;              // Seconds between direction changes
        float timer = 0.0f;                 // Timer for next change
        float angleRange = 45.0f;           // Max angle change in degrees
    };

} // namespace rtype::ecs
