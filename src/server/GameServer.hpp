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
#include "server/LevelParser.hpp"

#include <memory>
#include <chrono>
#include <iostream>
#include <set>
#include <mutex>
#include <atomic>

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

    struct LevelElement {
        float x;
        float y;
        network::EntityType type;
        uint8_t variant;            // (ex: which powerup type)
    };

    struct LevelConfig {
        std::string levelName;
        float width;
        float height;
        std::vector<LevelElement> elements;
    };

    /**
     * @brief Component factory function type
     * Creates components based on EntityType and adds them to an entity
     */
    using ComponentFactory = std::function<void(ecs::Registry&, ecs::Entity, const LevelElement&)>;

    /**
     * @brief Server-side game simulation
     *
     * Features:
     * - Fixed 60 Hz tick rate (deterministic simulation)
     * - Headless (no rendering, no Raylib dependency)
     * - Reuses client ECS systems (Movement, Bullet, Pattern, Trajectory)
     * - Collision detection
     * - Demo projectile spawning for testing
     *
     * Future network sync will be added here.
     */
    class GameServer {
    public:
        /**
         * @brief Construct a new Game Server object
         */
        GameServer();

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

    private:
        /**
         * @brief Initialize all ECS systems
         */
        void initializeSystems();

        /**
         * @brief Spawn demo projectiles periodically
         *
         * Spawns bullets with different trajectories:
         * - Linear
         * - Sinusoidal
         * - Spiral
         * - Homing
         * - Circular
         * - Zigzag
         * - Figure8
         */
        void spawnDemoProjectiles();

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
        float m_demoSpawnTimer;

        // Constants
        static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;
        static constexpr float DEMO_SPAWN_INTERVAL = 2.0f; // Spawn demo bullets every 2 seconds
        static constexpr float LOG_INTERVAL = 5.0f; // Log status every 5 seconds
        static constexpr float RESPAWN_DELAY = 3.0f; // Respawn delay in seconds
        static constexpr int SCREEN_WIDTH = 1280;
        static constexpr int SCREEN_HEIGHT = 720;

        // Demo spawn state
        int m_demoSpawnCounter;
        float m_logTimer;
        std::atomic<bool> m_gameStarted;  // True when 2+ clients ready and projectiles can spawn

        // Track which clients are ready (clicked Play)
        std::set<uint32_t> m_readyClients;

        // Thread synchronization for m_readyClients
        mutable std::mutex m_readyClientsMutex;

        // Event subscriptions
        ecs::EventBus::SubscriberId m_collisionSubId;

        // Respawn system
        std::vector<RespawnState> m_pendingRespawns;
        std::atomic<bool> m_gameOver{false};

        /**
         * @brief Load level configuration and spawn entities
         * @param config Level configuration data
         */
        void loadLevelFromConfig(const LevelConfig& config);

        /**
         * @brief Default level path
         */
        std::string defaultLevelPath = "assets/levels/simple_test.txt";

        /**
         * @brief Component factory mapping for dynamic component creation
         */
        std::unordered_map<network::EntityType, ComponentFactory> m_componentFactories;

        /**
         * @brief Initialize component factory mappings
         */
        void initializeComponentFactories();

        /**
         * @brief Create EntitySpawnMessage for broadcasting to clients
         */
        network::EntitySpawnMessage createEntitySpawnMessage(ecs::Entity entity, const LevelElement& element, uint32_t networkId);
    };
} // namespace rtype::server
