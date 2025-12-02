/*
** R-Type ECS - WaveTrajectory
** Sinusoidal and oscillating trajectory patterns
*/

#pragma once

#include "../TrajectoryTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Sinusoidal trajectory - moves in a wave pattern
     * 
     * Creates smooth wave motion perpendicular to travel direction.
     * Great for snake-like or undulating bullet patterns.
     */
    struct SinusoidalTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Sinusoidal;

        float amplitude = 200.0f;     // Wave height (pixels)
        float frequency = 15.0f;      // Oscillations per second
        float phase = 0.0f;           // Starting phase offset (radians)
        bool perpendicular = true;    // Oscillate perpendicular to direction?

        SinusoidalTrajectory() = default;

        SinusoidalTrajectory(float amp, float freq, float ph = 0.0f)
            : amplitude(amp), frequency(freq), phase(ph) {}

        static SinusoidalTrajectory gentle() {
            return SinusoidalTrajectory(50.0f, 3.0f);
        }

        static SinusoidalTrajectory aggressive() {
            return SinusoidalTrajectory(150.0f, 8.0f);
        }

        static SinusoidalTrajectory snake() {
            return SinusoidalTrajectory(80.0f, 5.0f);
        }
    };

    /**
     * @brief Wobble trajectory - random small oscillations
     * 
     * Adds unpredictability to bullet movement without major direction changes.
     */
    struct WobbleTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Wobble;

        float intensity = 20.0f;      // Maximum wobble offset
        float speed = 10.0f;          // Wobble frequency

        WobbleTrajectory() = default;

        WobbleTrajectory(float intens, float spd)
            : intensity(intens), speed(spd) {}
    };

    /**
     * @brief Pendulum trajectory - swinging motion
     * 
     * Bullet swings back and forth like a pendulum while traveling.
     */
    struct PendulumTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Pendulum;

        float amplitude = 60.0f;      // Max swing angle (degrees)
        float frequency = 2.0f;       // Swings per second
        float damping = 0.0f;         // Damping factor (0 = no damping)

        PendulumTrajectory() = default;

        PendulumTrajectory(float amp, float freq, float damp = 0.0f)
            : amplitude(amp), frequency(freq), damping(damp) {}
    };

} // namespace rtype::ecs
