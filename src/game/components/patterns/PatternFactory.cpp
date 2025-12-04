/*
** R-Type ECS - Pattern Factory
** Implementation of factory methods for creating common Touhou-style patterns
*/

#include "PatternFactory.hpp"

namespace rtype::ecs {

    BulletPatternComponent PatternFactory::createCircle(int bulletCount, float speed,
                                                        BulletType type, BulletColor color) {
        BulletPatternComponent pattern("Circle");
        PatternWave wave;
        wave.shape = PatternShape::Circle;
        wave.bulletCount = bulletCount;
        wave.speed = speed;
        wave.bulletType = type;
        wave.bulletColor = color;
        pattern.waves.push_back(wave);
        return pattern;
    }

    BulletPatternComponent PatternFactory::createAimedFan(int bulletCount, float spread, float speed,
                                                          BulletType type, BulletColor color) {
        BulletPatternComponent pattern("AimedFan");
        PatternWave wave;
        wave.shape = PatternShape::Fan;
        wave.bulletCount = bulletCount;
        wave.angleSpread = spread;
        wave.speed = speed;
        wave.bulletType = type;
        wave.bulletColor = color;
        wave.aimMode = AimMode::AtPlayer;
        pattern.waves.push_back(wave);
        return pattern;
    }

    BulletPatternComponent PatternFactory::createSpiral(int arms, int bulletsPerArm, float speed,
                                                        float rotSpeed, BulletType type, BulletColor color) {
        BulletPatternComponent pattern("Spiral");
        pattern.rotationSpeed = rotSpeed;

        PatternWave wave;
        wave.shape = PatternShape::Spiral;
        wave.bulletCount = arms;
        wave.burstCount = bulletsPerArm;
        wave.burstDelay = 0.05f;
        wave.speed = speed;
        wave.bulletType = type;
        wave.bulletColor = color;
        pattern.waves.push_back(wave);
        return pattern;
    }

    BulletPatternComponent PatternFactory::createRings(int ringCount, int bulletsPerRing, float baseSpeed,
                                                       float speedIncrement, BulletType type, BulletColor color) {
        BulletPatternComponent pattern("Rings");
        pattern.parallelWaves = true;

        for (int i = 0; i < ringCount; ++i) {
            PatternWave wave;
            wave.shape = PatternShape::Circle;
            wave.bulletCount = bulletsPerRing;
            wave.speed = baseSpeed + i * speedIncrement;
            wave.spawnDelay = i * 0.1f;
            wave.bulletType = type;
            wave.bulletColor = color;
            wave.angleOffset = (i % 2 == 0) ? 0.0f : (180.0f / bulletsPerRing);
            pattern.waves.push_back(wave);
        }
        return pattern;
    }

    BulletPatternComponent PatternFactory::createStream(int bulletCount, float interval, float speed,
                                                        BulletType type, BulletColor color, bool aimed) {
        BulletPatternComponent pattern("Stream");

        PatternWave wave;
        wave.shape = PatternShape::Stream;
        wave.bulletCount = bulletCount;
        wave.burstCount = bulletCount;
        wave.burstDelay = interval;
        wave.speed = speed;
        wave.bulletType = type;
        wave.bulletColor = color;
        wave.aimMode = aimed ? AimMode::AtPlayer : AimMode::Fixed;
        pattern.waves.push_back(wave);
        return pattern;
    }

    BulletPatternComponent PatternFactory::createRose(int petalCount, int bulletsPerPetal, float speed,
                                                      BulletType type, BulletColor color) {
        BulletPatternComponent pattern("Rose");
        float petalAngle = 360.0f / petalCount;

        for (int i = 0; i < petalCount; ++i) {
            PatternWave wave;
            wave.shape = PatternShape::Fan;
            wave.bulletCount = bulletsPerPetal;
            wave.angleOffset = i * petalAngle;
            wave.angleSpread = petalAngle * 0.8f;
            wave.speed = speed;
            wave.bulletType = type;
            wave.bulletColor = color;
            pattern.waves.push_back(wave);
        }
        pattern.parallelWaves = true;
        return pattern;
    }

    BulletPatternComponent PatternFactory::createHoming(int bulletCount, float speed, float homingStrength,
                                                        BulletType type, BulletColor color) {
        BulletPatternComponent pattern("Homing");
        PatternWave wave;
        wave.shape = PatternShape::Circle;
        wave.bulletCount = bulletCount;
        wave.speed = speed;
        wave.bulletType = type;
        wave.bulletColor = color;
        wave.trajectoryType = TrajectoryType::Homing;
        wave.trajectoryParam1 = homingStrength;
        pattern.waves.push_back(wave);
        return pattern;
    }

    BulletPatternComponent PatternFactory::createCross(int bulletsPerArm, float speed,
                                                       BulletType type, BulletColor color) {
        BulletPatternComponent pattern("Cross");
        PatternWave wave;
        wave.shape = PatternShape::Cross;
        wave.bulletCount = 4;
        wave.burstCount = bulletsPerArm;
        wave.burstDelay = 0.05f;
        wave.speed = speed;
        wave.bulletType = type;
        wave.bulletColor = color;
        pattern.waves.push_back(wave);
        return pattern;
    }

    BulletPatternComponent PatternFactory::createWave(int bulletCount, float speed, float amplitude,
                                                      float frequency, BulletType type, BulletColor color) {
        BulletPatternComponent pattern("Wave");
        PatternWave wave;
        wave.shape = PatternShape::Line;
        wave.bulletCount = bulletCount;
        wave.burstDelay = 0.1f;
        wave.burstCount = bulletCount;
        wave.speed = speed;
        wave.bulletType = type;
        wave.bulletColor = color;
        wave.trajectoryType = TrajectoryType::Sinusoidal;
        wave.trajectoryParam1 = amplitude;
        wave.trajectoryParam2 = frequency;
        pattern.waves.push_back(wave);
        return pattern;
    }

    BulletPatternComponent PatternFactory::createHelix(int bulletCount, float speed, float rotSpeed,
                                                       BulletType type, BulletColor color1, BulletColor color2) {
        BulletPatternComponent pattern("Helix");
        pattern.rotationSpeed = rotSpeed;

        PatternWave wave1;
        wave1.shape = PatternShape::Single;
        wave1.bulletCount = 1;
        wave1.burstCount = bulletCount;
        wave1.burstDelay = 0.05f;
        wave1.speed = speed;
        wave1.bulletType = type;
        wave1.bulletColor = color1;
        pattern.waves.push_back(wave1);

        PatternWave wave2 = wave1;
        wave2.angleOffset = 180.0f;
        wave2.bulletColor = color2;
        pattern.waves.push_back(wave2);

        pattern.parallelWaves = true;
        return pattern;
    }

    BulletPatternComponent PatternFactory::createShotgun(int bulletCount, float spread, float speed,
                                                         float speedVariation, BulletType type, BulletColor color) {
        BulletPatternComponent pattern("Shotgun");
        PatternWave wave;
        wave.shape = PatternShape::Fan;
        wave.bulletCount = bulletCount;
        wave.angleSpread = spread;
        wave.speed = speed;
        wave.speedVariation = speedVariation;
        wave.bulletType = type;
        wave.bulletColor = color;
        pattern.waves.push_back(wave);
        return pattern;
    }

} // namespace rtype::ecs
