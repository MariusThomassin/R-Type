/*
** R-Type ECS - BulletSystem Implementation
** Handles projectile spawning, movement, and lifetime
*/

#include "BulletSystem.hpp"
#include <cmath>
#include <vector>
#include <utility>

namespace rtype::ecs {

    BulletSystem::BulletSystem(EventBus& eventBus) : m_eventBus(eventBus) {
        m_shootSubId = m_eventBus.subscribe<events::ShootEvent>(
            [this](const events::ShootEvent& e) {
                spawnProjectile(e.shooterId);
            }
        );

        m_danmakuSubId = m_eventBus.subscribe<events::DanmakuEvent>(
            [this](const events::DanmakuEvent& e) {
                (void)e;
                spawnDanmakuPattern();
            }
        );
    }

    BulletSystem::~BulletSystem() {
        m_eventBus.unsubscribe<events::ShootEvent>(m_shootSubId);
        m_eventBus.unsubscribe<events::DanmakuEvent>(m_danmakuSubId);
    }

    void BulletSystem::update(float dt) {
        if (!m_registry) return;

        m_gameTime += dt;

        ProjectileLifetimeManager::updateLifetimes(*m_registry, dt, m_expiredProjectiles);
        ProjectileLifetimeManager::destroyExpired(*m_registry, m_expiredProjectiles);
        ProjectileLifetimeManager::destroyOffscreen(*m_registry, m_screenWidth, m_screenHeight);
    }

    void BulletSystem::setScreenSize(int width, int height) {
        m_screenWidth = width;
        m_screenHeight = height;
    }

    SystemPhase BulletSystem::getPhase() const {
        return SystemPhase::GameLogic;
    }

