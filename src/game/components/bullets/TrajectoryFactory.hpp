/*
** R-Type ECS - Trajectory Factory
** Factory methods for creating common trajectory types
*/

#pragma once

#include "TrajectoryComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Factory for creating TrajectoryComponent instances
     */
    struct TrajectoryFactory {
        /**
         * @brief Create a homing trajectory
         */
        static TrajectoryComponent createHoming(EntityId target, float strength = 5.0f, float duration = 0.0f) {
            TrajectoryComponent t(TrajectoryType::Homing);
            t.targetId = target;
            t.homingStrength = strength;
            t.homingDuration = duration;
            return t;
        }

        /**
         * @brief Create a sinusoidal wave trajectory
         */
        static TrajectoryComponent createSinusoidal(float amplitude, float frequency, float phase = 0.0f) {
            TrajectoryComponent t(TrajectoryType::Sinusoidal);
            t.waveAmplitude = amplitude;
            t.waveFrequency = frequency;
            t.wavePhase = phase;
            return t;
        }

        /**
         * @brief Create a bezier curve trajectory
         */
        static TrajectoryComponent createBezier(float sx, float sy,
                                                  float c1x, float c1y,
                                                  float c2x, float c2y,
                                                  float ex, float ey,
                                                  float duration = 2.0f) {
            TrajectoryComponent t(TrajectoryType::Bezier);
            t.startX = sx; t.startY = sy;
            t.control1X = c1x; t.control1Y = c1y;
            t.control2X = c2x; t.control2Y = c2y;
            t.endX = ex; t.endY = ey;
            t.bezierDuration = duration;
            return t;
        }

        /**
         * @brief Create a circular orbit trajectory
         */
        static TrajectoryComponent createCircular(float centerX, float centerY, float radius,
                                                    float angVel, float radiusChange = 0.0f) {
            TrajectoryComponent t(TrajectoryType::Circular);
            t.orbitCenterX = centerX;
            t.orbitCenterY = centerY;
            t.orbitRadius = radius;
            t.angularVelocity = angVel;
            t.radiusChangeRate = radiusChange;
            return t;
        }

        /**
         * @brief Create an accelerating/decelerating trajectory
         */
        static TrajectoryComponent createAccelerating(float targetSpd, float accel, float delay = 0.0f) {
            TrajectoryComponent t(TrajectoryType::Accelerating);
            t.targetSpeed = targetSpd;
            t.acceleration = accel;
            t.speedChangeDelay = delay;
            return t;
        }

        /**
         * @brief Create an aimed trajectory (aims at target once, then goes straight)
         */
        static TrajectoryComponent createAimed(EntityId target) {
            TrajectoryComponent t(TrajectoryType::Aimed);
            t.targetId = target;
            return t;
        }

        /**
         * @brief Create a spiral trajectory
         */
        static TrajectoryComponent createSpiral(float expansionRate, float tightness) {
            TrajectoryComponent t(TrajectoryType::Spiral);
            t.spiralExpansionRate = expansionRate;
            t.spiralTightness = tightness;
            return t;
        }

        /**
         * @brief Create a boomerang trajectory
         */
        static TrajectoryComponent createBoomerang(float distance) {
            TrajectoryComponent t(TrajectoryType::Boomerang);
            t.boomerangDistance = distance;
            return t;
        }

        /**
         * @brief Create a random movement trajectory
         */
        static TrajectoryComponent createRandom(float interval = 0.5f, float angleRange = 45.0f) {
            TrajectoryComponent t(TrajectoryType::Random);
            t.randomInterval = interval;
            t.randomAngleRange = angleRange;
            return t;
        }
    };

} // namespace rtype::ecs
