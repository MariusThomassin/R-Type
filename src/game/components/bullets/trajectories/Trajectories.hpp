/*
** R-Type ECS - Trajectories
** Aggregate header for all trajectory types
*/

#pragma once

// Base types
#include "TrajectoryBase.hpp"

// Homing behaviors
#include "HomingTrajectory.hpp"

// Wave/oscillation patterns
#include "WaveTrajectory.hpp"

// Curves and complex paths
#include "CurveTrajectory.hpp"

// Circular and spiral patterns
#include "CircularTrajectory.hpp"

// Speed modifications
#include "SpeedTrajectory.hpp"

// Random and unpredictable
#include "RandomTrajectory.hpp"

#include <variant>

namespace rtype::ecs {

    /**
     * @brief Variant holding any trajectory type
     * 
     * Much more memory-efficient than the old TrajectoryComponent that
     * stored ALL trajectory fields regardless of type.
     * 
     * Old size: ~400 bytes (all fields)
     * New size: ~80 bytes max (only active type's data)
     */
    using TrajectoryVariant = std::variant<
        LinearTrajectory,
        HomingTrajectory,
        DelayedHomingTrajectory,
        SinusoidalTrajectory,
        WobbleTrajectory,
        PendulumTrajectory,
        BezierTrajectory,
        Figure8Trajectory,
        CircularTrajectory,
        SpiralTrajectory,
        SpiralInwardTrajectory,
        AcceleratingTrajectory,
        WhipTrajectory,
        BoomerangTrajectory,
        RandomTrajectory,
        ZigzagTrajectory,
        AimedTrajectory
    >;

    /**
     * @brief Get the TrajectoryType enum from a variant
     */
    inline TrajectoryType getTrajectoryType(const TrajectoryVariant& var) {
        return std::visit([](const auto& traj) {
            using T = std::decay_t<decltype(traj)>;
            return T::TYPE;
        }, var);
    }

} // namespace rtype::ecs
