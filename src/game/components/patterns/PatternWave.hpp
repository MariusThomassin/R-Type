/*
** R-Type ECS - PatternWave
** Single wave/phase in a pattern sequence
*/

#pragma once

#include "PatternTypes.hpp"
#include "../bullets/TrajectoryTypes.hpp"
#include "../BulletTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Single wave/phase in a pattern sequence
     *
     * Defines how bullets are spawned in a single wave of a pattern.
     */
    struct PatternWave {
        PatternShape shape = PatternShape::Circle;
        int bulletCount = 8;                    // Bullets per wave
        float angleOffset = 0.0f;               // Base angle offset (degrees)
        float angleSpread = 360.0f;             // Total spread (degrees)
        float speed = 200.0f;                   // Bullet speed
        float speedVariation = 0.0f;            // Random speed variation (+/-)
        float spawnDelay = 0.0f;                // Delay before this wave starts
        float burstDelay = 0.0f;                // Delay between bullets in burst
        int burstCount = 1;                     // Bullets per burst
        BulletType bulletType = BulletType::Ball;
        BulletColor bulletColor = BulletColor::Red;
        AimMode aimMode = AimMode::Fixed;
        TrajectoryType trajectoryType = TrajectoryType::Linear;

        float trajectoryParam1 = 0.0f;          // Varies by trajectory type
        float trajectoryParam2 = 0.0f;
        float trajectoryParam3 = 0.0f;

        PatternWave() = default;

        PatternWave& setShape(PatternShape s) { shape = s; return *this; }
        PatternWave& setBulletCount(int count) { bulletCount = count; return *this; }
        PatternWave& setAngle(float offset, float spread = 360.0f) { 
            angleOffset = offset; angleSpread = spread; return *this; 
        }
        PatternWave& setSpeed(float s, float variation = 0.0f) { 
            speed = s; speedVariation = variation; return *this; 
        }
        PatternWave& setTiming(float delay, float burst = 0.0f, int count = 1) {
            spawnDelay = delay; burstDelay = burst; burstCount = count; return *this;
        }
        PatternWave& setBullet(BulletType type, BulletColor color) {
            bulletType = type; bulletColor = color; return *this;
        }
        PatternWave& setAim(AimMode mode) { aimMode = mode; return *this; }
        PatternWave& setTrajectory(TrajectoryType type, float p1 = 0.0f, float p2 = 0.0f, float p3 = 0.0f) {
            trajectoryType = type;
            trajectoryParam1 = p1; trajectoryParam2 = p2; trajectoryParam3 = p3;
            return *this;
        }
    };

} // namespace rtype::ecs
