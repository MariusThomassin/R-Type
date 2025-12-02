/*
** R-Type ECS - PatternSystem
** Manages bullet pattern spawning from PatternSpawnerComponent entities
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
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
#include <vector>
#include <cstdlib>

namespace rtype::ecs {

    /**
     * @brief System that processes pattern spawners and creates bullets
     *
     * Handles:
     * - Updating spawner timers and states
     * - Interpreting pattern definitions
     * - Calculating aimed shots toward player
     * - Spawning bullets with appropriate components
     */
    class PatternSystem : public ISystem {
    public:
        PatternSystem() = default;
        ~PatternSystem() override = default;

        void update(float dt) override {
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

        SystemPhase getPhase() const override {
            return SystemPhase::GameLogic;
        }

        void setScreenSize(int width, int height) {
            m_screenWidth = width;
            m_screenHeight = height;
        }

    private:
        void updateSpawner(PatternSpawnerComponent& spawner, const TransformComponent& transform, float dt) {
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

        void processWave(PatternSpawnerComponent& spawner, BulletPatternComponent& pattern,
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

        void processBurstSpawn(PatternSpawnerComponent& spawner, BulletPatternComponent& pattern,
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

        void spawnBulletsForWave(const PatternWave& wave, const BulletPatternComponent& pattern,
                                 PatternSpawnerComponent& spawner, float spawnX, float spawnY) {
            float baseAngle = wave.angleOffset + pattern.globalAngleOffset + spawner.currentRotation;

            baseAngle = AimCalculator::calculateAimedAngle(
                baseAngle, wave.aimMode, spawnX, spawnY,
                wave.speed, wave.angleSpread, m_playerTracker);

            spawnPatternShape(wave, pattern, spawner, spawnX, spawnY, baseAngle);
        }

        void spawnPatternShape(const PatternWave& wave, const BulletPatternComponent& pattern,
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
                case PatternShape::Wave:
                    spawnSingleBullet(wave, pattern, spawner, x, y, baseAngle);
                    break;
                default:
                    spawnCirclePattern(wave, pattern, spawner, x, y, baseAngle);
                    break;
            }
        }

        // === Pattern spawn implementations ===

        void spawnSingleBullet(const PatternWave& wave, const BulletPatternComponent& pattern,
                               PatternSpawnerComponent& spawner, float x, float y, float angle) {
            float speed = calculateBulletSpeed(wave, pattern);
            float velX = std::cos(angle * BulletMath::DEG_TO_RAD) * speed;
            float velY = std::sin(angle * BulletMath::DEG_TO_RAD) * speed;
            createBullet(x, y, velX, velY, wave, spawner);
        }

        void spawnFanPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
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

        void spawnCirclePattern(const PatternWave& wave, const BulletPatternComponent& pattern,
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

        void spawnArcPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
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

        void spawnSpiralPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                                PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
            int arms = wave.bulletCount > 0 ? wave.bulletCount : 4;
            float armSpacing = 360.0f / arms;

            for (int arm = 0; arm < arms; ++arm) {
                float angle = baseAngle + arm * armSpacing;
                float speed = calculateBulletSpeed(wave, pattern);
                float velX = std::cos(angle * BulletMath::DEG_TO_RAD) * speed;
                float velY = std::sin(angle * BulletMath::DEG_TO_RAD) * speed;
                createBullet(x, y, velX, velY, wave, spawner);
            }
        }

        void spawnCrossPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                               PatternSpawnerComponent& spawner, float x, float y, float baseAngle) {
            for (int arm = 0; arm < 4; ++arm) {
                float angle = baseAngle + arm * 90.0f;
                float speed = calculateBulletSpeed(wave, pattern);
                float velX = std::cos(angle * BulletMath::DEG_TO_RAD) * speed;
                float velY = std::sin(angle * BulletMath::DEG_TO_RAD) * speed;
                createBullet(x, y, velX, velY, wave, spawner);
            }
        }

        void spawnStarPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
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

        void spawnGridPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
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

        float calculateBulletSpeed(const PatternWave& wave, const BulletPatternComponent& pattern) {
            float speed = wave.speed * pattern.globalSpeedMultiplier;
            if (wave.speedVariation > 0.0f) {
                speed += (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * 2.0f * wave.speedVariation;
            }
            return speed;
        }

        void createBullet(float x, float y, float velX, float velY,
                          const PatternWave& wave, PatternSpawnerComponent& spawner) {
            (void)spawner;

            Entity bullet = m_registry->createEntity();

            float bulletRotation = 0.0f;
            if (wave.bulletType == BulletType::Rice || 
                wave.bulletType == BulletType::Dot) {
                bulletRotation = std::atan2(velY, velX) * (180.0f / 3.14159265f) - 90.0f;
            }

            m_registry->addComponent(bullet, TransformComponent(x, y, bulletRotation, 1.5f, 1.5f));

            float maxSpeed = std::sqrt(velX * velX + velY * velY) * 2.0f;
            m_registry->addComponent(bullet, VelocityComponent(velX, velY, maxSpeed));

            SpritesheetComponent sprite(wave.bulletType, wave.bulletColor);
            sprite.hasGlow = true;
            sprite.glowIntensity = 0.5f;
            sprite.layer = 99 - (m_bulletCounter % 100);
            m_bulletCounter++;
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
        }

        void addTrajectoryComponent(Entity& bullet, const PatternWave& wave,
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

    private:
        float m_gameTime = 0.0f;
        int m_screenWidth = 1280;
        int m_screenHeight = 720;
        int m_bulletCounter = 0;
        PlayerTracker m_playerTracker;
    };

} // namespace rtype::ecs
