/*
** R-Type ECS - SpeedTrajectory
** Acceleration, deceleration, and speed-changing trajectories
*/

#pragma once

#include "../TrajectoryTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Accelerating trajectory
     * 
     * Bullet accelerates or decelerates to a target speed.
     */
    struct AcceleratingTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Accelerating;

        float targetSpeed = 500.0f;    // Speed to reach
        float acceleration = 200.0f;   // Speed change per second
        float delay = 0.0f;            // Delay before acceleration starts

        AcceleratingTrajectory() = default;

        AcceleratingTrajectory(float target, float accel, float del = 0.0f)
            : targetSpeed(target), acceleration(accel), delay(del) {}

        static AcceleratingTrajectory speedUp(float target, float time) {
            return AcceleratingTrajectory(target, target / time);
        }

        static AcceleratingTrajectory slowDown(float startSpeed, float time) {
            return AcceleratingTrajectory(0.0f, startSpeed / time);
        }
    };

    /**
     * @brief Whip trajectory - accelerates then decelerates
     * 
     * Creates a whip-crack effect: slow start, fast middle, slow end.
     */
    struct WhipTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Whip;

        float accelPhase = 0.3f;       // Portion of time spent accelerating (0-1)
        float maxSpeed = 800.0f;       // Peak speed at transition
        float minSpeed = 100.0f;       // Speed at start and end

        WhipTrajectory() = default;

        WhipTrajectory(float accelPortion, float peak, float minSpd = 100.0f)
            : accelPhase(accelPortion), maxSpeed(peak), minSpeed(minSpd) {}
    };

    /**
     * @brief Boomerang trajectory - returns to origin
     * 
     * Bullet travels forward, then reverses and returns.
     */
    struct BoomerangTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Boomerang;

        float distance = 300.0f;       // Max distance before returning
        float phase = 0.0f;            // Current phase (runtime state)
        float originX = 0.0f;          // Starting position X
        float originY = 0.0f;          // Starting position Y

        BoomerangTrajectory() = default;

        explicit BoomerangTrajectory(float dist)
            : distance(dist) {}
    };

} // namespace rtype::ecs
