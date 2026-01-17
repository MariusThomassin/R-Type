/*
** R-Type - GameServer
** Headless game simulation for multiplayer server
** Runs ECS at fixed 60 Hz timestep without rendering
*/

#pragma once

#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/core/SystemManager.hpp"
#include "engine/ecs/events/definitions/GameEvents.hpp"
#include "game/systems/LevelConfig.hpp"

#include <memory>
#include <chrono>
#include <iostream>
#include <set>
#include <map>
#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <optional>

#include "shared/network/Protocol.hpp"

// Forward declarations to avoid Raylib dependencies in header
namespace rtype::ecs {
    class MovementSystem;
    class LifetimeSystem;
    class TrajectorySystem;
    class SpinSystem;
    class CollisionSystem;
}

namespace rtype::server {
    class NetworkManager;
    class NetworkIdManager;
    class PlayerManager;

    /**
     * @brief Server-side game simulation
     *
     * Features:
     * - Fixed 60 Hz tick rate (deterministic simulation)
     * - Headless (no rendering, no Raylib dependency)
     * - Reuses client ECS systems (Movement, Bullet, Pattern, Trajectory)
     * - Collision detection
     * - Level-based enemy spawning
     * - Support for single-player mode (local server)
     *
     * Architecture: Quake-style - same server code for solo and multiplayer.
     */
    class GameServer {
    public:
        /**
         * @brief Construct a new Game Server object
         * @param port Server port to listen on (default: 4242)
         * @param allowSinglePlayer If true, game starts with 1 player (for solo mode)
         */
        explicit GameServer(uint16_t port = 4242, bool allowSinglePlayer = false);

        /**
         * @brief Destroy the Game Server object
         */
        ~GameServer();  // Defined in .cpp to allow unique_ptr to incomplete types

        /**
         * @brief Initialize the server (systems, demo entities)
         */
        void initialize();

        /**
         * @brief Run the server game loop
         *
         * Runs at fixed 60 Hz timestep until stopped.
         * Call this from main thread.
         */
        void run();

        /**
         * @brief Stop the server gracefully
         */
        void stop();

        /**
         * @brief Get the current tick count
         * @return uint64_t Number of ticks since start
         */
        uint64_t getTickCount() const { return m_tickCount; }

        /**
         * @brief Get the current game time in seconds
         * @return float Game time
         */
        float getGameTime() const { return m_gameTime; }

        /**
         * @brief Get the number of active entities
         * @return size_t Entity count
         */
        size_t getEntityCount() const;

        /**
         * @brief Check if single-player mode is enabled
         */
        bool isSinglePlayerMode() const { return m_allowSinglePlayer; }

    private:
        /**
         * @brief Initialize all ECS systems
         */
        void initializeSystems();

        /**
         * @brief Update game simulation by one tick
         * @param dt Fixed delta time (1/60 seconds)
         */
        void tick(float dt);

        /**
         * @brief Log server status periodically
         */
        void logStatus();

        /**
         * @brief Handle player-projectile collision events
         * @param event Collision event data
         */
        void handlePlayerCollision(const ecs::CollisionEvent& event);

        /**
         * @brief Handle player death (health <= 0)
         * @param playerEntity Player entity that died
         */
        void handlePlayerDeath(ecs::EntityId playerEntity);

        /**
         * @brief Update pending respawns
         * @param dt Delta time
         */
        void updateRespawns(float dt);

        /**
         * @brief Respawn a player
         * @param clientId Client ID to respawn
         */
        void respawnPlayer(uint32_t clientId);

        /**
         * @brief Check game over condition
         */
        void checkGameOver();

        /**
         * @brief Handle Force orb spawn/upgrade request
         * @param event Spawn event data
         */
        void handleForceOrbSpawn(const ecs::events::SpawnForceOrb& event);

        /**
         * @brief Handle bomb activation
         * @param event Bomb event data
         */
        void handleBombActivation(const ecs::events::BombActivated& event);

        // ============================================================
        // Level Management
        // ============================================================

        /**
         * @brief Load a level configuration by index
         * @param levelIndex 0-based level index
         * @return true if loaded successfully
         */
        bool loadLevel(uint8_t levelIndex);

        /**
         * @brief Start the current level (begin spawning)
         */
        void startLevel();

        /**
         * @brief Handle wave completion
         */
        void handleWaveComplete(int waveNumber);

