/*
** R-Type ECS - TrajectoryComponent
** Component for complex bullet trajectory behaviors
*/

#pragma once

#include "../../../engine/ecs/core/IComponent.hpp"
#include "../../../engine/ecs/core/Types.hpp"
#include "TrajectoryTypes.hpp"
#include "TrajectoryParams.hpp"

namespace rtype::ecs {

    /**
     * @brief Component for complex bullet trajectory behaviors
     *
     * Attach to projectiles that need non-linear movement patterns.
     * Works alongside VelocityComponent - trajectory modifies velocity each frame.
     */
    struct TrajectoryComponent : public IComponent {
        // Core trajectory settings
        TrajectoryType type = TrajectoryType::Linear;
        float elapsedTime = 0.0f;          // Time since trajectory started
        float delay = 0.0f;                 // Delay before trajectory activates

        // Trajectory-specific parameters (use based on type)
        // Homing
        EntityId targetId = NULL_ENTITY;
        float homingStrength = 5.0f;
        float homingDuration = 0.0f;
        bool predictTarget = false;

        // Sinusoidal
        float waveAmplitude = 200.0f;
        float waveFrequency = 15.0f;
        float wavePhase = 0.0f;
        bool wavePerpendicular = true;

        // Bezier
        float startX = 0.0f, startY = 0.0f;
        float control1X = 0.0f, control1Y = 0.0f;
        float control2X = 0.0f, control2Y = 0.0f;
        float endX = 0.0f, endY = 0.0f;
        float bezierDuration = 2.0f;

        // Circular/orbital
        float orbitCenterX = 0.0f, orbitCenterY = 0.0f;
        float orbitRadius = 100.0f;
        float angularVelocity = 3.0f;
        float currentAngle = 0.0f;
        float radiusChangeRate = 0.0f;

        // Acceleration
        float targetSpeed = 500.0f;
        float acceleration = 200.0f;
        float speedChangeDelay = 0.5f;

        // Aimed
        float aimTime = 0.0f;
        bool hasAimed = false;

        // Boomerang
        float boomerangDistance = 300.0f;
        float boomerangPhase = 0.0f;
        float originX = 0.0f, originY = 0.0f;

        // Spiral
        float spiralExpansionRate = 50.0f;
        float spiralTightness = 2.0f;

        // Random
        float randomInterval = 0.5f;
        float randomTimer = 0.0f;
        float randomAngleRange = 45.0f;
        
        // Zigzag
        float zigzagWidth = 100.0f;       // Width of each zig
        float zigzagLength = 50.0f;       // Length before changing direction
        float zigzagProgress = 0.0f;      // Progress along current segment
        int zigzagDirection = 1;          // 1 or -1
        
        // Figure8
        float figure8Width = 150.0f;      // Width of the figure 8
        float figure8Height = 100.0f;     // Height of the figure 8
        float figure8Speed = 2.0f;        // Speed of traversal
        
        // DelayedHoming
        float homingDelay = 1.0f;         // Time before homing activates
        bool homingStarted = false;
        
        // Pendulum
        float pendulumAmplitude = 60.0f;  // Max angle in degrees
        float pendulumFrequency = 2.0f;   // Swings per second
        float pendulumDamping = 0.0f;     // Damping factor (0 = no damping)
        
        // SpiralInward
        float spiralTargetX = 0.0f;       // Target point X
        float spiralTargetY = 0.0f;       // Target point Y
        float spiralInwardRate = 30.0f;   // Rate of inward movement
        
        // Whip
        float whipAccelPhase = 0.3f;      // Duration of acceleration phase (0-1)
        float whipMaxSpeed = 800.0f;      // Maximum speed at peak
        float whipMinSpeed = 100.0f;      // Speed at end
        
        // Wobble
        float wobbleIntensity = 20.0f;    // Max wobble offset
        float wobbleSpeed = 10.0f;        // Wobble frequency
        
        // Modifiers
        TrajectoryModifier modifiers = TrajectoryModifier::None;

        // State tracking
        float baseVelX = 0.0f, baseVelY = 0.0f;
        bool initialized = false;

        TrajectoryComponent() = default;
        explicit TrajectoryComponent(TrajectoryType t) : type(t) {}

        std::string getTypeName() const override {
            return "TrajectoryComponent";
        }
    };

} // namespace rtype::ecs
