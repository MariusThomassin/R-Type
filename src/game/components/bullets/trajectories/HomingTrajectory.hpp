/*
** R-Type ECS - HomingTrajectory
** Trajectory data for homing and delayed-homing bullets
*/

#pragma once

#include "../../../../engine/ecs/core/Types.hpp"
#include "../TrajectoryTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Homing trajectory - tracks toward a target entity
     * 
     * The bullet gradually turns toward the target based on homingStrength.
     * Can optionally predict target movement for smarter tracking.
     */
    struct HomingTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::Homing;

        EntityId targetId = NULL_ENTITY;  // Entity to track
        float strength = 5.0f;             // Turn rate (higher = sharper turns)
        float duration = 0.0f;             // 0 = infinite homing
        bool predictTarget = false;        // Predict target movement?

        HomingTrajectory() = default;
        
        HomingTrajectory(EntityId target, float str = 5.0f)
            : targetId(target), strength(str) {}

        HomingTrajectory& withDuration(float dur) {
            duration = dur;
            return *this;
        }

        HomingTrajectory& withPrediction(bool predict = true) {
            predictTarget = predict;
            return *this;
        }
    };

    /**
     * @brief Delayed homing - travels straight, then starts homing
     * 
     * Useful for missiles that need to clear the shooter before tracking.
     */
    struct DelayedHomingTrajectory {
        static constexpr TrajectoryType TYPE = TrajectoryType::DelayedHoming;

        EntityId targetId = NULL_ENTITY;
        float strength = 5.0f;
        float delay = 1.0f;               // Seconds before homing activates
        bool started = false;              // Runtime state: has homing begun?

        DelayedHomingTrajectory() = default;

        DelayedHomingTrajectory(EntityId target, float delayTime, float str = 5.0f)
            : targetId(target), strength(str), delay(delayTime) {}
    };

} // namespace rtype::ecs
