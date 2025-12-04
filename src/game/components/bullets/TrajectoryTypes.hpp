/*
** R-Type ECS - Trajectory Types
** Enumeration of trajectory behaviors
*/

#pragma once

namespace rtype::ecs {

    /**
     * @brief Types of bullet trajectories
     */
    enum class TrajectoryType : int {
        Linear = 0,      // Straight line (default, uses VelocityComponent directly)
        Homing,          // Tracks toward a target entity
        Sinusoidal,      // Sine wave pattern
        Bezier,          // Bezier curve interpolation
        Circular,        // Circular/orbital motion
        Accelerating,    // Speed changes over time
        Aimed,           // Initially aimed at target, then linear
        Boomerang,       // Returns to origin
        Spiral,          // Spiraling outward/inward
        Random,          // Random direction changes
        
        // New trajectory types
        Zigzag,          // Sharp zigzag pattern (angular, not smooth)
        Figure8,         // Infinity/figure-8 pattern
        DelayedHoming,   // Delays before starting to home
        Pendulum,        // Swinging pendulum motion
        SpiralInward,    // Spirals inward to a point
        Whip,            // Accelerates then decelerates (whip crack)
        Wobble,          // Random small oscillations

        COUNT
    };
    
    /**
     * @brief Modifier flags for trajectory behavior
     * Can be combined with bitwise OR
     */
    enum class TrajectoryModifier : unsigned int {
        None          = 0,
        EaseIn        = 1 << 0,   // Apply ease-in to trajectory
        EaseOut       = 1 << 1,   // Apply ease-out to trajectory
        EaseInOut     = 1 << 2,   // Apply ease-in-out to trajectory
        Bounce        = 1 << 3,   // Bounce at boundaries
        Loop          = 1 << 4,   // Loop trajectory when complete
        Reverse       = 1 << 5,   // Reverse direction when complete
        FaceDirection = 1 << 6,   // Rotate sprite to face movement direction
    };
    
    inline TrajectoryModifier operator|(TrajectoryModifier a, TrajectoryModifier b) {
        return static_cast<TrajectoryModifier>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
    }
    
    inline bool hasModifier(TrajectoryModifier flags, TrajectoryModifier check) {
        return (static_cast<unsigned int>(flags) & static_cast<unsigned int>(check)) != 0;
    }

} // namespace rtype::ecs
