/*
** R-Type ECS - RandomTrajectory
** Unpredictable and chaotic trajectory patterns
*/

#pragma once

#include "../TrajectoryTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Random trajectory - changes direction unpredictably
     * 
     * At regular intervals, the bullet randomly changes direction
     * within a specified angle range.
     */
    struct RandomTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Random;

        float interval = 0.5f;         // Time between direction changes
        float timer = 0.0f;            // Time since last change (runtime)
        float angleRange = 45.0f;      // Max angle change (degrees)

        RandomTrajectory() = default;

        RandomTrajectory(float intvl, float range)
            : interval(intvl), angleRange(range) {}

        static RandomTrajectory chaotic() {
            return RandomTrajectory(0.2f, 90.0f);
        }

        static RandomTrajectory subtle() {
            return RandomTrajectory(0.8f, 20.0f);
        }
    };

    /**
     * @brief Zigzag trajectory - sharp directional changes
     * 
     * Bullet moves in a sharp zigzag pattern.
     */
    struct ZigzagTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Zigzag;

        float width = 100.0f;          // Width of each zig
        float length = 50.0f;          // Distance before changing direction
        float progress = 0.0f;         // Progress along current segment (runtime)
        int direction = 1;             // Current direction: 1 or -1 (runtime)

        ZigzagTrajectory() = default;

        ZigzagTrajectory(float w, float len)
            : width(w), length(len) {}

        static ZigzagTrajectory tight() {
            return ZigzagTrajectory(50.0f, 30.0f);
        }

        static ZigzagTrajectory wide() {
            return ZigzagTrajectory(200.0f, 100.0f);
        }
    };

    /**
     * @brief Aimed trajectory - aims once then goes straight
     * 
     * At spawn (or after a delay), aims directly at target, then
     * continues in a straight line.
     */
    struct AimedTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Aimed;

        float aimTime = 0.0f;          // Time before aiming (0 = immediate)
        bool hasAimed = false;         // Has the aiming occurred? (runtime)

        AimedTrajectory() = default;

        explicit AimedTrajectory(float delay)
            : aimTime(delay) {}
    };

} // namespace rtype::ecs
