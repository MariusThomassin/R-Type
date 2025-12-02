/*
** R-Type ECS - Trajectory Components (Decomposed)
** Separate small components for each trajectory type
** More memory-efficient and cache-friendly than one bloated component
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include "engine/ecs/core/Types.hpp"
#include "TrajectoryTypes.hpp"

namespace rtype::ecs {

    // ==========================================================================
    // Base Trajectory Component - Required by all trajectory entities
    // ==========================================================================

    /**
     * @brief Base trajectory marker and common timing data
     * Attach this to any entity that has trajectory behavior
     */
    struct TrajectoryBase : public IComponent {
        TrajectoryType type = TrajectoryType::Linear;
        float elapsedTime = 0.0f;
        float delay = 0.0f;           // Delay before trajectory activates
        bool initialized = false;
        
        // Base velocity (stored for trajectory calculations)
        float baseVelX = 0.0f;
        float baseVelY = 0.0f;
        
        // Modifiers
        TrajectoryModifier modifiers = TrajectoryModifier::None;

        TrajectoryBase() = default;
        explicit TrajectoryBase(TrajectoryType t) : type(t) {}

        std::string getTypeName() const override { return "TrajectoryBase"; }
    };

    // ==========================================================================
    // Specific Trajectory Components - Add alongside TrajectoryBase as needed
    // ==========================================================================

    /**
     * @brief Homing trajectory - tracks toward a target
     */
    struct HomingTrajectory : public IComponent {
        EntityId targetId = NULL_ENTITY;
        float strength = 5.0f;         // Turn rate
        float duration = 0.0f;          // 0 = infinite
        bool predictTarget = false;     // Predict target position

        std::string getTypeName() const override { return "HomingTrajectory"; }
    };

    /**
     * @brief Delayed homing - starts homing after a delay
     */
    struct DelayedHomingTrajectory : public IComponent {
        EntityId targetId = NULL_ENTITY;
        float homingDelay = 1.0f;      // Time before homing activates
        float strength = 5.0f;
        bool homingStarted = false;

        std::string getTypeName() const override { return "DelayedHomingTrajectory"; }
    };

    /**
     * @brief Sinusoidal wave motion
     */
    struct SinusoidalTrajectory : public IComponent {
        float amplitude = 200.0f;       // Wave height
        float frequency = 15.0f;        // Wave speed
        float phase = 0.0f;             // Starting phase offset
        bool perpendicular = true;      // Wave perpendicular to velocity

        std::string getTypeName() const override { return "SinusoidalTrajectory"; }
    };

    /**
     * @brief Bezier curve interpolation
     */
    struct BezierTrajectory : public IComponent {
        float startX = 0.0f, startY = 0.0f;
        float control1X = 0.0f, control1Y = 0.0f;
        float control2X = 0.0f, control2Y = 0.0f;
        float endX = 0.0f, endY = 0.0f;
        float duration = 2.0f;          // Time to complete curve

        std::string getTypeName() const override { return "BezierTrajectory"; }
    };

    /**
     * @brief Circular/orbital motion
     */
    struct CircularTrajectory : public IComponent {
        float centerX = 0.0f, centerY = 0.0f;
        float radius = 100.0f;
        float angularVelocity = 3.0f;   // Radians per second
        float currentAngle = 0.0f;
        float radiusChangeRate = 0.0f;  // For spiral motion

        std::string getTypeName() const override { return "CircularTrajectory"; }
    };

    /**
     * @brief Acceleration over time
     */
    struct AcceleratingTrajectory : public IComponent {
        float targetSpeed = 500.0f;
        float acceleration = 200.0f;    // Units per second²
        float delayBeforeStart = 0.5f;

        std::string getTypeName() const override { return "AcceleratingTrajectory"; }
    };

    /**
     * @brief Initially aimed at target, then linear
     */
    struct AimedTrajectory : public IComponent {
        EntityId targetId = NULL_ENTITY;
        float aimDuration = 0.0f;       // Time to complete aiming
        bool hasAimed = false;

        std::string getTypeName() const override { return "AimedTrajectory"; }
    };

    /**
     * @brief Boomerang - returns to origin
     */
    struct BoomerangTrajectory : public IComponent {
        float originX = 0.0f, originY = 0.0f;
        float maxDistance = 300.0f;
        float phase = 0.0f;             // Current phase (0-2π for out-and-back)

        std::string getTypeName() const override { return "BoomerangTrajectory"; }
    };

    /**
     * @brief Spiral motion (outward or inward)
     */
    struct SpiralTrajectory : public IComponent {
        float expansionRate = 50.0f;    // Radius change per second
        float tightness = 2.0f;         // Angular velocity
        float targetX = 0.0f, targetY = 0.0f;  // For inward spiral
        bool inward = false;

        std::string getTypeName() const override { return "SpiralTrajectory"; }
    };

    /**
     * @brief Random direction changes
     */
    struct RandomTrajectory : public IComponent {
        float changeInterval = 0.5f;    // Seconds between direction changes
        float timer = 0.0f;
        float angleRange = 45.0f;       // Max angle change in degrees

        std::string getTypeName() const override { return "RandomTrajectory"; }
    };

    /**
     * @brief Zigzag pattern
     */
    struct ZigzagTrajectory : public IComponent {
        float width = 100.0f;           // Width of each zig
        float segmentLength = 50.0f;    // Length before changing direction
        float progress = 0.0f;
        int direction = 1;              // 1 or -1

        std::string getTypeName() const override { return "ZigzagTrajectory"; }
    };

    /**
     * @brief Figure-8/infinity pattern
     */
    struct Figure8Trajectory : public IComponent {
        float width = 150.0f;
        float height = 100.0f;
        float speed = 2.0f;             // Traversal speed

        std::string getTypeName() const override { return "Figure8Trajectory"; }
    };

    /**
     * @brief Pendulum swinging motion
     */
    struct PendulumTrajectory : public IComponent {
        float amplitude = 60.0f;        // Max angle in degrees
        float frequency = 2.0f;         // Swings per second
        float damping = 0.0f;           // Damping factor

        std::string getTypeName() const override { return "PendulumTrajectory"; }
    };

    /**
     * @brief Whip - accelerates then decelerates
     */
    struct WhipTrajectory : public IComponent {
        float accelPhase = 0.3f;        // Duration of acceleration (0-1)
        float maxSpeed = 800.0f;
        float minSpeed = 100.0f;

        std::string getTypeName() const override { return "WhipTrajectory"; }
    };

    /**
     * @brief Small random oscillations
     */
    struct WobbleTrajectory : public IComponent {
        float intensity = 20.0f;        // Max wobble offset
        float speed = 10.0f;            // Wobble frequency

        std::string getTypeName() const override { return "WobbleTrajectory"; }
    };

} // namespace rtype::ecs
