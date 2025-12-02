/*
** R-Type ECS - TrajectoryBase
** Base structures and common types for trajectories
*/

#pragma once

#include "../../../../engine/ecs/core/Types.hpp"
#include "../TrajectoryTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Common state shared by all trajectory types
     */
    struct TrajectoryState {
        float elapsedTime = 0.0f;      // Time since trajectory started
        float baseVelX = 0.0f;         // Initial velocity X
        float baseVelY = 0.0f;         // Initial velocity Y
        bool initialized = false;       // Whether trajectory has been initialized
    };

    /**
     * @brief Linear trajectory (simplest case)
     * 
     * Bullets travel in a straight line using only VelocityComponent.
     * No additional data needed.
     */
    struct LinearTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Linear;
        // No extra data - uses base velocity directly
    };

} // namespace rtype::ecs
