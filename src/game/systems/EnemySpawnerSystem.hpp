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
    };

    /**
     * @brief Configuration for a wave of enemies
     */
    struct WaveConfig {
        float delayBefore = 0.0f;      // Delay before wave starts
        std::vector<EnemySpawnConfig> enemies;
        float spawnInterval = 0.5f;     // Time between each enemy spawn
        bool simultaneous = false;      // Spawn all at once vs sequential
    };

    /**
     * @brief Configuration for an entire level
     */
    struct LevelConfig {
        std::vector<WaveConfig> waves;
        float waveDelay = 2.0f;         // Default delay between waves
        int difficulty = 1;
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

            // Check for wave completion
            if (m_currentWave >= 0 && m_currentWave < static_cast<int>(m_level.waves.size())) {
                updateCurrentWave(dt);
            } else if (m_currentWave >= static_cast<int>(m_level.waves.size())) {
                // All waves complete - loop or wait
                if (m_looping) {
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
            m_waveStarted = false;
            m_active = true;

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
            if (config.type == EnemyType::Shooter || config.type == EnemyType::Turret || config.type == EnemyType::Boss) {
                WeaponComponent weapon(1.5f, 10);  // Slower fire rate for enemies
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
        float m_waveTimer = 0.0f;
        float m_spawnTimer = 0.0f;
        bool m_waveStarted = false;
        bool m_active = false;
        bool m_looping = true;

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

            const WaveConfig& wave = m_level.waves[m_currentWave];

            // Wait for delay before wave starts
            if (!m_waveStarted) {
                m_waveTimer += dt;
                if (m_waveTimer >= wave.delayBefore) {
                    m_waveStarted = true;
                    m_spawnTimer = wave.spawnInterval;  // Spawn first enemy immediately
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

                if (m_currentWave < static_cast<int>(m_level.waves.size())) {
                    m_eventBus.emit(events::WaveStarted{
                        m_currentWave + 1, 
                        getWaveEnemyCount(m_currentWave)
                    });
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