        /**
         * @brief Handle level completion
         */
        void handleLevelComplete();

        /**
         * @brief Advance to next level (or loop)
         */
        void advanceToNextLevel();

        /**
         * @brief Broadcast level info to all clients
         */
        void broadcastLevelInfo();

        /**
         * @brief Broadcast wave start to all clients
         */
        void broadcastWaveStart(uint8_t waveNumber, uint8_t enemyCount);

        /**
         * @brief Update level waves - spawn enemies based on level config
         */
        void updateLevelWaves(float dt);

        /**
         * @brief Spawn an enemy entity based on config
         */
        void spawnEnemy(const ecs::EnemySpawnConfig& config);

        /**
         * @brief Update player score and broadcast
         */
        void updatePlayerScore(uint32_t clientId, int32_t delta, network::ScoreReason reason);

    private:
        /**
         * @brief State for tracking dead players waiting to respawn
         */
        struct RespawnState {
            uint32_t clientId;
            float timeUntilRespawn;
            uint8_t playerSlot;

            RespawnState(uint32_t id, float delay, uint8_t slot)
                : clientId(id), timeUntilRespawn(delay), playerSlot(slot) {}
        };

        // ECS Core
        ecs::Registry m_registry;
        ecs::EventBus m_eventBus;
        ecs::SystemManager m_systemManager;

        // Systems (owned by SystemManager, stored as raw pointers for access)
        ecs::MovementSystem* m_movementSystem;
        ecs::LifetimeSystem* m_lifetimeSystem;
        ecs::TrajectorySystem* m_trajectorySystem;
        ecs::SpinSystem* m_spinSystem;
        ecs::CollisionSystem* m_collisionSystem;

        // Network (owned by GameServer)
        std::unique_ptr<NetworkManager> m_networkManager;
        std::unique_ptr<NetworkIdManager> m_networkIdManager;
        std::unique_ptr<PlayerManager> m_playerManager;

        // Game state
        bool m_running;
        uint64_t m_tickCount;
        float m_gameTime;

        // Constants
        static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
        static constexpr float LOG_INTERVAL = 5.0f; // Log status every 5 seconds
        static constexpr float RESPAWN_DELAY = 3.0f; // Respawn delay in seconds
        static constexpr int SCREEN_WIDTH = 1280;
        static constexpr int SCREEN_HEIGHT = 720;
        static constexpr int MAX_LEVELS = 3; // Number of available levels

        // Server configuration
        uint16_t m_port;
        bool m_allowSinglePlayer;  // Allow game to start with 1 player

        // Status logging
        float m_logTimer;
        std::atomic<bool> m_gameStarted;  // True when clients ready and game can spawn

        // Track which clients are ready (clicked Play)
        std::set<uint32_t> m_readyClients;

        // Thread synchronization for m_readyClients
        mutable std::mutex m_readyClientsMutex;

        // Event subscriptions
        ecs::EventBus::SubscriberId m_collisionSubId;

        // Respawn system
        std::vector<RespawnState> m_pendingRespawns;
        std::atomic<bool> m_gameOver{false};

        // ============================================================
        // Level/Progression State
        // ============================================================
        
        uint8_t m_currentLevelIndex = 0;    // Current level (0-based)
        uint8_t m_loopCount = 0;            // Current loop (0 = first playthrough)
        float m_difficultyMultiplier = 1.0f; // Difficulty scaling
        bool m_levelActive = false;          // Is a level currently being played
        
        // Level file paths
        std::vector<std::string> m_levelPaths = {
            "config/levels/level1.json",
            "config/levels/level2.json", 
            "config/levels/level3.json"
        };

        // Level wave state
        std::optional<ecs::LevelConfig> m_currentLevelConfig;  // Loaded level config
        size_t m_currentWaveIndex = 0;           // Current wave being spawned
        float m_waveTimer = 0.0f;                // Time since wave started
        float m_levelTimer = 0.0f;               // Time since level started
        size_t m_enemiesSpawnedInWave = 0;       // Enemies spawned in current wave
        float m_enemySpawnTimer = 0.0f;          // Timer for sequential enemy spawns
        bool m_waveActive = false;               // Is a wave currently spawning
        size_t m_enemiesAlive = 0;               // Track alive enemies for wave completion

        // Player scores (clientId → score)
        std::map<uint32_t, uint32_t> m_playerScores;
        uint32_t m_teamScore = 0;
    };

} // namespace rtype::server
