/*
** R-Type ECS - PatternSystem Implementation
** Manages bullet pattern spawning from PatternSpawnerComponent entities
*/

#include "PatternSystem.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "game/components/patterns/PatternSpawnerComponent.hpp"
#include "game/components/patterns/BulletPatternComponent.hpp"
#include "game/components/patterns/PatternWave.hpp"
#include "game/components/bullets/TrajectoryComponent.hpp"
#include "game/components/bullets/SpinComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/components/SpritesheetComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "common/GameMath.hpp"
#include "common/TargetTracker.hpp"
#include "common/ColorPalette.hpp"
#include "common/TargetingCalculator.hpp"
#include "patterns/PatternStateManager.hpp"

#include <cmath>

namespace rtype::ecs {

    void PatternSystem::update(float dt) {
        if (!m_registry) return;

        m_gameTime += dt;
        m_playerTracker.update(*m_registry);

        m_registry->forEach<PatternSpawnerComponent, TransformComponent>(
            [this, dt](EntityId entity) {
                auto& spawner = m_registry->getComponent<PatternSpawnerComponent>(entity);
                const auto& transform = m_registry->getComponent<TransformComponent>(entity);
                updateSpawner(spawner, transform, dt);
            }
        );
    }

    void PatternSystem::updateSpawner(PatternSpawnerComponent& spawner, const TransformComponent& transform, float dt) {
        if (PatternStateManager::shouldStart(spawner)) {
            spawner.start();
        }

        if (!PatternStateManager::isReadyToProcess(spawner)) {
            return;
        }

        PatternSlot* slot = spawner.getActiveSlot();
        if (!slot || !slot->enabled) return;

        BulletPatternComponent& pattern = slot->pattern;

        if (spawner.stateTimer < slot->startDelay) {
            spawner.stateTimer += dt;
            return;
        }

        spawner.currentRotation += pattern.rotationSpeed * dt;
        spawner.currentRotation = BulletMath::normalizeAngleDeg(spawner.currentRotation);
        spawner.patternTimer += dt;

        if (spawner.currentWaveIndex < static_cast<int>(pattern.waves.size())) {
            processWave(spawner, pattern, transform, dt);
        } else {
            PatternStateManager::handlePatternComplete(spawner);
        }
    }

    void PatternSystem::processWave(PatternSpawnerComponent& spawner, BulletPatternComponent& pattern,
                                   const TransformComponent& transform, float dt) {
        PatternWave& wave = pattern.waves[spawner.currentWaveIndex];

        if (spawner.waveTimer < wave.spawnDelay) {
            spawner.waveTimer += dt;
            return;
        }

        float spawnX = transform.x + spawner.spawnOffsetX + pattern.offsetX;
        float spawnY = transform.y + spawner.spawnOffsetY + pattern.offsetY;

        if (wave.burstCount > 1) {
            processBurstSpawn(spawner, pattern, wave, spawnX, spawnY, dt);
        } else {
            spawnBulletsForWave(wave, pattern, spawner, spawnX, spawnY);
            PatternStateManager::advanceWave(spawner, pattern);
        }
    }

    void PatternSystem::spawnBulletsForWave(const PatternWave& wave, const BulletPatternComponent& pattern,
                                           PatternSpawnerComponent& spawner, float spawnX, float spawnY) {
        float baseAngle = wave.angleOffset + pattern.globalAngleOffset + spawner.currentRotation;

        baseAngle = AimCalculator::calculateAimedAngle(
            baseAngle, wave.aimMode, spawnX, spawnY,
            wave.speed, wave.angleSpread, m_playerTracker);

        spawnPatternShape(wave, pattern, spawner, spawnX, spawnY, baseAngle);
    }

    void PatternSystem::spawnPatternShape(const PatternWave& wave, const BulletPatternComponent& pattern,
                                         PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
        switch (wave.shape) {
            case PatternShape::Single:
            case PatternShape::Line:
            case PatternShape::Stream:
                spawnSingleBullet(wave, pattern, spawner, x, y, baseAngle);
                break;
            case PatternShape::Fan:
                spawnFanPattern(wave, pattern, spawner, x, y, baseAngle);
                break;
            case PatternShape::Circle:
            case PatternShape::Ring:
                spawnCirclePattern(wave, pattern, spawner, x, y, baseAngle);
                break;
            case PatternShape::Arc:
                spawnArcPattern(wave, pattern, spawner, x, y, baseAngle);
                break;
            case PatternShape::Spiral:
                spawnSpiralPattern(wave, pattern, spawner, x, y, baseAngle);
                break;
            case PatternShape::Cross:
                spawnCrossPattern(wave, pattern, spawner, x, y, baseAngle);
                break;
            case PatternShape::Star:
                spawnStarPattern(wave, pattern, spawner, x, y, baseAngle);
                break;
            case PatternShape::Grid:
                spawnGridPattern(wave, pattern, spawner, x, y, baseAngle);
                break;
            default:
                spawnSingleBullet(wave, pattern, spawner, x, y, baseAngle);
                break;
        }
    }

