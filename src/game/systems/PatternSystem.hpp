/*
** R-Type ECS - PatternSystem
** Manages bullet pattern spawning from PatternSpawnerComponent entities
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "game/components/patterns/PatternSpawnerComponent.hpp"
#include "game/components/patterns/BulletPatternComponent.hpp"
#include "game/components/patterns/PatternWave.hpp"
#include "common/TargetTracker.hpp"

#include <vector>

namespace rtype::ecs {

    // Forward declarations
    class Entity;

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
        /**
         * @brief Construct a new Pattern System object
         */
        PatternSystem() = default;
        /**
         * @brief Destroy the Pattern System object
         */
        ~PatternSystem() override = default;

        /**
         * @brief Update all PatternSpawnerComponent entities
         * @param dt Delta time since last update
         */
        void update(float dt) override;

        /**
         * @brief Get the system phase (GameLogic)
         * @return SystemPhase
         */
        SystemPhase getPhase() const override {
            return SystemPhase::GameLogic;
        }

        /**
         * @brief Set screen dimensions for calculations
         * @param width Screen width
         * @param height Screen height
         */
        void setScreenSize(int width, int height) {
            m_screenWidth = width;
            m_screenHeight = height;
        }

    private:
        /**
         * @brief Game time accumulator for time-based effects
         */
        float m_gameTime = 0.0f;
        /**
         * @brief Screen width for aimed calculations
         */
        int m_screenWidth = 1280;
        /**
         * @brief Screen height for aimed calculations
         */
        int m_screenHeight = 720;
        /**
         * @brief Counter for bullets spawned, used for layering
         */
        int m_bulletCounter = 0;
        /**
         * @brief Player tracker for aimed shots
         */
        PlayerTracker m_playerTracker;

        /**
         * @brief Update a single PatternSpawnerComponent
         * @param spawner Reference to the spawner component
         * @param transform Reference to the entity's transform
         * @param dt Delta time since last update
         */
        void updateSpawner(PatternSpawnerComponent& spawner, const TransformComponent& transform, float dt);

        /**
         * @brief Process the current wave of the pattern
         * @param spawner Reference to the spawner component
         * @param pattern Reference to the active bullet pattern
         * @param transform Reference to the entity's transform
         * @param dt Delta time since last update
         */
        void processWave(PatternSpawnerComponent& spawner, BulletPatternComponent& pattern,
                        const TransformComponent& transform, float dt);

        /**
         * @brief Process burst spawning for a wave
         * @param spawner Reference to the spawner component
         * @param pattern Reference to the active bullet pattern
         * @param wave Reference to the current pattern wave
         * @param spawnX X coordinate for spawning bullets
         * @param spawnY Y coordinate for spawning bullets
         * @param dt Delta time since last update
         */
        void processBurstSpawn(PatternSpawnerComponent& spawner, BulletPatternComponent& pattern,
                               const PatternWave& wave, float spawnX, float spawnY, float dt);

        /**
         * @brief Spawn bullets for a given wave
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @param spawner Reference to the spawner component
         * @param spawnX X coordinate for spawning bullets
         * @param spawnY Y coordinate for spawning bullets
         */
        void spawnBulletsForWave(const PatternWave& wave, const BulletPatternComponent& pattern,
                                 PatternSpawnerComponent& spawner, float spawnX, float spawnY);

        /**
         * @brief Spawn bullets according to the wave's shape
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @param spawner Reference to the spawner component
         * @param x X coordinate for spawning bullets
         * @param y Y coordinate for spawning bullets
         * @param baseAngle Base angle for spawning bullets
         */
        void spawnPatternShape(const PatternWave& wave, const BulletPatternComponent& pattern,
                               PatternSpawnerComponent& spawner, float x, float y, float baseAngle);

        /**
         * @brief Spawn a single bullet
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @param spawner Reference to the spawner component
         * @param x X coordinate for spawning bullets
         * @param y Y coordinate for spawning bullets
         * @param baseAngle Base angle for spawning bullets
         */
        void spawnSingleBullet(const PatternWave& wave, const BulletPatternComponent& pattern,
                               PatternSpawnerComponent& spawner, float x, float y, float baseAngle);

        /**
         * @brief Spawn a fan pattern of bullets
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @param spawner Reference to the spawner component
         * @param x X coordinate for spawning bullets
         * @param y Y coordinate for spawning bullets
         * @param baseAngle Base angle for spawning bullets
         */
        void spawnFanPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                             PatternSpawnerComponent& spawner, float x, float y, float baseAngle);

        /**
         * @brief Spawn a circle pattern of bullets
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @param spawner Reference to the spawner component
         * @param x X coordinate for spawning bullets
         * @param y Y coordinate for spawning bullets
         * @param baseAngle Base angle for spawning bullets
         */
        void spawnCirclePattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                                PatternSpawnerComponent& spawner, float x, float y, float baseAngle);

        /**
         * @brief Spawn an arc pattern of bullets
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @param spawner Reference to the spawner component
         * @param x X coordinate for spawning bullets
         * @param y Y coordinate for spawning bullets
         * @param baseAngle Base angle for spawning bullets
         */
        void spawnArcPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                             PatternSpawnerComponent& spawner, float x, float y, float baseAngle);

        /**
         * @brief Spawn a spiral pattern of bullets
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @param spawner Reference to the spawner component
         * @param x X coordinate for spawning bullets
         * @param y Y coordinate for spawning bullets
         * @param baseAngle Base angle for spawning bullets
         */
        void spawnSpiralPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                                PatternSpawnerComponent& spawner, float x, float y, float baseAngle);

        /**
         * @brief Spawn a cross pattern of bullets
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @param spawner Reference to the spawner component
         * @param x X coordinate for spawning bullets
         * @param y Y coordinate for spawning bullets
         * @param baseAngle Base angle for spawning bullets
         */
        void spawnCrossPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                               PatternSpawnerComponent& spawner, float x, float y, float baseAngle);

        /**
         * @brief Spawn a star pattern of bullets
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @param spawner Reference to the spawner component
         * @param x X coordinate for spawning bullets
         * @param y Y coordinate for spawning bullets
         * @param baseAngle Base angle for spawning bullets
         */
        void spawnStarPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                              PatternSpawnerComponent& spawner, float x, float y, float baseAngle);

        /**
         * @brief Spawn a grid pattern of bullets
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @param spawner Reference to the spawner component
         * @param x X coordinate for spawning bullets
         * @param y Y coordinate for spawning bullets
         * @param baseAngle Base angle for spawning bullets
         */
        void spawnGridPattern(const PatternWave& wave, const BulletPatternComponent& pattern,
                              PatternSpawnerComponent& spawner, float x, float y, float baseAngle);

        /**
         * @brief Calculate bullet speed including pattern modifiers
         * @param wave Reference to the pattern wave
         * @param pattern Reference to the active bullet pattern
         * @return Final bullet speed
         */
        float calculateBulletSpeed(const PatternWave& wave, const BulletPatternComponent& pattern) const {
            return wave.speed * pattern.globalSpeedMultiplier;
        }

        /**
         * @brief Create and configure a bullet entity
         * @param x X coordinate for spawning the bullet
         * @param y Y coordinate for spawning the bullet
         * @param velX X component of bullet velocity
         * @param velY Y component of bullet velocity
         * @param wave Reference to the pattern wave
         * @param spawner Reference to the spawner component
         */
        void createBullet(float x, float y, float velX, float velY,
                          const PatternWave& wave, PatternSpawnerComponent& spawner);

        /**
         * @brief Add a TrajectoryComponent to a bullet based on the wave's trajectory type
         * @param bullet Reference to the bullet entity
         * @param wave Reference to the pattern wave
         * @param velX X component of the bullet's initial velocity
         * @param velY Y component of the bullet's initial velocity
         * @param x X coordinate where the bullet was spawned
         * @param y Y coordinate where the bullet was spawned
         */
        void addTrajectoryComponent(Entity& bullet, const PatternWave& wave,
                                    float velX, float velY, float x, float y);
    };

} // namespace rtype::ecs