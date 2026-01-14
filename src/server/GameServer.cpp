/*
** R-Type - GameServer Implementation
** Headless game simulation with demo projectile spawning
*/

#include "GameServer.hpp"

// Network includes
#include "NetworkManager.hpp"
#include "NetworkIdManager.hpp"
#include "PlayerManager.hpp"
#include "shared/network/Protocol.hpp"

// System includes (only in .cpp to avoid Raylib dependencies in header)
#include "engine/ecs/systems/MovementSystem.hpp"
#include "engine/ecs/systems/LifetimeSystem.hpp"
// Note: BulletSystem and PatternSystem removed from server (not needed, server spawns manually)
// Both use SpritesheetComponent which causes Raylib linking errors
#include "game/systems/TrajectorySystem.hpp"
#include "game/systems/SpinSystem.hpp"
#include "game/systems/CollisionSystem.hpp"

// Component includes
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "engine/ecs/components/NetworkComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/components/PlayerComponent.hpp"
// Note: SpritesheetComponent removed - not needed on headless server
#include "game/components/bullets/TrajectoryComponent.hpp"
#include "game/components/bullets/SpinComponent.hpp"

// Event includes
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/definitions/GameEvents.hpp"

// Powerup components
#include "game/components/ForceOrbComponent.hpp"
#include "game/components/PowerupComponent.hpp"

#include <thread>
#include <cmath>
#include <ctime>
#include <cstdlib>

namespace rtype::server {

    GameServer::GameServer(uint16_t port)
        : m_registry()
        , m_eventBus()
        , m_systemManager(&m_registry)  // SystemManager takes a pointer
        , m_movementSystem(nullptr)
        , m_lifetimeSystem(nullptr)
        , m_trajectorySystem(nullptr)
        , m_spinSystem(nullptr)
        , m_collisionSystem(nullptr)
        , m_networkManager(nullptr)
        , m_networkIdManager(nullptr)
        , m_playerManager(nullptr)
        , m_running(false)
        , m_tickCount(0)
        , m_gameTime(0.0f)
        , m_demoSpawnTimer(0.0f)
        , m_demoSpawnCounter(0)
        , m_logTimer(0.0f)
        , m_gameStarted{false}
        , m_port(port)
    {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }

    // Destructor defined here to allow unique_ptr to incomplete types in header
    GameServer::~GameServer() = default;

