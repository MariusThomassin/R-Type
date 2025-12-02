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

        static float normalizeAngle(float angle) {
            while (angle > M_PI_F) angle -= 2.0f * M_PI_F;
            while (angle < -M_PI_F) angle += 2.0f * M_PI_F;
            return angle;
        }

        static float normalizeAngleDeg(float angleDeg) {
            while (angleDeg >= 360.0f) angleDeg -= 360.0f;
            while (angleDeg < 0.0f) angleDeg += 360.0f;
            return angleDeg;
        }

        static float distance(float x1, float y1, float x2, float y2) {
            float dx = x2 - x1;
            float dy = y2 - y1;
            return std::sqrt(dx * dx + dy * dy);
        }

        static float speed(float vx, float vy) {
            return std::sqrt(vx * vx + vy * vy);
        }

        static float velocityAngle(float vx, float vy) {
            return std::atan2(vy, vx);
        }

        static void setVelocityAngle(float& vx, float& vy, float angleRad) {
            float spd = speed(vx, vy);
            vx = std::cos(angleRad) * spd;
            vy = std::sin(angleRad) * spd;
        }

        static void setVelocitySpeed(float& vx, float& vy, float newSpeed) {
            float currentSpeed = speed(vx, vy);
            if (currentSpeed > 0.001f) {
                float scale = newSpeed / currentSpeed;
                vx *= scale;
                vy *= scale;
            }
        }

        static void clampSpeed(float& vx, float& vy, float maxSpeed) {
            float spd = speed(vx, vy);
            if (spd > maxSpeed && spd > 0.0f) {
                float scale = maxSpeed / spd;
                vx *= scale;
                vy *= scale;
            }
        }

        static void perpendicular(float vx, float vy, float& perpX, float& perpY) {
            float spd = speed(vx, vy);
            if (spd > 0.0f) {
                perpX = -vy / spd;
                perpY = vx / spd;
            } else {
                perpX = 0.0f;
                perpY = 1.0f;
            }
        }

        static void bezierPoint(float t, float p0x, float p0y, float p1x, float p1y,
                                float p2x, float p2y, float p3x, float p3y,
                                float& outX, float& outY) {
            float u = 1.0f - t;
            float u2 = u * u;
            float u3 = u2 * u;
            float t2 = t * t;
            float t3 = t2 * t;

            outX = u3 * p0x + 3.0f * u2 * t * p1x + 3.0f * u * t2 * p2x + t3 * p3x;
            outY = u3 * p0y + 3.0f * u2 * t * p1y + 3.0f * u * t2 * p2y + t3 * p3y;
        }
    };

    // Backward compatibility alias
    using BulletMath = GameMath;

} // namespace rtype::ecs
