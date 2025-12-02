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
        static TrajectoryComponent createHoming(EntityId target, float strength = 5.0f, float duration = 0.0f);

        /**
         * @brief Create a sinusoidal wave trajectory
         */
        static TrajectoryComponent createSinusoidal(float amplitude, float frequency, float phase = 0.0f);

        /**
         * @brief Create a bezier curve trajectory
         */
        static TrajectoryComponent createBezier(float sx, float sy,
                                                float c1x, float c1y,
                                                float c2x, float c2y,
                                                float ex, float ey,
                                                float duration = 2.0f);

        /**
         * @brief Create a circular orbit trajectory
         */
        static TrajectoryComponent createCircular(float centerX, float centerY, float radius,
                                                  float angVel, float radiusChange = 0.0f);

        /**
         * @brief Create an accelerating/decelerating trajectory
         */
        static TrajectoryComponent createAccelerating(float targetSpd, float accel, float delay = 0.0f);

        /**
         * @brief Create an aimed trajectory (aims at target once, then goes straight)
         */
        static TrajectoryComponent createAimed(EntityId target);

        /**
         * @brief Create a spiral trajectory
         */
        static TrajectoryComponent createSpiral(float expansionRate, float tightness);

        /**
         * @brief Create a boomerang trajectory
         */
        static TrajectoryComponent createBoomerang(float distance);

        /**
         * @brief Create a random movement trajectory
         */
        static TrajectoryComponent createRandom(float interval = 0.5f, float angleRange = 45.0f);
    };

} // namespace rtype::ecs
