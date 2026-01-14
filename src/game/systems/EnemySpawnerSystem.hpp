/*
** R-Type ECS - EnemySpawnerSystem
** Handles spawning enemies in waves with configurable patterns
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/SpritesheetComponent.hpp"
#include "engine/ecs/events/definitions/GameEvents.hpp"

#include <vector>
#include <random>
#include <cmath>

namespace rtype::ecs {

    /**
     * @brief Configuration for a single enemy spawn
     */
    struct EnemySpawnConfig {
        EnemyType type = EnemyType::Basic;
        float x = 0.0f, y = 0.0f;
        float vx = -100.0f, vy = 0.0f;
        int health = 1;
        int scoreValue = 100;
        float spawnDelay = 0.0f;        // Per-enemy delay offset for ordered spawning
        bool shootsAtPlayer = false;    // Override shooting behavior
        float fireRate = 1.5f;          // Custom fire rate if shootsAtPlayer
    };

    /**
     * @brief Configuration for a wave of enemies
     */
    struct WaveConfig {
        float delayBefore = 0.0f;       // Delay before wave starts
        std::vector<EnemySpawnConfig> enemies;
        float spawnInterval = 0.5f;     // Time between each enemy spawn
        bool simultaneous = false;      // Spawn all at once vs sequential
        bool ordered = false;           // Respect spawnDelay offsets for precise ordering
    };

    /**
     * @brief Configuration for a powerup spawn in level
     */
    struct PowerupSpawnConfig {
        int type = 0;                   // PowerupType as int
        float x = 0.0f, y = 0.0f;
        float triggerTime = 0.0f;       // When to spawn (seconds from level start)
        bool spawned = false;           // Track if already spawned
    };

    /**
     * @brief Configuration for a bomb spawn in level
     */
    struct BombSpawnConfig {
        float x = 0.0f, y = 0.0f;
        float triggerTime = 0.0f;       // When to spawn (seconds from level start)
        bool spawned = false;           // Track if already spawned
    };

    /**
     * @brief Boss phase configuration
     */
    struct BossPhaseConfig {
        int phase = 1;                  // Phase number
        float healthThreshold = 1.0f;   // Health percentage to trigger (0.0-1.0)
        std::string pattern;            // Attack pattern identifier
        float moveSpeed = 50.0f;        // Movement speed in this phase
    };

    /**
     * @brief Boss section configuration
     */
    struct BossSectionConfig {
        bool enabled = false;
        EnemySpawnConfig boss;          // Boss spawn configuration
        std::vector<BossPhaseConfig> phases;
        float triggerDelay = 2.0f;      // Delay after waves complete before boss spawns
        bool musicChange = true;        // Switch to boss music when triggered
    };

    /**
     * @brief Configuration for an entire level
     */
    struct LevelConfig {
        std::vector<WaveConfig> waves;
        float waveDelay = 2.0f;         // Default delay between waves
        int difficulty = 1;
        
        // Level assets
        std::string name;               // Level name
        std::string background;         // Path to background image
        std::string stageMusic;         // Path to stage music
        std::string bossMusic;          // Path to boss music
        
        // Powerup and bomb spawns
        std::vector<PowerupSpawnConfig> powerupSpawns;
        std::vector<BombSpawnConfig> bombSpawns;
        
        // Boss section
        BossSectionConfig bossSection;
    };

    /**
     * @brief System that spawns enemies according to wave/level configuration
     * 
     * Manages:
     * - Wave progression
     * - Timed enemy spawning
     * - Difficulty scaling
     * - Enemy creation with proper components
     */
    class EnemySpawnerSystem : public ISystem {
    public:
        /**
         * @brief Construct EnemySpawnerSystem
         * @param eventBus Reference to EventBus
         * @param screenWidth Screen width for spawn positioning
         * @param screenHeight Screen height for spawn positioning
         */
        EnemySpawnerSystem(EventBus& eventBus, int screenWidth = 1280, int screenHeight = 720)
            : m_eventBus(eventBus)
            , m_screenWidth(screenWidth)
            , m_screenHeight(screenHeight)
            , m_rng(std::random_device{}()) {
            
            // Create default level configuration
            createDefaultLevel();
        }

        ~EnemySpawnerSystem() override = default;

        /**
         * @brief Update spawner, handle wave timing and enemy creation
         */
        void update(float dt) override {
            if (!m_registry || !m_active) return;

            m_timer += dt;
            m_levelTimer += dt;

            // Update timed powerup/bomb spawns
            updateTimedSpawns();

            // Check for wave completion
            if (m_currentWave >= 0 && m_currentWave < static_cast<int>(m_level.waves.size())) {
                updateCurrentWave(dt);
            } else if (m_currentWave >= static_cast<int>(m_level.waves.size())) {
                // All waves complete - check for boss section
                if (m_level.bossSection.enabled && !m_bossSpawned) {
                    updateBossSection(dt);
                } else if (m_looping && !m_level.bossSection.enabled) {
                    startLevel();
                }
            }
        }

        /**
         * @brief Start the level from the beginning
         */
        void startLevel() {
            m_currentWave = 0;
            m_currentEnemy = 0;
            m_waveTimer = 0.0f;
            m_spawnTimer = 0.0f;
            m_timer = 0.0f;
            m_levelTimer = 0.0f;
            m_waveStarted = false;
            m_active = true;
            m_bossSpawned = false;
            m_bossTriggered = false;
            m_bossTriggerTimer = 0.0f;
            m_bossEntity = Entity();  // Reset to invalid entity
            
            // Reset timed spawns
            for (auto& spawn : m_level.powerupSpawns) {
                spawn.spawned = false;
            }
            for (auto& spawn : m_level.bombSpawns) {
                spawn.spawned = false;
            }

            m_eventBus.emit(events::WaveStarted{1, getWaveEnemyCount(0)});
        }

        /**
         * @brief Stop spawning
         */
        void stop() {
            m_active = false;
        }

        /**
         * @brief Resume spawning
         */
        void resume() {
            m_active = true;
        }

        /**
         * @brief Set whether level loops after completion
         */
        void setLooping(bool loop) {
            m_looping = loop;
        }

        /**
         * @brief Set the level configuration
         */
        void setLevel(const LevelConfig& level) {
            m_level = level;
        }

        /**
         * @brief Get current wave number (1-based)
         */
        int getCurrentWave() const {
            return m_currentWave + 1;
        }

        /**
         * @brief Get total number of waves
         */
        int getTotalWaves() const {
            return static_cast<int>(m_level.waves.size());
        }

        /**
         * @brief Check if spawner is active
         */
        bool isActive() const {
            return m_active;
        }

        /**
         * @brief Spawn a single enemy at specified position
         */
        Entity spawnEnemy(const EnemySpawnConfig& config) {
            Entity enemy = m_registry->createEntity();

            // Transform
            float spawnX = config.x;
            float spawnY = config.y;
            if (spawnX == 0.0f) spawnX = m_screenWidth + 50.0f;  // Off-screen right
            if (spawnY == 0.0f) {
                std::uniform_real_distribution<float> dist(100.0f, m_screenHeight - 100.0f);
                spawnY = dist(m_rng);
            }
            m_registry->addComponent(enemy, TransformComponent(spawnX, spawnY, 0.0f, 1.0f, 1.0f));

            // Velocity based on type
            float vx = config.vx;
            float vy = config.vy;
            float maxSpeed = 200.0f;
            
            switch (config.type) {
                case EnemyType::Basic:
                    vx = -150.0f;
                    maxSpeed = 150.0f;
                    break;
                case EnemyType::Chaser:
                    vx = -80.0f;
                    maxSpeed = 250.0f;
                    break;
                case EnemyType::Shooter:
                    vx = -100.0f;
                    maxSpeed = 120.0f;
                    break;
                case EnemyType::Turret:
                    vx = 0.0f;
                    vy = 0.0f;
                    maxSpeed = 0.0f;
                    break;
                default:
                    break;
            }
            
            m_registry->addComponent(enemy, VelocityComponent(vx, vy, maxSpeed));

            // Enemy component
            m_registry->addComponent(enemy, EnemyComponent(config.type, m_level.difficulty, config.scoreValue));

            // Health
            HealthComponent health;
            health.maxHealth = static_cast<float>(config.health);
            health.currentHealth = health.maxHealth;
            m_registry->addComponent(enemy, health);

            // Collision
            ColliderComponent collision(32.0f, 32.0f, CollisionLayer::Enemy);
            // Combine Player and PlayerShot for mask using bitwise OR on underlying type
            collision.mask = static_cast<CollisionLayer>(
                static_cast<unsigned int>(CollisionLayer::Player) | 
                static_cast<unsigned int>(CollisionLayer::PlayerShot)
            );
            m_registry->addComponent(enemy, collision);

            // Weapon for shooting enemies
            if (config.type == EnemyType::Shooter || config.type == EnemyType::Turret || 
                config.type == EnemyType::Boss || config.shootsAtPlayer) {
                WeaponComponent weapon(config.fireRate, 10);  // Use custom fire rate
                weapon.projectileSpeed = 400.0f;
                m_registry->addComponent(enemy, weapon);
            }

            // Emit spawn event
            m_eventBus.emit(events::EnemySpawned{
                static_cast<EntityId>(enemy),
                static_cast<int>(config.type),
                spawnX, spawnY
            });

            return enemy;
        }

        /**
         * @brief Check if boss section is active
         */
        bool isBossFight() const {
            return m_bossTriggered && !m_bossDefeated;
        }

        /**
         * @brief Check if boss has been defeated
         */
        bool isBossDefeated() const {
            return m_bossDefeated;
        }

        /**
         * @brief Get boss entity (if spawned)
         */
        Entity getBossEntity() const {
            return m_bossEntity;
        }

        SystemPhase getPhase() const override { return SystemPhase::GameLogic; }

    private:
        EventBus& m_eventBus;
        int m_screenWidth;
        int m_screenHeight;
        std::mt19937 m_rng;

        LevelConfig m_level;
        int m_currentWave = -1;
        int m_currentEnemy = 0;
        float m_timer = 0.0f;
        float m_levelTimer = 0.0f;          // Total time since level start
        float m_waveTimer = 0.0f;
        float m_spawnTimer = 0.0f;
        bool m_waveStarted = false;
        bool m_active = false;
        bool m_looping = true;
        
        // Boss section state
        bool m_bossTriggered = false;
        bool m_bossSpawned = false;
        bool m_bossDefeated = false;
        float m_bossTriggerTimer = 0.0f;
        Entity m_bossEntity;  // Default constructor sets id to NULL_ENTITY
        
        // Ordered spawning state
        std::vector<float> m_orderedSpawnTimers;  // Per-enemy timers for ordered waves

        /**
         * @brief Create default level with sample waves
         */
        void createDefaultLevel() {
            m_level.difficulty = 1;
            m_level.waveDelay = 3.0f;

            // Wave 1: Basic enemies from right
            WaveConfig wave1;
            wave1.delayBefore = 1.0f;
            wave1.spawnInterval = 0.8f;
            for (int i = 0; i < 5; ++i) {
                EnemySpawnConfig enemy;
                enemy.type = EnemyType::Basic;
                enemy.y = 150.0f + i * 100.0f;
                enemy.health = 1;
                enemy.scoreValue = 100;
                wave1.enemies.push_back(enemy);
            }
            m_level.waves.push_back(wave1);

            // Wave 2: Chasers
            WaveConfig wave2;
            wave2.delayBefore = 2.0f;
            wave2.spawnInterval = 1.0f;
            for (int i = 0; i < 3; ++i) {
                EnemySpawnConfig enemy;
                enemy.type = EnemyType::Chaser;
                enemy.health = 2;
                enemy.scoreValue = 200;
                wave2.enemies.push_back(enemy);
            }
            m_level.waves.push_back(wave2);

            // Wave 3: Shooters
            WaveConfig wave3;
            wave3.delayBefore = 2.0f;
            wave3.spawnInterval = 1.5f;
            for (int i = 0; i < 4; ++i) {
                EnemySpawnConfig enemy;
                enemy.type = EnemyType::Shooter;
                enemy.y = 100.0f + i * 150.0f;
                enemy.health = 3;
                enemy.scoreValue = 300;
                wave3.enemies.push_back(enemy);
            }
            m_level.waves.push_back(wave3);

            // Wave 4: Mixed wave
            WaveConfig wave4;
            wave4.delayBefore = 3.0f;
            wave4.spawnInterval = 0.6f;
            for (int i = 0; i < 8; ++i) {
                EnemySpawnConfig enemy;
                enemy.type = (i % 3 == 0) ? EnemyType::Shooter : 
                            (i % 3 == 1) ? EnemyType::Chaser : EnemyType::Basic;
                enemy.health = (i % 3 == 0) ? 3 : (i % 3 == 1) ? 2 : 1;
                enemy.scoreValue = enemy.health * 100;
                wave4.enemies.push_back(enemy);
            }
            m_level.waves.push_back(wave4);
        }

        /**
         * @brief Update current wave progress
         */
        void updateCurrentWave(float dt) {
            if (m_currentWave < 0 || m_currentWave >= static_cast<int>(m_level.waves.size())) {
                return;
            }

            WaveConfig& wave = m_level.waves[m_currentWave];

            // Wait for delay before wave starts
            if (!m_waveStarted) {
                m_waveTimer += dt;
                if (m_waveTimer >= wave.delayBefore) {
                    m_waveStarted = true;
                    m_spawnTimer = wave.spawnInterval;  // Spawn first enemy immediately
                    
                    // Initialize ordered spawn timers if needed
                    if (wave.ordered) {
                        m_orderedSpawnTimers.clear();
                        m_orderedSpawnTimers.resize(wave.enemies.size(), 0.0f);
                    }
                }
                return;
            }

            // Spawn enemies
            if (m_currentEnemy < static_cast<int>(wave.enemies.size())) {
                m_spawnTimer += dt;

                if (wave.simultaneous) {
                    // Spawn all at once
                    for (const auto& config : wave.enemies) {
                        spawnEnemy(config);
                    }
                    m_currentEnemy = static_cast<int>(wave.enemies.size());
                } else if (wave.ordered) {
                    // Ordered spawning with per-enemy delays
                    float waveElapsed = m_spawnTimer;
                    for (size_t i = 0; i < wave.enemies.size(); ++i) {
                        if (m_orderedSpawnTimers[i] < 0.0f) continue;  // Already spawned
                        
                        float spawnTime = wave.enemies[i].spawnDelay;
                        if (waveElapsed >= spawnTime && m_orderedSpawnTimers[i] >= 0.0f) {
                            spawnEnemy(wave.enemies[i]);
                            m_orderedSpawnTimers[i] = -1.0f;  // Mark as spawned
                            m_currentEnemy++;
                        }
                    }
                } else {
                    // Sequential spawning
                    if (m_spawnTimer >= wave.spawnInterval) {
                        spawnEnemy(wave.enemies[m_currentEnemy]);
                        m_currentEnemy++;
                        m_spawnTimer = 0.0f;
                    }
                }
            } else {
                // Wave complete, move to next
                m_eventBus.emit(events::WaveCompleted{m_currentWave + 1, m_waveTimer});
                
                m_currentWave++;
                m_currentEnemy = 0;
                m_waveTimer = 0.0f;
                m_spawnTimer = 0.0f;
                m_waveStarted = false;
                m_orderedSpawnTimers.clear();

                if (m_currentWave < static_cast<int>(m_level.waves.size())) {
                    m_eventBus.emit(events::WaveStarted{
                        m_currentWave + 1, 
                        getWaveEnemyCount(m_currentWave)
                    });
                }
            }
        }

        /**
         * @brief Update timed powerup and bomb spawns
         */
        void updateTimedSpawns() {
            // Spawn powerups at their trigger times
            for (auto& spawn : m_level.powerupSpawns) {
                if (!spawn.spawned && m_levelTimer >= spawn.triggerTime) {
                    spawnTimedPowerup(spawn);
                    spawn.spawned = true;
                }
            }
            
            // Spawn bombs at their trigger times
            for (auto& spawn : m_level.bombSpawns) {
                if (!spawn.spawned && m_levelTimer >= spawn.triggerTime) {
                    spawnTimedBomb(spawn);
                    spawn.spawned = true;
                }
            }
        }

        /**
         * @brief Spawn a timed powerup
         */
        void spawnTimedPowerup(const PowerupSpawnConfig& config) {
            if (!m_registry) return;
            
            Entity powerup = m_registry->createEntity();
            
            float spawnX = config.x > 0 ? config.x : m_screenWidth + 50.0f;
            float spawnY = config.y > 0 ? config.y : m_screenHeight / 2.0f;
            
            m_registry->addComponent(powerup, TransformComponent(spawnX, spawnY, 0.0f, 1.0f, 1.0f));
            m_registry->addComponent(powerup, VelocityComponent(-80.0f, 0.0f, 100.0f));  // Float left
            
            // Import PowerupComponent - assume type is properly cast
            // The PowerupSystem will handle the actual pickup logic
            m_eventBus.emit(events::PowerupSpawned{
                static_cast<EntityId>(powerup),
                config.type,
                spawnX, spawnY
            });
        }

        /**
         * @brief Spawn a timed bomb powerup
         */
        void spawnTimedBomb(const BombSpawnConfig& config) {
            if (!m_registry) return;
            
            Entity bomb = m_registry->createEntity();
            
            float spawnX = config.x > 0 ? config.x : m_screenWidth + 50.0f;
            float spawnY = config.y > 0 ? config.y : m_screenHeight / 2.0f;
            
            m_registry->addComponent(bomb, TransformComponent(spawnX, spawnY, 0.0f, 1.0f, 1.0f));
            m_registry->addComponent(bomb, VelocityComponent(-80.0f, 0.0f, 100.0f));  // Float left
            
            // Emit bomb spawn event (type 6 = BOMB)
            m_eventBus.emit(events::PowerupSpawned{
                static_cast<EntityId>(bomb),
                6,  // PowerupType::BOMB
                spawnX, spawnY
            });
        }

        /**
         * @brief Update boss section after all waves complete
         */
        void updateBossSection(float dt) {
            if (!m_level.bossSection.enabled) return;
            
            if (!m_bossTriggered) {
                m_bossTriggerTimer += dt;
                
                if (m_bossTriggerTimer >= m_level.bossSection.triggerDelay) {
                    m_bossTriggered = true;
                    
                    // Emit boss music change event if enabled
                    if (m_level.bossSection.musicChange) {
                        m_eventBus.emit(events::BossFightStarted{
                            0,  // Will be updated after spawn
                            static_cast<int>(m_level.bossSection.phases.size())
                        });
                    }
                    
                    // Spawn the boss
                    EnemySpawnConfig bossConfig = m_level.bossSection.boss;
                    bossConfig.type = EnemyType::Boss;
                    m_bossEntity = spawnEnemy(bossConfig);
                    m_bossSpawned = true;
                }
            }
        }

        /**
         * @brief Get enemy count for a wave
         */
        int getWaveEnemyCount(int waveIndex) const {
            if (waveIndex < 0 || waveIndex >= static_cast<int>(m_level.waves.size())) {
                return 0;
            }
            return static_cast<int>(m_level.waves[waveIndex].enemies.size());
        }
    };

} // namespace rtype::ecs