    EntityId BulletSystem::spawnProjectile(EntityId shooterId) {
        if (!m_registry) return NULL_ENTITY;
        if (!m_registry->hasComponent<WeaponComponent>(shooterId)) return NULL_ENTITY;
        if (!m_registry->hasComponent<TransformComponent>(shooterId)) return NULL_ENTITY;

        auto& weapon = m_registry->getComponent<WeaponComponent>(shooterId);
        const auto& transform = m_registry->getComponent<TransformComponent>(shooterId);

        if (!weapon.isReady(m_gameTime)) {
            return NULL_ENTITY;
        }

        weapon.lastFiredTime = m_gameTime;

        bool isPlayer = m_registry->hasComponent<PlayerComponent>(shooterId);
        float direction = isPlayer ? 1.0f : -1.0f;
        float offsetX = 40.0f * transform.scaleX;

        EntityId firstProjectile = NULL_ENTITY;

        // Determine shot pattern based on weapon power level (0-4)
        // Level 0: 1 shot straight
        // Level 1: 2 shots parallel
        // Level 2: 3 shots (straight + slight angle)
        // Level 3: 4 shots spread
        // Level 4+: 5 shots wide spread
        int level = weapon.powerLevel;
        int shotCount = 1;
        std::vector<std::pair<float, float>> shotOffsets; // {angleOffset, yOffset}

        if (level == 0) {
            shotCount = 1;
            shotOffsets.push_back({0.0f, 0.0f});
        } else if (level == 1) {
            shotCount = 2;
            shotOffsets.push_back({0.0f, -10.0f});
            shotOffsets.push_back({0.0f, 10.0f});
        } else if (level == 2) {
            shotCount = 3;
            shotOffsets.push_back({0.0f, 0.0f});
            shotOffsets.push_back({10.0f, -15.0f});  // Slight upward angle
            shotOffsets.push_back({-10.0f, 15.0f}); // Slight downward angle
        } else if (level == 3) {
            shotCount = 4;
            shotOffsets.push_back({0.0f, -8.0f});
            shotOffsets.push_back({0.0f, 8.0f});
            shotOffsets.push_back({15.0f, -20.0f});
            shotOffsets.push_back({-15.0f, 20.0f});
        } else { // level >= 4
            shotCount = 5;
            shotOffsets.push_back({0.0f, 0.0f});
            shotOffsets.push_back({8.0f, -12.0f});
            shotOffsets.push_back({-8.0f, 12.0f});
            shotOffsets.push_back({20.0f, -24.0f});
            shotOffsets.push_back({-20.0f, 24.0f});
        }

        // Spawn each projectile in the pattern
        for (int i = 0; i < shotCount; i++) {
            float angleOffset = shotOffsets[i].first;
            float yOffset = shotOffsets[i].second;

            Entity projectile = m_registry->createEntity();

            m_registry->addComponent(projectile, TransformComponent(
                transform.x + offsetX,
                transform.y + yOffset,
                0.0f,
                1.0f, 1.0f
            ));

            // Calculate velocity with angle offset
            float baseSpeed = weapon.projectileSpeed;
            float angleRad = angleOffset * (3.14159265f / 180.0f);
            float velX = baseSpeed * direction * std::cos(angleRad);
            float velY = baseSpeed * std::sin(angleRad) * direction;

            m_registry->addComponent(projectile, VelocityComponent(
                velX,
                velY,
                baseSpeed * 1.5f
            ));

            SpritesheetComponent bulletSprite;
            bulletSprite.textureId = "touhou_bullets";
            bulletSprite.frameWidth = 16;
            bulletSprite.frameHeight = 16;
            bulletSprite.hasGlow = true;
            bulletSprite.glowIntensity = 0.4f;

            if (isPlayer) {
                bulletSprite.layer = 100;
                // Color varies with power level
                BulletColor color = static_cast<BulletColor>(std::min(level, 7));
                bulletSprite.setBullet(BulletType::Rice, color);
                bulletSprite.rotation = -90.0f + angleOffset;
                // Apply color tint based on level
                switch (level) {
                    case 0: bulletSprite.tintR = 80; bulletSprite.tintG = 200; bulletSprite.tintB = 255; break;  // Cyan
                    case 1: bulletSprite.tintR = 80; bulletSprite.tintG = 255; bulletSprite.tintB = 80; break;   // Green
                    case 2: bulletSprite.tintR = 255; bulletSprite.tintG = 255; bulletSprite.tintB = 80; break;  // Yellow
                    case 3: bulletSprite.tintR = 255; bulletSprite.tintG = 150; bulletSprite.tintB = 50; break;  // Orange
                    default: bulletSprite.tintR = 255; bulletSprite.tintG = 80; bulletSprite.tintB = 255; break; // Magenta
                }
            } else {
                bulletSprite.layer = 99 - (m_enemyBulletCounter % 100);
                m_enemyBulletCounter++;
                BulletType type = (std::rand() % 2 == 0) ? BulletType::Ball : BulletType::Outline;
                BulletColor color = static_cast<BulletColor>(std::rand() % static_cast<int>(BulletColor::COUNT));
                bulletSprite.setBullet(type, color);
                BulletColors::applyBulletColorTint(bulletSprite, color);
            }

            m_registry->addComponent(projectile, bulletSprite);

            m_registry->addComponent(projectile, ProjectileComponent(
                shooterId,
                weapon.getEffectiveDamage(),  // Use level-scaled damage
                isPlayer
            ));

            if (isPlayer) {
                m_registry->addComponent(projectile, LifetimeComponent(3.0f));
            }

            if (firstProjectile == NULL_ENTITY) {
                firstProjectile = projectile.id;
            }
        }

        return firstProjectile;
    }

    EntityId BulletSystem::spawnTouhouBullet(float x, float y, float velX, float velY,
                               BulletType bulletType, BulletColor bulletColor,
                               bool isPlayerBullet,
                               const TrajectoryComponent* trajectory) {
        if (!m_registry) return NULL_ENTITY;

        Entity projectile = m_registry->createEntity();

        float bulletRotation = 0.0f;
        if (bulletType == BulletType::Rice || bulletType == BulletType::Dot) {
            bulletRotation = std::atan2(velY, velX) * (180.0f / 3.14159265f) - 90.0f;
        }

        m_registry->addComponent(projectile, TransformComponent(x, y, bulletRotation, 1.5f, 1.5f));
        m_registry->addComponent(projectile, VelocityComponent(velX, velY, 1000.0f));

        SpritesheetComponent bulletSprite(bulletType, bulletColor);
        bulletSprite.hasGlow = true;
        bulletSprite.glowIntensity = 0.5f;
        
        if (isPlayerBullet) {
            bulletSprite.layer = 100;
        } else {
            bulletSprite.layer = 99 - (m_enemyBulletCounter % 100);
            m_enemyBulletCounter++;
        }

        BulletColors::applyBulletColorTint(bulletSprite, bulletColor);

        m_registry->addComponent(projectile, bulletSprite);
        m_registry->addComponent(projectile, ProjectileComponent(NULL_ENTITY, 10, isPlayerBullet));

        if (bulletType == BulletType::Ball || bulletType == BulletType::Dot) {
            m_registry->addComponent(projectile, SpinComponent::randomSpin(120.0f, 400.0f, 0.4f));
        }

        if (trajectory != nullptr) {
            TrajectoryComponent traj = *trajectory;
            traj.baseVelX = velX;
            traj.baseVelY = velY;
            m_registry->addComponent(projectile, traj);
        }

        m_registry->addComponent(projectile, LifetimeComponent(8.0f));

        return projectile.id;
    }

