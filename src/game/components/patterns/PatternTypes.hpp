/*
** R-Type ECS - Pattern Types
** Enumeration for pattern shapes and aim modes
*/

#pragma once

namespace rtype::ecs {

    /**
     * @brief How bullets are aimed when spawned
     */
    enum class AimMode : int {
        Fixed = 0,       // Fixed angle relative to spawner
        AtPlayer,        // Aimed directly at player position
        AtPlayerLead,    // Aimed with prediction (leads the player)
        Random,          // Random angle within range
        Sequence,        // Sequential angles (rotates over time)

        COUNT
    };

    /**
     * @brief Shape of the bullet spawn pattern
     */
    enum class PatternShape : int {
        Single = 0,      // Single bullet
        Line,            // Line of bullets
        Fan,             // Fan/spread of bullets
        Circle,          // Full 360° circle
        Arc,             // Partial arc
        Ring,            // Ring that expands outward
        Spiral,          // Spiral arms
        Cross,           // + shaped
        Star,            // Star shaped
        Grid,            // Grid of bullets
        Wave,            // Wave formation
        Stream,          // Continuous stream

        COUNT
    };

    /**
     * @brief State of a pattern spawner
     */
    enum class SpawnerState : int {
        Idle = 0,        // Not spawning
        Active,          // Currently spawning
        Paused,          // Paused mid-pattern
        Cooldown,        // Between pattern repeats
        Finished,        // Pattern completed (non-looping)

        COUNT
    };

} // namespace rtype::ecs
