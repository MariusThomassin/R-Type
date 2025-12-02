/*
** R-Type ECS - Game Math Utilities
** Common math constants and functions for game systems
*/

#pragma once

#include <cmath>

namespace rtype::ecs {

    /**
     * @brief Math utilities for trajectory, pattern, and general calculations
     * Used by bullets, effects, enemy AI, and any movement systems
     * Note: Uses M_PI_F to avoid raylib PI macro conflict
     */
    struct GameMath {
        static constexpr float M_PI_F = 3.14159265358979f;
        static constexpr float DEG_TO_RAD = M_PI_F / 180.0f;
        static constexpr float RAD_TO_DEG = 180.0f / M_PI_F;

        static float normalizeAngle(float angle);
        static float normalizeAngleDeg(float angleDeg);
        static float distance(float x1, float y1, float x2, float y2);
        static float speed(float vx, float vy);
        static float velocityAngle(float vx, float vy);
        static void setVelocityAngle(float& vx, float& vy, float angleRad);
        static void setVelocitySpeed(float& vx, float& vy, float newSpeed);
        static void clampSpeed(float& vx, float& vy, float maxSpeed);
        static void perpendicular(float vx, float vy, float& perpX, float& perpY);
        static void bezierPoint(float t, float p0x, float p0y, float p1x, float p1y,
                                float p2x, float p2y, float p3x, float p3y,
                                float& outX, float& outY);
    };

    // Backward compatibility alias
    using BulletMath = GameMath;

} // namespace rtype::ecs