    std::vector<EntityId> BulletSystem::spawnBulletBatch(const std::vector<BulletSpawnData>& bullets) {
        std::vector<EntityId> created;
        created.reserve(bullets.size());

        for (const auto& data : bullets) {
            EntityId id;
            
            if (data.trajectoryType != TrajectoryType::Linear) {
                TrajectoryComponent traj(data.trajectoryType);
                switch (data.trajectoryType) {
                    case TrajectoryType::Homing:
                        traj.homingStrength = data.trajParam1 > 0.0f ? data.trajParam1 : 5.0f;
                        traj.homingDuration = data.trajParam2;
                        break;
                    case TrajectoryType::Sinusoidal:
                        traj.waveAmplitude = data.trajParam1 > 0.0f ? data.trajParam1 : 50.0f;
                        traj.waveFrequency = data.trajParam2 > 0.0f ? data.trajParam2 : 3.0f;
                        traj.wavePhase = data.trajParam3;
                        break;
                    case TrajectoryType::Accelerating:
                        traj.targetSpeed = data.trajParam1;
                        traj.acceleration = data.trajParam2 > 0.0f ? data.trajParam2 : 200.0f;
                        traj.speedChangeDelay = data.trajParam3;
                        break;
                    default:
                        break;
                }
                id = spawnTouhouBullet(data.x, data.y, data.velX, data.velY,
                                       data.type, data.color, data.isPlayer, &traj);
            } else {
                id = spawnTouhouBullet(data.x, data.y, data.velX, data.velY,
                                       data.type, data.color, data.isPlayer);
            }

            if (id != NULL_ENTITY) {
                created.push_back(id);
            }
        }

        return created;
    }

    void BulletSystem::spawnCirclePattern(float x, float y, int bulletCount, float speed,
                           BulletType bulletType, BulletColor bulletColor) {
        float angleStep = 360.0f / static_cast<float>(bulletCount);

        for (int i = 0; i < bulletCount; ++i) {
            float angle = i * angleStep * (3.14159f / 180.0f);
            float velX = std::cos(angle) * speed;
            float velY = std::sin(angle) * speed;

            spawnTouhouBullet(x, y, velX, velY, bulletType, bulletColor, false);
        }
    }

    void BulletSystem::spawnSpiralPattern(float x, float y, int arms, int bulletsPerArm,
                           float speed, BulletType bulletType, BulletColor bulletColor) {
        float armAngleStep = 360.0f / static_cast<float>(arms);

        for (int arm = 0; arm < arms; ++arm) {
            float baseAngle = arm * armAngleStep;

            for (int i = 0; i < bulletsPerArm; ++i) {
                float spiralOffset = i * 15.0f;  // Spiral twist
                float angle = (baseAngle + spiralOffset) * (3.14159f / 180.0f);
                float bulletSpeed = speed + i * 20.0f;  // Accelerating spiral

                float velX = std::cos(angle) * bulletSpeed;
                float velY = std::sin(angle) * bulletSpeed;

                spawnTouhouBullet(x, y, velX, velY, bulletType, bulletColor, false);
            }
        }
    }

    float BulletSystem::getGameTime() const {
        return m_gameTime;
    }

    void BulletSystem::spawnDanmakuPattern() {
        float spawnX = m_screenWidth * 0.75f;
        float spawnY = m_screenHeight / 2.0f;

        static const BulletType types[] = { BulletType::Ball, BulletType::Pellet, BulletType::Rice, BulletType::Dot };
        BulletType type1 = types[std::rand() % 4];
        BulletType type2 = types[std::rand() % 4];
        
        BulletColor color1 = static_cast<BulletColor>(std::rand() % static_cast<int>(BulletColor::COUNT));
        BulletColor color2 = static_cast<BulletColor>(std::rand() % static_cast<int>(BulletColor::COUNT));

        spawnCirclePattern(spawnX, spawnY, 24, 200.0f, type1, color1);
        spawnCirclePattern(spawnX, spawnY, 16, 150.0f, type2, color2);
    }

} // namespace rtype::ecs