    void GameServer::initialize() {
        std::cout << "[GameServer] Initializing headless game simulation..." << std::endl;

        // Initialize network managers
        m_networkIdManager = std::make_unique<NetworkIdManager>();
        m_networkManager = std::make_unique<NetworkManager>(m_registry, m_port);
        m_playerManager = std::make_unique<PlayerManager>(m_registry, *m_networkManager, *m_networkIdManager);

        initializeSystems();

        // Set up network callbacks for player management
        m_networkManager->setOnClientConnected([this](uint32_t clientId) {
            std::cout << "[GameServer] Client connected: " << clientId << ", spawning player..." << std::endl;
            m_playerManager->spawnPlayer(clientId);
        });

        m_networkManager->setOnClientDisconnected([this](uint32_t clientId) {
            std::cout << "[GameServer] Client disconnected: " << clientId << ", removing player..." << std::endl;
            m_playerManager->removePlayer(clientId);
        });

        m_networkManager->setOnClientInput([this](uint32_t clientId, const network::ClientInputMessage& input) {
            m_playerManager->applyInput(clientId, input);
        });

        m_networkManager->setOnPlayerReady([this](uint32_t clientId) {
            std::cout << "[GameServer] Client " << clientId << " is ready!" << std::endl;
            
            // Get player count outside mutex to avoid potential lock ordering issues
            // Note: There's a potential race if a player disconnects between this call
            // and the check below, but this is a safe race (we might just delay game start)
            size_t playerCount = m_playerManager->getPlayerCount();
            
            {
                std::lock_guard<std::mutex> lock(m_readyClientsMutex);
                m_readyClients.insert(clientId);
                size_t readyCount = m_readyClients.size();

                // Check if game should start (2+ players, all ready)
                // Double-check m_gameStarted inside the lock to avoid race condition
                if (!m_gameStarted.load() && playerCount >= 2 && readyCount >= playerCount) {
                    m_gameStarted.store(true);
                    std::cout << "[GameServer] Game started! " << readyCount << " players ready." << std::endl;
                }
            }
        });

        // Subscribe to collision events for player-projectile damage
        m_collisionSubId = m_eventBus.subscribe<ecs::CollisionEvent>(
            [this](const ecs::CollisionEvent& event) {
                handlePlayerCollision(event);
            }
        );

        // Subscribe to Force Orb spawn events
        m_eventBus.subscribe<ecs::events::SpawnForceOrb>(
            [this](const ecs::events::SpawnForceOrb& event) {
                handleForceOrbSpawn(event);
            }
        );

        // Subscribe to Bomb activation events
        m_eventBus.subscribe<ecs::events::BombActivated>(
            [this](const ecs::events::BombActivated& event) {
                handleBombActivation(event);
            }
        );

        // Start network
        m_networkManager->start();

        std::cout << "[GameServer] Initialized successfully!" << std::endl;
        std::cout << "[GameServer] - Fixed timestep: " << FIXED_TIMESTEP << "s (60 Hz)" << std::endl;
        std::cout << "[GameServer] - Screen size: " << SCREEN_WIDTH << "x" << SCREEN_HEIGHT << std::endl;
        std::cout << "[GameServer] - Demo spawn interval: " << DEMO_SPAWN_INTERVAL << "s" << std::endl;
    }

    void GameServer::initializeSystems() {
        // Create systems in execution order
        // Phase order: Input → Physics → Collision → GameLogic → Render

        // Physics phase: Movement and Trajectories
        m_movementSystem = m_systemManager.addSystem<ecs::MovementSystem>();
        m_trajectorySystem = m_systemManager.addSystem<ecs::TrajectorySystem>();
        m_spinSystem = m_systemManager.addSystem<ecs::SpinSystem>();

        // Collision phase
        m_collisionSystem = m_systemManager.addSystem<ecs::CollisionSystem>(m_eventBus);

        // GameLogic phase: Lifetime
        // Note: BulletSystem and PatternSystem not needed - server spawns projectiles manually
        ecs::LifetimeSystemConfig lifetimeConfig;
        lifetimeConfig.screenWidth = SCREEN_WIDTH;
        lifetimeConfig.screenHeight = SCREEN_HEIGHT;
        lifetimeConfig.offscreenMargin = 100.0f;
        lifetimeConfig.destroyOffscreen = true;
        m_lifetimeSystem = m_systemManager.addSystem<ecs::LifetimeSystem>(lifetimeConfig);

        // Configure systems with screen size
        m_lifetimeSystem->setScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);