    void PatternSystem::spawnSingleBullet(const PatternWave& wave, const BulletPatternComponent& pattern,
                                         PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
        float speed = calculateBulletSpeed(wave, pattern);
        float velX = std::cos(baseAngle * BulletMath::DEG_TO_RAD) * speed;
        float velY = std::sin(baseAngle * BulletMath::DEG_TO_RAD) * speed;
        createBullet(x, y, velX, velY, wave, spawner);
    }

    void PatternSystem::spawnFanPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                                       PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
        if (wave.bulletCount <= 0) return;

        float startAngle = baseAngle - wave.angleSpread / 2.0f;
        float angleStep = (wave.bulletCount > 1) ? wave.angleSpread / (wave.bulletCount - 1) : 0.0f;

        for (int i = 0; i < wave.bulletCount; ++i) {
            float angle = startAngle + i * angleStep;
            float speed = calculateBulletSpeed(wave, pattern);
            float velX = std::cos(angle * BulletMath::DEG_TO_RAD) * speed;
            float velY = std::sin(angle * BulletMath::DEG_TO_RAD) * speed;
            createBullet(x, y, velX, velY, wave, spawner);
        }
    }

    void PatternSystem::spawnCirclePattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                                          PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
        if (wave.bulletCount <= 0) return;

        float angleStep = 360.0f / wave.bulletCount;

        for (int i = 0; i < wave.bulletCount; ++i) {
            float angle = baseAngle + i * angleStep;
            float speed = calculateBulletSpeed(wave, pattern);
            float velX = std::cos(angle * BulletMath::DEG_TO_RAD) * speed;
            float velY = std::sin(angle * BulletMath::DEG_TO_RAD) * speed;
            createBullet(x, y, velX, velY, wave, spawner);
        }
    }

    void PatternSystem::spawnArcPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                                       PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
        if (wave.bulletCount <= 0) return;

        float angleStep = wave.angleSpread / wave.bulletCount;

        for (int i = 0; i < wave.bulletCount; ++i) {
            float angle = baseAngle - wave.angleSpread / 2.0f + i * angleStep;
            float speed = calculateBulletSpeed(wave, pattern);
            float velX = std::cos(angle * BulletMath::DEG_TO_RAD) * speed;
            float velY = std::sin(angle * BulletMath::DEG_TO_RAD) * speed;
            createBullet(x, y, velX, velY, wave, spawner);
        }
    }

    void PatternSystem::spawnSpiralPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                                          PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
        if (wave.bulletCount <= 0) return;

        float angleStep = wave.angleSpread / wave.bulletCount;

        for (int i = 0; i < wave.bulletCount; ++i) {
            float angle = baseAngle + i * angleStep;
            float speed = calculateBulletSpeed(wave, pattern);
            float velX = std::cos(angle * BulletMath::DEG_TO_RAD) * speed;
            float velY = std::sin(angle * BulletMath::DEG_TO_RAD) * speed;
            createBullet(x, y, velX, velY, wave, spawner);
        }
    }

    void PatternSystem::spawnCrossPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                                         PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
        int bulletsPerArm = wave.bulletCount / 4;
        if (bulletsPerArm <= 0) bulletsPerArm = 1;

        for (int arm = 0; arm < 4; ++arm) {
            float armAngle = baseAngle + arm * 90.0f;
            for (int i = 0; i < bulletsPerArm; ++i) {
                float speed = calculateBulletSpeed(wave, pattern);
                float velX = std::cos(armAngle * BulletMath::DEG_TO_RAD) * speed;
                float velY = std::sin(armAngle * BulletMath::DEG_TO_RAD) * speed;
                createBullet(x, y, velX, velY, wave, spawner);
            }
        }
    }

    void PatternSystem::spawnStarPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                                        PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
        int points = wave.bulletCount > 0 ? wave.bulletCount : 5;
        float pointSpacing = 360.0f / points;

        for (int i = 0; i < points * 2; ++i) {
            float angle = baseAngle + i * (pointSpacing / 2.0f);
            float speed = calculateBulletSpeed(wave, pattern) * (i % 2 == 1 ? 0.6f : 1.0f);
            float velX = std::cos(angle * BulletMath::DEG_TO_RAD) * speed;
            float velY = std::sin(angle * BulletMath::DEG_TO_RAD) * speed;
            createBullet(x, y, velX, velY, wave, spawner);
        }
    }

    void PatternSystem::spawnGridPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                                        PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
        int gridSize = static_cast<int>(std::sqrt(static_cast<float>(wave.bulletCount)));
        if (gridSize < 2) gridSize = 2;

        float spacing = 30.0f;
        float startX = x - (gridSize - 1) * spacing / 2.0f;
        float startY = y - (gridSize - 1) * spacing / 2.0f;

        for (int row = 0; row < gridSize; ++row) {
            for (int col = 0; col < gridSize; ++col) {
                float bx = startX + col * spacing;
                float by = startY + row * spacing;
                float speed = calculateBulletSpeed(wave, pattern);
                float velX = std::cos(baseAngle * BulletMath::DEG_TO_RAD) * speed;
                float velY = std::sin(baseAngle * BulletMath::DEG_TO_RAD) * speed;
                createBullet(bx, by, velX, velY, wave, spawner);
            }
        }
    }

    void PatternSystem::createBullet(float x, float y, float velX, float velY,
                                    const PatternWave& wave, PatternSpawnerComponent& spawner) {
        Entity bullet = m_registry->createEntity();

        TransformComponent transform(x, y);
        m_registry->addComponent(bullet, transform);

        VelocityComponent velocity(velX, velY, 1000.0f);
        m_registry->addComponent(bullet, velocity);

        SpritesheetComponent sprite(wave.bulletType, wave.bulletColor);
        BulletColors::applyBulletColorTint(sprite, wave.bulletColor);
        m_registry->addComponent(bullet, sprite);

        m_registry->addComponent(bullet, ProjectileComponent(NULL_ENTITY, 10, false));
        m_registry->addComponent(bullet, LifetimeComponent(8.0f));
        
        if (wave.bulletType == BulletType::Ball || wave.bulletType == BulletType::Dot) {
            m_registry->addComponent(bullet, SpinComponent::randomSpin(120.0f, 400.0f, 0.4f));
        }

        if (wave.trajectoryType != TrajectoryType::Linear) {
            addTrajectoryComponent(bullet, wave, velX, velY, x, y);
        }

        m_bulletCounter++;
    }

    void PatternSystem::addTrajectoryComponent(Entity& bullet, const PatternWave& wave,
                                              float velX, float velY, float x, float y) {
        TrajectoryComponent trajectory(wave.trajectoryType);
        trajectory.baseVelX = velX;
        trajectory.baseVelY = velY;

        switch (wave.trajectoryType) {
            case TrajectoryType::Homing:
                trajectory.targetId = m_playerTracker.getPlayerId();
                trajectory.homingStrength = wave.trajectoryParam1 > 0.0f ? wave.trajectoryParam1 : 5.0f;
                trajectory.homingDuration = wave.trajectoryParam2;
                break;
            case TrajectoryType::Sinusoidal:
                trajectory.waveAmplitude = wave.trajectoryParam1 > 0.0f ? wave.trajectoryParam1 : 50.0f;
                trajectory.waveFrequency = wave.trajectoryParam2 > 0.0f ? wave.trajectoryParam2 : 3.0f;
                trajectory.wavePhase = wave.trajectoryParam3;
                break;
            case TrajectoryType::Accelerating:
                trajectory.targetSpeed = wave.trajectoryParam1 > 0.0f ? wave.trajectoryParam1 : wave.speed * 2.0f;
                trajectory.acceleration = wave.trajectoryParam2 > 0.0f ? wave.trajectoryParam2 : 200.0f;
                trajectory.speedChangeDelay = wave.trajectoryParam3;
                break;
            case TrajectoryType::Circular:
                trajectory.orbitCenterX = x;
                trajectory.orbitCenterY = y;
                trajectory.angularVelocity = wave.trajectoryParam1 > 0.0f ? wave.trajectoryParam1 : 3.0f;
                trajectory.radiusChangeRate = wave.trajectoryParam2;
                break;
            default:
                break;
        }

        m_registry->addComponent(bullet, trajectory);
    }

    void PatternSystem::processBurstSpawn(PatternSpawnerComponent& spawner, BulletPatternComponent& pattern,
                                         const PatternWave& wave, float spawnX, float spawnY, float dt) {
        spawner.burstTimer += dt;

        while (spawner.burstBulletsSpawned < wave.burstCount && spawner.burstTimer >= wave.burstDelay) {
            spawnBulletsForWave(wave, pattern, spawner, spawnX, spawnY);
            spawner.burstBulletsSpawned++;
            spawner.burstTimer -= wave.burstDelay;
        }

        if (spawner.burstBulletsSpawned >= wave.burstCount) {
            PatternStateManager::advanceWave(spawner, pattern);
        }
    }

} // namespace rtype::ecs