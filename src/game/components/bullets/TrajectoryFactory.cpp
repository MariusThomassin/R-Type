/*
** R-Type ECS - Trajectory Factory
** Implementation of factory methods for creating common trajectory types
*/

#include "TrajectoryFactory.hpp"

namespace rtype::ecs {

    TrajectoryComponent TrajectoryFactory::createHoming(EntityId target, float strength, float duration) {
        TrajectoryComponent t(TrajectoryType::Homing);
        t.targetId = target;
        t.homingStrength = strength;
        t.homingDuration = duration;
        return t;
    }

    TrajectoryComponent TrajectoryFactory::createSinusoidal(float amplitude, float frequency, float phase) {
        TrajectoryComponent t(TrajectoryType::Sinusoidal);
        t.waveAmplitude = amplitude;
        t.waveFrequency = frequency;
        t.wavePhase = phase;
        return t;
    }

    TrajectoryComponent TrajectoryFactory::createBezier(float sx, float sy,
                                                        float c1x, float c1y,
                                                        float c2x, float c2y,
                                                        float ex, float ey,
                                                        float duration) {
        TrajectoryComponent t(TrajectoryType::Bezier);
        t.startX = sx; t.startY = sy;
        t.control1X = c1x; t.control1Y = c1y;
        t.control2X = c2x; t.control2Y = c2y;
        t.endX = ex; t.endY = ey;
        t.bezierDuration = duration;
        return t;
    }

    TrajectoryComponent TrajectoryFactory::createCircular(float centerX, float centerY, float radius,
                                                          float angVel, float radiusChange) {
        TrajectoryComponent t(TrajectoryType::Circular);
        t.orbitCenterX = centerX;
        t.orbitCenterY = centerY;
        t.orbitRadius = radius;
        t.angularVelocity = angVel;
        t.radiusChangeRate = radiusChange;
        return t;
    }

    TrajectoryComponent TrajectoryFactory::createAccelerating(float targetSpd, float accel, float delay) {
        TrajectoryComponent t(TrajectoryType::Accelerating);
        t.targetSpeed = targetSpd;
        t.acceleration = accel;
        t.speedChangeDelay = delay;
        return t;
    }

    TrajectoryComponent TrajectoryFactory::createAimed(EntityId target) {
        TrajectoryComponent t(TrajectoryType::Aimed);
        t.targetId = target;
        return t;
    }

    TrajectoryComponent TrajectoryFactory::createSpiral(float expansionRate, float tightness) {
        TrajectoryComponent t(TrajectoryType::Spiral);
        t.spiralExpansionRate = expansionRate;
        t.spiralTightness = tightness;
        return t;
    }

    TrajectoryComponent TrajectoryFactory::createBoomerang(float distance) {
        TrajectoryComponent t(TrajectoryType::Boomerang);
        t.boomerangDistance = distance;
        return t;
    }

    TrajectoryComponent TrajectoryFactory::createRandom(float interval, float angleRange) {
        TrajectoryComponent t(TrajectoryType::Random);
        t.randomInterval = interval;
        t.randomAngleRange = angleRange;
        return t;
    }

} // namespace rtype::ecs