        std::cout << "[GameServer] Registered " << m_systemManager.getSystemCount() << " systems" << std::endl;
    }

    void GameServer::run() {
        m_running = true;
        m_tickCount = 0;
        m_gameTime = 0.0f;

        std::cout << "[GameServer] Starting game loop at 60 Hz..." << std::endl;

        using Clock = std::chrono::high_resolution_clock;
        using Duration = std::chrono::duration<float>;

        auto lastTime = Clock::now();
        float accumulator = 0.0f;

        while (m_running) {
            auto currentTime = Clock::now();
            float frameTime = std::chrono::duration_cast<Duration>(currentTime - lastTime).count();
            lastTime = currentTime;

            // Cap frame time to prevent spiral of death
            if (frameTime > 0.25f) {
                frameTime = 0.25f;
            }

            accumulator += frameTime;

            // Fixed timestep updates
            while (accumulator >= FIXED_TIMESTEP) {
                tick(FIXED_TIMESTEP);
                accumulator -= FIXED_TIMESTEP;
            }

            // Sleep to avoid burning CPU (target 60 FPS)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        std::cout << "[GameServer] Game loop stopped." << std::endl;
    }

    void GameServer::tick(float dt) {
        m_tickCount++;
        m_gameTime += dt;
        m_demoSpawnTimer += dt;
        m_logTimer += dt;

        // Don't update game if game over
        if (m_gameOver.load()) {
            return;
        }

        // Update network (send state updates at 20 Hz)
        m_networkManager->update(dt);

        // Update player manager (clamp positions, etc.)
        m_playerManager->update(dt);

        // Update pending respawns
        updateRespawns(dt);

        // Spawn demo projectiles periodically (only if game has started)
        if (m_gameStarted.load() && m_demoSpawnTimer >= DEMO_SPAWN_INTERVAL) {
            spawnDemoProjectiles();
            m_demoSpawnTimer = 0.0f;
        }

        // Update all systems (SystemManager handles phase ordering)
        m_systemManager.updateAll(dt);

        // Log status periodically
        if (m_logTimer >= LOG_INTERVAL) {
            logStatus();
            m_logTimer = 0.0f;
        }
    }

    void GameServer::spawnDemoProjectiles() {
        m_demoSpawnCounter++;

        std::cout << "[GameServer] Spawning demo projectiles (batch #" << m_demoSpawnCounter << ")..." << std::endl;

        // Define spawn positions (spread across screen)
        struct SpawnDef {
            float x, y;
            float velX, velY;
            ecs::TrajectoryType trajectory;
            const char* name;
        };

        SpawnDef spawns[] = {
            // Linear (left to right) - Moved away from player corners
            {50.0f, 250.0f, 300.0f, 0.0f, ecs::TrajectoryType::Linear, "Linear"},

            // Sinusoidal (wavy motion) - Safe center-left position
            {50.0f, 350.0f, 250.0f, 0.0f, ecs::TrajectoryType::Sinusoidal, "Sinusoidal"},

            // Spiral (expanding spiral) - Center of screen
            {640.0f, 360.0f, 200.0f, 0.0f, ecs::TrajectoryType::Spiral, "Spiral"},

            // Circular (orbit motion) - Center-right
            {900.0f, 400.0f, 150.0f, 0.0f, ecs::TrajectoryType::Circular, "Circular"},

            // Zigzag (sharp turns) - Bottom center-left, away from corners
            {300.0f, 500.0f, 280.0f, 0.0f, ecs::TrajectoryType::Zigzag, "Zigzag"},

            // Figure8 (infinity symbol) - Right side center
            {1000.0f, 360.0f, 100.0f, 0.0f, ecs::TrajectoryType::Figure8, "Figure8"},

            // Pendulum (swinging) - Bottom center
            {500.0f, 600.0f, 200.0f, 0.0f, ecs::TrajectoryType::Pendulum, "Pendulum"},

            // Whip (accelerate then decelerate) - Center top, away from corners
            {640.0f, 50.0f, 150.0f, 100.0f, ecs::TrajectoryType::Whip, "Whip"}
        };

        for (const auto& spawn : spawns) {
            ecs::Entity bullet = m_registry.createEntity();

            // Transform
            m_registry.addComponent(bullet, ecs::TransformComponent(spawn.x, spawn.y));

            // Velocity
            m_registry.addComponent(bullet, ecs::VelocityComponent(spawn.velX, spawn.velY, 500.0f));

            // Projectile (use constructor instead of designated initializers)
            m_registry.addComponent(bullet, ecs::ProjectileComponent(ecs::NULL_ENTITY, 10, false));

            // Collider (for collision detection)
            ecs::ColliderComponent collider;
            collider.width = 16.0f;
            collider.height = 16.0f;
            collider.layer = ecs::CollisionLayer::EnemyShot;
            collider.mask = static_cast<ecs::CollisionLayer>(
                static_cast<uint32_t>(ecs::CollisionLayer::Player) |
                static_cast<uint32_t>(ecs::CollisionLayer::Wall)
            );
            m_registry.addComponent(bullet, collider);

            // Trajectory (if not Linear)
            if (spawn.trajectory != ecs::TrajectoryType::Linear) {
                ecs::TrajectoryComponent traj;
                traj.type = spawn.trajectory;
                traj.initialized = false;
                // Note: TrajectoryComponent uses named parameters, not param1/param2
                // The trajectory updaters will initialize parameters automatically
                m_registry.addComponent(bullet, traj);
            }

            // Spin (set spinSpeed after default construction)
            ecs::SpinComponent spin;
            spin.spinSpeed = 90.0f;  // 90 degrees per second
            m_registry.addComponent(bullet, spin);

            // Lifetime (auto-destroy after 10 seconds)
            m_registry.addComponent(bullet, ecs::LifetimeComponent(10.0f));

            // Network - Allocate network ID and add NetworkComponent
            uint32_t networkId = m_networkIdManager->allocate(bullet);
            m_registry.addComponent(bullet, ecs::NetworkComponent(networkId, false));

            // Send ENTITY_SPAWN to all clients
            network::EntitySpawnMessage spawnMsg{};
            spawnMsg.networkId = networkId;
            spawnMsg.entityType = network::EntityType::PROJECTILE;
            spawnMsg.x = spawn.x;
            spawnMsg.y = spawn.y;
            spawnMsg.rotation = 0.0f;
            spawnMsg.vx = spawn.velX;
            spawnMsg.vy = spawn.velY;
            spawnMsg.trajectoryType = static_cast<uint8_t>(spawn.trajectory);
            spawnMsg.trajectoryParam1 = 0.0f;  // Will be initialized by trajectory system
            spawnMsg.trajectoryParam2 = 0.0f;
            spawnMsg.spinSpeed = 90.0f;
            spawnMsg.maxLifetime = 10.0f;
            spawnMsg.colliderWidth = 16.0f;
            spawnMsg.colliderHeight = 16.0f;
            spawnMsg.collisionLayer = static_cast<uint32_t>(ecs::CollisionLayer::EnemyShot);
            spawnMsg.collisionMask = static_cast<uint32_t>(ecs::CollisionLayer::Player) |
                                     static_cast<uint32_t>(ecs::CollisionLayer::Wall);

            m_networkManager->broadcastEntitySpawn(spawnMsg);

            std::cout << "  - Spawned " << spawn.name << " bullet at ("
                      << spawn.x << ", " << spawn.y << ") [networkId=" << networkId << "]" << std::endl;
        }

        std::cout << "[GameServer] Spawned " << sizeof(spawns) / sizeof(spawns[0])
                  << " demo projectiles" << std::endl;
    }

    void GameServer::stop() {
        std::cout << "[GameServer] Stopping server..." << std::endl;
        m_running = false;

        // Stop network
        if (m_networkManager) {
            m_networkManager->stop();
        }
    }

    size_t GameServer::getEntityCount() const {
        size_t count = 0;
        // Cast away const to call forEach (Registry::forEach should be const but isn't)
        const_cast<ecs::Registry&>(m_registry).forEach<ecs::TransformComponent>([&count](ecs::EntityId) {
            count++;
        });
        return count;
    }

    void GameServer::logStatus() {
        std::cout << "[GameServer] Status Report:" << std::endl;
        std::cout << "  - Tick: " << m_tickCount << std::endl;
        std::cout << "  - Game Time: " << m_gameTime << "s" << std::endl;
        std::cout << "  - Active Entities: " << getEntityCount() << std::endl;
        std::cout << "  - FPS: ~60 (fixed timestep)" << std::endl;
    }

    void GameServer::handlePlayerCollision(const ecs::CollisionEvent& event) {
        // Determine which entity is the player and which is the projectile
        ecs::EntityId playerEntityId = ecs::NULL_ENTITY;
        ecs::EntityId projectileEntityId = ecs::NULL_ENTITY;

        auto* playerA = m_registry.tryGetComponent<ecs::PlayerComponent>(event.entityA);
        auto* playerB = m_registry.tryGetComponent<ecs::PlayerComponent>(event.entityB);
        auto* projectileA = m_registry.tryGetComponent<ecs::ProjectileComponent>(event.entityA);
        auto* projectileB = m_registry.tryGetComponent<ecs::ProjectileComponent>(event.entityB);

        if (playerA && projectileB) {
            playerEntityId = event.entityA;
            projectileEntityId = event.entityB;
        } else if (playerB && projectileA) {
            playerEntityId = event.entityB;
            projectileEntityId = event.entityA;
        } else {
            // Not a player-projectile collision, ignore
            return;
        }

        // Get components
        auto* health = m_registry.tryGetComponent<ecs::HealthComponent>(playerEntityId);
        auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(playerEntityId);
        auto* network = m_registry.tryGetComponent<ecs::NetworkComponent>(playerEntityId);
        auto* projectile = m_registry.tryGetComponent<ecs::ProjectileComponent>(projectileEntityId);

        if (!health || !transform || !network || !projectile) {
            return; // Missing components
        }

        // Apply damage
        int damage = projectile->damage;
        health->currentHealth -= damage;
        if (health->currentHealth < 0) {
            health->currentHealth = 0;
        }

        std::cout << "[GameServer] Player hit! NetworkID=" << network->networkId
                  << " Health: " << health->currentHealth << "/" << health->maxHealth << std::endl;

        // Broadcast PLAYER_HIT message to all clients
        network::PlayerHitMessage hitMsg;
        hitMsg.networkId = network->networkId;
        hitMsg.newHealth = static_cast<float>(health->currentHealth);
        hitMsg.hitX = transform->x;
        hitMsg.hitY = transform->y;

        auto buffer = network::serializeMessage(network::MessageType::PLAYER_HIT, hitMsg);
        m_networkManager->broadcast(buffer);

        // Remove projectile from network ID manager before destroying
        auto* projectileNetwork = m_registry.tryGetComponent<ecs::NetworkComponent>(projectileEntityId);
        if (projectileNetwork) {
            ecs::Entity projectileEntity(projectileEntityId);
            m_networkIdManager->remove(projectileEntity);
            m_networkManager->broadcastEntityDestroy(projectileNetwork->networkId);
        }

        // Destroy the projectile
        m_registry.destroyEntity(projectileEntityId);

        // Handle player death
        if (health->currentHealth <= 0) {
            handlePlayerDeath(playerEntityId);
        }
    }

    void GameServer::handlePlayerDeath(ecs::EntityId playerEntityId) {
        auto* player = m_registry.tryGetComponent<ecs::PlayerComponent>(playerEntityId);
        auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(playerEntityId);
        auto* network = m_registry.tryGetComponent<ecs::NetworkComponent>(playerEntityId);
        auto* collider = m_registry.tryGetComponent<ecs::ColliderComponent>(playerEntityId);

        if (!player || !transform || !network) {
            return;
        }

        // Lose a life
        bool hasLivesRemaining = player->loseLife();

        std::cout << "[GameServer] Player died! NetworkID=" << network->networkId
                  << " Lives remaining: " << player->lives << std::endl;

        // Remove collider to prevent further collisions while dead
        if (collider) {
            m_registry.removeComponent<ecs::ColliderComponent>(playerEntityId);
        }

        // Broadcast PLAYER_DEATH message
        network::PlayerDeathMessage deathMsg;
        deathMsg.networkId = network->networkId;
        deathMsg.remainingLives = static_cast<uint8_t>(player->lives);
        deathMsg.deathX = transform->x;
        deathMsg.deathY = transform->y;

        auto buffer = network::serializeMessage(network::MessageType::PLAYER_DEATH, deathMsg);
        m_networkManager->broadcast(buffer);

        if (hasLivesRemaining) {
            // Add to respawn queue
            uint32_t clientId = player->networkClientId;
            m_pendingRespawns.emplace_back(clientId, RESPAWN_DELAY, player->slot);
            std::cout << "[GameServer] Player will respawn in " << RESPAWN_DELAY << " seconds" << std::endl;
        } else {
            std::cout << "[GameServer] Player has no lives remaining!" << std::endl;
            // Remove from player manager (disconnects the player effectively)
            m_playerManager->removePlayer(player->networkClientId);
            checkGameOver();
        }
    }

    void GameServer::updateRespawns(float dt) {
        if (m_pendingRespawns.empty()) {
            return;
        }

        // Update timers
        for (auto& respawn : m_pendingRespawns) {
            respawn.timeUntilRespawn -= dt;
        }

        // Respawn players whose timer has expired
        auto it = m_pendingRespawns.begin();
        while (it != m_pendingRespawns.end()) {
            if (it->timeUntilRespawn <= 0.0f) {
                respawnPlayer(it->clientId);
                it = m_pendingRespawns.erase(it);
            } else {
                ++it;
            }
        }
    }

    void GameServer::respawnPlayer(uint32_t clientId) {
        std::cout << "[GameServer] Respawning player for client " << clientId << std::endl;

        // Spawn new player entity
        ecs::Entity newPlayer = m_playerManager->spawnPlayer(clientId);
        if (newPlayer.id == ecs::NULL_ENTITY) {
            std::cout << "[GameServer] Failed to respawn player for client " << clientId << std::endl;
            return;
        }

        // Get components
        auto* network = m_registry.tryGetComponent<ecs::NetworkComponent>(newPlayer);
        auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(newPlayer);
        auto* playerComp = m_registry.tryGetComponent<ecs::PlayerComponent>(newPlayer);

        if (!network || !transform || !playerComp) {
            return;
        }

        // Broadcast PLAYER_RESPAWN message
        network::PlayerRespawnMessage respawnMsg;
        respawnMsg.networkId = network->networkId;
        respawnMsg.playerSlot = playerComp->slot;
        respawnMsg.x = transform->x;
        respawnMsg.y = transform->y;
        respawnMsg.health = 100.0f;

        auto buffer = network::serializeMessage(network::MessageType::PLAYER_RESPAWN, respawnMsg);
        m_networkManager->broadcast(buffer);

        std::cout << "[GameServer] Player respawned at (" << transform->x << ", " << transform->y << ")" << std::endl;
    }

    void GameServer::checkGameOver() {
        if (m_gameOver.load()) {
            return;
        }

        size_t playerCount = m_playerManager->getPlayerCount();
        if (playerCount == 0) {
            m_gameOver.store(true);
            std::cout << "[GameServer] GAME OVER! All players are dead." << std::endl;

            // Broadcast GAME_OVER message
            network::GameOverMessage gameOverMsg;
            gameOverMsg.survivorCount = 0;

            auto buffer = network::serializeMessage(network::MessageType::GAME_OVER, gameOverMsg);
            m_networkManager->broadcast(buffer);
        }
    }

    void GameServer::handleForceOrbSpawn(const ecs::events::SpawnForceOrb& event) {
        std::cout << "[GameServer] Force orb spawn requested for player " << event.playerId 
                  << " (level " << event.level << ")" << std::endl;

        // Check if player already has an orb
        ecs::Entity existingOrb{ecs::NULL_ENTITY};
        auto orbEntities = m_registry.getEntitiesWith<ecs::ForceOrbComponent>();
        for (ecs::EntityId eid : orbEntities) {
            auto* orb = m_registry.tryGetComponent<ecs::ForceOrbComponent>(eid);
            if (orb && orb->ownerId == event.playerId) {
                existingOrb = ecs::Entity{eid};
                break;
            }
        }

        if (existingOrb.id != ecs::NULL_ENTITY) {
            // Upgrade existing orb
            auto* orbComp = m_registry.tryGetComponent<ecs::ForceOrbComponent>(existingOrb);
            if (orbComp && orbComp->level < 3) {
                int oldLevel = orbComp->level;
                orbComp->level = std::min(orbComp->level + 1, 3);
                orbComp->updateForLevel();
                std::cout << "[GameServer] Upgraded Force orb from level " << oldLevel 
                          << " to " << orbComp->level << std::endl;

                // Emit upgrade event
                m_eventBus.emit(ecs::events::ForceOrbUpgraded{
                    existingOrb.id, event.playerId, oldLevel, orbComp->level
                });

                // TODO: Broadcast orb upgrade to clients
            }
        } else {
            // Spawn new Force orb
            ecs::Entity orb = m_registry.createEntity();
            
            // Get player position for orb placement
            auto* playerTransform = m_registry.tryGetComponent<ecs::TransformComponent>(
                ecs::Entity{event.playerId});
            float orbX = 100.0f, orbY = 360.0f;  // Default position
            if (playerTransform) {
                orbX = playerTransform->x - 40.0f;  // Left of player
                orbY = playerTransform->y;
            }

            m_registry.addComponent(orb, ecs::TransformComponent(orbX, orbY));
            m_registry.addComponent(orb, ecs::ForceOrbComponent(event.playerId, 
                ecs::OrbDockSide::Left, event.level));

            // Add network component for sync
            uint32_t netId = m_networkIdManager->allocate(orb);
            m_registry.addComponent(orb, ecs::NetworkComponent(netId));

            std::cout << "[GameServer] Spawned Force orb entity " << orb.id 
                      << " with network ID " << netId << std::endl;

            // Emit spawn event
            m_eventBus.emit(ecs::events::ForceOrbSpawned{orb.id, event.playerId, event.level});

            // TODO: Broadcast orb spawn to clients
        }
    }

    void GameServer::handleBombActivation(const ecs::events::BombActivated& event) {
        std::cout << "[GameServer] Bomb activated by player " << event.playerId 
                  << " at (" << event.x << ", " << event.y << ")" << std::endl;

        int enemiesDestroyed = 0;
        int projectilesDestroyed = 0;

        // Destroy all enemy projectiles
        std::vector<ecs::Entity> toDestroy;
        auto projectileEntities = m_registry.getEntitiesWith<ecs::ProjectileComponent>();
        for (ecs::EntityId eid : projectileEntities) {
            auto* proj = m_registry.tryGetComponent<ecs::ProjectileComponent>(eid);
            // Only destroy enemy projectiles (not player's)
            if (proj && !proj->isPlayerProjectile) {
                toDestroy.push_back(ecs::Entity{eid});
                projectilesDestroyed++;
            }
        }

        // Destroy enemies within bomb radius (screen-wide effect)
        auto healthEntities = m_registry.getEntitiesWith<ecs::HealthComponent, ecs::TransformComponent>();
        for (ecs::EntityId eid : healthEntities) {
            // Check if it's an enemy (not a player)
            if (!m_registry.tryGetComponent<ecs::PlayerComponent>(eid)) {
                // Screen-wide bomb effect - destroy all enemies
                toDestroy.push_back(ecs::Entity{eid});
                enemiesDestroyed++;
            }
        }

        // Actually destroy entities
        for (const auto& entity : toDestroy) {
            m_registry.destroyEntity(entity.id);
        }

        std::cout << "[GameServer] Bomb destroyed " << enemiesDestroyed << " enemies and " 
                  << projectilesDestroyed << " projectiles" << std::endl;

        // Emit completion event
        m_eventBus.emit(ecs::events::BombComplete{event.playerId, enemiesDestroyed, projectilesDestroyed});

        // TODO: Broadcast bomb effect to clients for visual feedback
    }

} // namespace rtype::server
