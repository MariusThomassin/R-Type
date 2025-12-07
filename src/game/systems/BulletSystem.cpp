/*
** R-Type ECS - BulletSystem Implementation
** Handles projectile spawning, movement, and lifetime
*/

#include "BulletSystem.hpp"

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

        Entity projectile = m_registry->createEntity();

        float offsetX = 40.0f * transform.scaleX;  // Spawn in front of ship
        m_registry->addComponent(projectile, TransformComponent(
            transform.x + offsetX,
            transform.y,
            0.0f,
            1.0f, 1.0f  // Scale controlled by SpritesheetComponent
        ));

        float direction = isPlayer ? 1.0f : -1.0f;
        m_registry->addComponent(projectile, VelocityComponent(
            weapon.projectileSpeed * direction,
            0.0f,
            weapon.projectileSpeed * 1.5f  // Max speed
        ));

        SpritesheetComponent bulletSprite;
        bulletSprite.textureId = "touhou_bullets";
        bulletSprite.frameWidth = 16;
        bulletSprite.frameHeight = 16;
        bulletSprite.hasGlow = true;
        bulletSprite.glowIntensity = 0.4f;

        if (isPlayer) {
            bulletSprite.layer = 100;
            bulletSprite.setBullet(BulletType::Rice, BulletColor::Cyan);
            bulletSprite.rotation = -90.0f;
            bulletSprite.tintR = 80;
            bulletSprite.tintG = 240;
            bulletSprite.tintB = 255;
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
            weapon.damage,
            isPlayer
        ));

        if (isPlayer) {
            m_registry->addComponent(projectile, LifetimeComponent(3.0f));
        }

        return projectile.id;
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