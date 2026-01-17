/*
** R-Type - GameServer Implementation
** Headless game simulation with level-based enemy spawning
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
#include "game/systems/TrajectorySystem.hpp"
#include "game/systems/SpinSystem.hpp"
#include "game/systems/CollisionSystem.hpp"
#include "game/systems/LevelLoader.hpp"

// Component includes
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "engine/ecs/components/NetworkComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/EnemyComponent.hpp"
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
#include <cstring>
#include <cstdio>

namespace rtype::server {

    GameServer::GameServer(uint16_t port, bool allowSinglePlayer)
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
        , m_logTimer(0.0f)
        , m_gameStarted{false}
        , m_port(port)
        , m_allowSinglePlayer(allowSinglePlayer)
    {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        
        if (m_allowSinglePlayer) {
            std::cout << "[GameServer] Single-player mode enabled" << std::endl;
        }
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
            std::cout << "[GameServer] Client connected: " << clientId << std::endl;
            // Note: Player spawning now happens when game starts via host
            // For now, spawn player immediately for compatibility
            m_playerManager->spawnPlayer(clientId);
        });

        m_networkManager->setOnClientDisconnected([this](uint32_t clientId) {
            std::cout << "[GameServer] Client disconnected: " << clientId << ", removing player..." << std::endl;
            m_playerManager->removePlayer(clientId);
            
            // Remove from ready clients
            {
                std::lock_guard<std::mutex> lock(m_readyClientsMutex);
                m_readyClients.erase(clientId);
            }
        });

        m_networkManager->setOnClientInput([this](uint32_t clientId, const network::ClientInputMessage& input) {
            m_playerManager->applyInput(clientId, input);
        });

        m_networkManager->setOnPlayerReady([this](uint32_t clientId) {
            std::cout << "[GameServer] Client " << clientId << " is ready!" << std::endl;
            
            // Initialize player score
            m_playerScores[clientId] = 0;
            
            {
                std::lock_guard<std::mutex> lock(m_readyClientsMutex);
                m_readyClients.insert(clientId);
            }

            // Update room manager ready state
            m_networkManager->getRoomManager().setPlayerReady(clientId, true);

            // In single-player mode (local server), auto-start when ready
            if (m_allowSinglePlayer && !m_gameStarted.load()) {
                m_gameStarted.store(true);
                std::cout << "[GameServer] Single-player mode: Auto-starting game!" << std::endl;
                if (loadLevel(0)) {
                    startLevel();
                }
            }
            // In multiplayer mode, wait for host to start via HOST_START_GAME
        });

        // Set up host start game callback
        m_networkManager->setOnHostStartGame([this](const std::string& roomName, uint32_t hostClientId, uint8_t levelIndex) {
            std::cout << "[GameServer] Host " << hostClientId << " starting game in room '" 
                      << roomName << "' (level " << static_cast<int>(levelIndex) << ")" << std::endl;
            
            if (!m_gameStarted.load()) {
                m_gameStarted.store(true);
                
                // Load and start the level
                if (loadLevel(levelIndex)) {
                    startLevel();
                } else {
                    std::cerr << "[GameServer] Failed to load level " << static_cast<int>(levelIndex) << std::endl;
                    // Fallback to level 0
                    if (loadLevel(0)) {
                        startLevel();
                    }
                }
            }
        });

        // Subscribe to collision events for damage handling
        m_collisionSubId = m_eventBus.subscribe<ecs::CollisionEvent>(
            [this](const ecs::CollisionEvent& event) {
                handlePlayerCollision(event);
                handleEnemyCollision(event);
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
        m_logTimer += dt;

        // Don't update game if game over
        if (m_gameOver.load()) {
            return;
        }

        // Update network (send state updates at 20 Hz)
        m_networkManager->update(dt);

        // Update player manager (clamp positions, etc.)
        m_playerManager->update(dt);

        // Update player invincibility timers
        m_registry.forEach<ecs::PlayerComponent, ecs::HealthComponent>(
            [this, dt](ecs::EntityId e) {
                auto& health = m_registry.getComponent<ecs::HealthComponent>(e);
                if (health.isInvincible && health.invincibilityTimer > 0.0f) {
                    health.invincibilityTimer -= dt;
                    if (health.invincibilityTimer <= 0.0f) {
                        health.isInvincible = false;
                        health.invincibilityTimer = 0.0f;
                    }
                }
            }
        );

        // Update pending respawns
        updateRespawns(dt);

        // Update level waves (spawn enemies based on level config)
        if (m_gameStarted.load() && m_levelActive) {
            updateLevelWaves(dt);
            updatePowerupSpawns(dt);
            updateEnemies(dt);
        }

        // Update all systems (SystemManager handles phase ordering)
        m_systemManager.updateAll(dt);

        // Log status periodically
        if (m_logTimer >= LOG_INTERVAL) {
            logStatus();
            m_logTimer = 0.0f;
        }
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
        std::cout << "  - Level Timer: " << m_levelTimer << "s" << std::endl;
        std::cout << "  - Active Entities: " << getEntityCount() << std::endl;
        if (m_currentLevelConfig) {
            std::cout << "  - Powerup spawns: " << m_currentLevelConfig->powerupSpawns.size() 
                      << ", Bomb spawns: " << m_currentLevelConfig->bombSpawns.size() << std::endl;
        }
        std::cout << "  - FPS: ~60 (fixed timestep)" << std::endl;
    }

    void GameServer::handlePlayerCollision(const ecs::CollisionEvent& event) {
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
            return;
        }

        auto* health = m_registry.tryGetComponent<ecs::HealthComponent>(playerEntityId);
        auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(playerEntityId);
        auto* network = m_registry.tryGetComponent<ecs::NetworkComponent>(playerEntityId);
        auto* projectile = m_registry.tryGetComponent<ecs::ProjectileComponent>(projectileEntityId);

        if (!health || !transform || !network || !projectile) {
            return;
        }

        // Check invincibility
        if (health->isInvincible) {
            // Destroy projectile but don't damage player
            auto* projectileNetwork = m_registry.tryGetComponent<ecs::NetworkComponent>(projectileEntityId);
            if (projectileNetwork) {
                ecs::Entity projectileEntity(projectileEntityId);
                m_networkIdManager->remove(projectileEntity);
                m_networkManager->broadcastEntityDestroy(projectileNetwork->networkId);
            }
            m_registry.destroyEntity(projectileEntityId);
            return;
        }

        std::cout << "[GameServer] Player hit! NetworkID=" << network->networkId
                  << " - losing a life!" << std::endl;

        // Destroy projectile
        auto* projectileNetwork = m_registry.tryGetComponent<ecs::NetworkComponent>(projectileEntityId);
        if (projectileNetwork) {
            ecs::Entity projectileEntity(projectileEntityId);
            m_networkIdManager->remove(projectileEntity);
            m_networkManager->broadcastEntityDestroy(projectileNetwork->networkId);
        }
        m_registry.destroyEntity(projectileEntityId);

        // One hit = one death (lose one life)
        handlePlayerDeath(playerEntityId);
    }

    void GameServer::handlePlayerDeath(ecs::EntityId playerEntityId) {
        auto* player = m_registry.tryGetComponent<ecs::PlayerComponent>(playerEntityId);
        auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(playerEntityId);
        auto* network = m_registry.tryGetComponent<ecs::NetworkComponent>(playerEntityId);
        auto* collider = m_registry.tryGetComponent<ecs::ColliderComponent>(playerEntityId);

        if (!player || !transform || !network) {
            return;
        }

        bool hasLivesRemaining = player->loseLife();

        std::cout << "[GameServer] Player died! NetworkID=" << network->networkId
                  << " Lives remaining: " << player->lives << std::endl;

        if (collider) {
            m_registry.removeComponent<ecs::ColliderComponent>(playerEntityId);
        }

        network::PlayerDeathMessage deathMsg;
        deathMsg.networkId = network->networkId;
        deathMsg.remainingLives = static_cast<uint8_t>(player->lives);
        deathMsg.deathX = transform->x;
        deathMsg.deathY = transform->y;

        auto buffer = network::serializeMessage(network::MessageType::PLAYER_DEATH, deathMsg);
        m_networkManager->broadcast(buffer);

        if (hasLivesRemaining) {
            uint32_t clientId = player->networkClientId;
            m_pendingRespawns.emplace_back(clientId, RESPAWN_DELAY, player->slot);
            std::cout << "[GameServer] Player will respawn in " << RESPAWN_DELAY << " seconds" << std::endl;
        } else {
            std::cout << "[GameServer] Player has no lives remaining!" << std::endl;
            m_playerManager->removePlayer(player->networkClientId);
            checkGameOver();
        }
    }

    void GameServer::handleEnemyCollision(const ecs::CollisionEvent& event) {
        ecs::EntityId enemyEntityId = ecs::NULL_ENTITY;
        ecs::EntityId projectileEntityId = ecs::NULL_ENTITY;

        auto* enemyA = m_registry.tryGetComponent<ecs::EnemyComponent>(event.entityA);
        auto* enemyB = m_registry.tryGetComponent<ecs::EnemyComponent>(event.entityB);
        auto* projectileA = m_registry.tryGetComponent<ecs::ProjectileComponent>(event.entityA);
        auto* projectileB = m_registry.tryGetComponent<ecs::ProjectileComponent>(event.entityB);

        // Check for enemy-projectile collision
        if (enemyA && projectileB && projectileB->isPlayerProjectile) {
            enemyEntityId = event.entityA;
            projectileEntityId = event.entityB;
        } else if (enemyB && projectileA && projectileA->isPlayerProjectile) {
            enemyEntityId = event.entityB;
            projectileEntityId = event.entityA;
        } else {
            return;  // Not an enemy-player projectile collision
        }

        auto* health = m_registry.tryGetComponent<ecs::HealthComponent>(enemyEntityId);
        auto* enemy = m_registry.tryGetComponent<ecs::EnemyComponent>(enemyEntityId);
        auto* network = m_registry.tryGetComponent<ecs::NetworkComponent>(enemyEntityId);
        auto* projectile = m_registry.tryGetComponent<ecs::ProjectileComponent>(projectileEntityId);

        if (!health || !enemy || !network || !projectile) {
            return;
        }

        // Apply damage
        int damage = projectile->damage;
        health->currentHealth -= damage;
        if (health->currentHealth < 0) {
            health->currentHealth = 0;
        }

        std::cout << "[GameServer] Enemy hit! NetworkID=" << network->networkId
                  << " Health: " << health->currentHealth << "/" << health->maxHealth << std::endl;

        // Destroy the projectile
        auto* projectileNetwork = m_registry.tryGetComponent<ecs::NetworkComponent>(projectileEntityId);
        if (projectileNetwork) {
            ecs::Entity projectileEntity(projectileEntityId);
            m_networkIdManager->remove(projectileEntity);
            m_networkManager->broadcastEntityDestroy(projectileNetwork->networkId);
        }
        m_registry.destroyEntity(projectileEntityId);

        // Check if enemy is dead
        if (health->currentHealth <= 0) {
            handleEnemyDeath(enemyEntityId);
        }
    }

    void GameServer::handleEnemyDeath(ecs::EntityId enemyEntityId) {
        auto* enemy = m_registry.tryGetComponent<ecs::EnemyComponent>(enemyEntityId);
        auto* network = m_registry.tryGetComponent<ecs::NetworkComponent>(enemyEntityId);

        if (!enemy || !network) {
            return;
        }

        std::cout << "[GameServer] Enemy killed! NetworkID=" << network->networkId 
                  << " Score: " << enemy->scoreValue << std::endl;

        // Award score to all players and broadcast to clients
        m_teamScore += enemy->scoreValue;
        
        // Broadcast score update to all clients
        network::ScoreUpdateMessage scoreMsg;
        scoreMsg.clientId = 0;  // 0 = team score
        scoreMsg.newScore = static_cast<int32_t>(m_teamScore);
        scoreMsg.delta = enemy->scoreValue;
        auto scoreBuffer = network::serializeMessage(network::MessageType::SCORE_UPDATE, scoreMsg);
        m_networkManager->broadcast(scoreBuffer);

        // Decrement enemy count
        if (m_enemiesAlive > 0) {
            m_enemiesAlive--;
        }

        // Broadcast entity destroy
        m_networkIdManager->remove(ecs::Entity(enemyEntityId));
        m_networkManager->broadcastEntityDestroy(network->networkId);

        // Destroy the enemy entity
        m_registry.destroyEntity(enemyEntityId);

        // Check if level is complete
        checkLevelComplete();
    }

    void GameServer::checkLevelComplete() {
        if (!m_levelActive || !m_currentLevelConfig) {
            return;
        }

        // Check if all waves are done
        bool allWavesSpawned = (m_currentWaveIndex >= m_currentLevelConfig->waves.size());
        
        // Check if all enemies are dead
        bool allEnemiesDead = (m_enemiesAlive == 0);

        if (allWavesSpawned && allEnemiesDead) {
            std::cout << "[GameServer] Level complete! All waves finished and all enemies killed." << std::endl;
            handleLevelComplete();
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
        auto* health = m_registry.tryGetComponent<ecs::HealthComponent>(newPlayer);

        if (!network || !transform || !playerComp) {
            return;
        }

        // Set invincibility on respawn (3 seconds)
        if (health) {
            health->isInvincible = true;
            health->invincibilityTimer = 3.0f;
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

        std::cout << "[GameServer] Player respawned at (" << transform->x << ", " << transform->y << ") with 3s invincibility" << std::endl;
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

    // ============================================================
    // Level Management Implementation
    // ============================================================

    bool GameServer::loadLevel(uint8_t levelIndex) {
        if (levelIndex >= m_levelPaths.size()) {
            std::cerr << "[GameServer] Invalid level index: " << static_cast<int>(levelIndex) << std::endl;
            return false;
        }

        m_currentLevelIndex = levelIndex;
        
        std::cout << "[GameServer] Loading level " << static_cast<int>(levelIndex + 1) 
                  << " from " << m_levelPaths[levelIndex] << std::endl;

        // Load level config from JSON file
        m_currentLevelConfig = ecs::LevelLoader::loadFromFile(m_levelPaths[levelIndex]);
        if (!m_currentLevelConfig) {
            std::cerr << "[GameServer] Failed to load level config!" << std::endl;
            return false;
        }

        // Reset wave state
        m_currentWaveIndex = 0;
        m_waveTimer = 0.0f;
        m_levelTimer = 0.0f;
        m_enemiesSpawnedInWave = 0;
        m_enemySpawnTimer = 0.0f;
        m_waveActive = false;
        m_enemiesAlive = 0;

        std::cout << "[GameServer] Loaded level '" << m_currentLevelConfig->name 
                  << "' with " << m_currentLevelConfig->waves.size() << " waves" << std::endl;

        // Reset powerup spawn flags for this level
        for (auto& spawn : m_currentLevelConfig->powerupSpawns) {
            spawn.spawned = false;
        }
        for (auto& spawn : m_currentLevelConfig->bombSpawns) {
            spawn.spawned = false;
        }

        // Broadcast level info to clients
        broadcastLevelInfo();

        return true;
    }

    void GameServer::startLevel() {
        m_levelActive = true;
        
        std::cout << "[GameServer] Starting level " << static_cast<int>(m_currentLevelIndex + 1) << std::endl;

        // Broadcast level start
        network::LevelStartMessage startMsg;
        startMsg.levelIndex = m_currentLevelIndex;
        startMsg.serverTime = m_gameTime;
        startMsg.playerCount = static_cast<uint8_t>(m_playerManager->getPlayerCount());

        auto buffer = network::serializeMessage(network::MessageType::LEVEL_START, startMsg);
        m_networkManager->broadcast(buffer);

        // Start first wave
        if (m_currentLevelConfig && !m_currentLevelConfig->waves.empty()) {
            size_t enemyCount = m_currentLevelConfig->waves[0].enemies.size();
            broadcastWaveStart(1, static_cast<uint8_t>(enemyCount));
        }
    }

    void GameServer::handleWaveComplete(int waveNumber) {
        std::cout << "[GameServer] Wave " << waveNumber << " complete!" << std::endl;

        // Broadcast wave complete
        network::WaveCompleteMessage waveMsg;
        waveMsg.waveNumber = static_cast<uint8_t>(waveNumber);
        waveMsg.timeBonus = 0;  // TODO: Calculate time bonus

        auto buffer = network::serializeMessage(network::MessageType::WAVE_COMPLETE, waveMsg);
        m_networkManager->broadcast(buffer);

        // TODO: Check if more waves or boss, otherwise level complete
    }

    void GameServer::handleLevelComplete() {
        m_levelActive = false;

        std::cout << "[GameServer] Level " << static_cast<int>(m_currentLevelIndex + 1) << " complete!" << std::endl;

        // Calculate level bonus
        uint32_t levelBonus = 1000 * (m_currentLevelIndex + 1) * static_cast<uint32_t>(m_difficultyMultiplier);
        m_teamScore += levelBonus;

        // Broadcast level complete
        network::LevelCompleteMessage completeMsg;
        completeMsg.levelIndex = m_currentLevelIndex;
        completeMsg.totalScore = m_teamScore;
        completeMsg.completionTime = m_gameTime;
        
        // Determine next level
        uint8_t nextLevel = m_currentLevelIndex + 1;
        bool isLooping = false;
        
        if (nextLevel >= MAX_LEVELS) {
            nextLevel = 0;  // Loop back to level 1
            isLooping = true;
        }
        
        completeMsg.nextLevelIndex = nextLevel;
        completeMsg.isLooping = isLooping ? 1 : 0;

        auto buffer = network::serializeMessage(network::MessageType::LEVEL_COMPLETE, completeMsg);
        m_networkManager->broadcast(buffer);

        // Auto-advance to next level after a short delay
        // For now, advance immediately
        advanceToNextLevel();
    }

    void GameServer::advanceToNextLevel() {
        m_currentLevelIndex++;
        
        if (m_currentLevelIndex >= MAX_LEVELS) {
            // Start new loop with increased difficulty
            m_currentLevelIndex = 0;
            m_loopCount++;
            m_difficultyMultiplier = 1.0f + (m_loopCount * 0.5f);  // 1.5x, 2.0x, 2.5x, etc.
            
            // Cap difficulty at reasonable level
            if (m_difficultyMultiplier > 3.0f) {
                m_difficultyMultiplier = 3.0f;
            }
            
            std::cout << "[GameServer] Starting loop " << static_cast<int>(m_loopCount) 
                      << " with difficulty x" << m_difficultyMultiplier << std::endl;

            // Broadcast difficulty change
            network::DifficultyChangeMessage diffMsg;
            diffMsg.loopCount = m_loopCount;
            diffMsg.difficultyMultiplier = m_difficultyMultiplier;
            
            // Format display name
            if (m_loopCount == 1) {
                std::strncpy(diffMsg.displayName, "NEW GAME+", sizeof(diffMsg.displayName) - 1);
            } else {
                snprintf(diffMsg.displayName, sizeof(diffMsg.displayName), "LOOP %d", m_loopCount + 1);
            }
            diffMsg.displayName[sizeof(diffMsg.displayName) - 1] = '\0';

            auto buffer = network::serializeMessage(network::MessageType::DIFFICULTY_CHANGE, diffMsg);
            m_networkManager->broadcast(buffer);
        }

        // Load and start next level
        if (loadLevel(m_currentLevelIndex)) {
            startLevel();
        }
    }

    void GameServer::broadcastLevelInfo() {
        network::LevelInfoMessage infoMsg;
        infoMsg.levelIndex = m_currentLevelIndex;
        infoMsg.loopCount = m_loopCount;
        infoMsg.difficultyMultiplier = m_difficultyMultiplier;
        
        // Use level config if available
        if (m_currentLevelConfig) {
            std::strncpy(infoMsg.levelName, m_currentLevelConfig->name.c_str(), sizeof(infoMsg.levelName) - 1);
            std::strncpy(infoMsg.backgroundPath, m_currentLevelConfig->background.c_str(), sizeof(infoMsg.backgroundPath) - 1);
            std::strncpy(infoMsg.stageMusicPath, m_currentLevelConfig->stageMusic.c_str(), sizeof(infoMsg.stageMusicPath) - 1);
            std::strncpy(infoMsg.bossMusicPath, m_currentLevelConfig->bossMusic.c_str(), sizeof(infoMsg.bossMusicPath) - 1);
            infoMsg.totalWaves = static_cast<uint8_t>(m_currentLevelConfig->waves.size());
            infoMsg.difficulty = static_cast<uint8_t>(m_currentLevelConfig->difficulty);
            infoMsg.bossEnabled = m_currentLevelConfig->bossSection.enabled ? 1 : 0;
        } else {
            // Fallback to defaults
            const char* levelNames[] = {"Training Grounds", "Deep Space", "Alien Hive"};
            if (m_currentLevelIndex < 3) {
                std::strncpy(infoMsg.levelName, levelNames[m_currentLevelIndex], sizeof(infoMsg.levelName) - 1);
            } else {
                snprintf(infoMsg.levelName, sizeof(infoMsg.levelName), "Level %d", m_currentLevelIndex + 1);
            }
            std::strncpy(infoMsg.backgroundPath, "assets/backgrounds/26211.png", sizeof(infoMsg.backgroundPath) - 1);
            std::strncpy(infoMsg.stageMusicPath, "assets/sound/music/Sketchbook 2024-10-13.ogg", sizeof(infoMsg.stageMusicPath) - 1);
            std::strncpy(infoMsg.bossMusicPath, "assets/sound/music/Sketchbook 2024-10-16.ogg", sizeof(infoMsg.bossMusicPath) - 1);
            infoMsg.totalWaves = 6;
            infoMsg.difficulty = static_cast<uint8_t>(m_currentLevelIndex + 1);
            infoMsg.bossEnabled = 1;
        }
        infoMsg.levelName[sizeof(infoMsg.levelName) - 1] = '\0';

        auto buffer = network::serializeMessage(network::MessageType::LEVEL_INFO, infoMsg);
        m_networkManager->broadcast(buffer);

        std::cout << "[GameServer] Broadcast LEVEL_INFO for level " << static_cast<int>(m_currentLevelIndex + 1) << std::endl;
    }

    void GameServer::broadcastWaveStart(uint8_t waveNumber, uint8_t enemyCount) {
        network::WaveStartMessage waveMsg;
        waveMsg.waveNumber = waveNumber;
        waveMsg.totalWaves = m_currentLevelConfig ? 
            static_cast<uint8_t>(m_currentLevelConfig->waves.size()) : 6;
        waveMsg.enemyCount = enemyCount;

        auto buffer = network::serializeMessage(network::MessageType::WAVE_START, waveMsg);
        m_networkManager->broadcast(buffer);

        std::cout << "[GameServer] Wave " << static_cast<int>(waveNumber) << "/" 
                  << static_cast<int>(waveMsg.totalWaves) << " starting with " 
                  << static_cast<int>(enemyCount) << " enemies" << std::endl;
    }

    void GameServer::updatePlayerScore(uint32_t clientId, int32_t delta, network::ScoreReason reason) {
        // Update player score
        m_playerScores[clientId] += delta;
        m_teamScore += delta;

        // Broadcast score update
        network::ScoreUpdateMessage scoreMsg;
        scoreMsg.clientId = clientId;
        scoreMsg.newScore = static_cast<int32_t>(m_playerScores[clientId]);
        scoreMsg.delta = delta;

        auto buffer = network::serializeMessage(network::MessageType::SCORE_UPDATE, scoreMsg);
        m_networkManager->broadcast(buffer);
    }

    void GameServer::updateLevelWaves(float dt) {
        if (!m_currentLevelConfig) return;
        
        m_levelTimer += dt;
        
        const auto& waves = m_currentLevelConfig->waves;
        
        // Check if all waves are complete
        if (m_currentWaveIndex >= waves.size()) {
            // Level complete - could trigger boss or next level
            return;
        }
        
        const auto& currentWave = waves[m_currentWaveIndex];
        
        // Wait for wave delay before starting
        if (!m_waveActive) {
            m_waveTimer += dt;
            if (m_waveTimer >= currentWave.delayBefore) {
                m_waveActive = true;
                m_enemiesSpawnedInWave = 0;
                m_enemySpawnTimer = 0.0f;
                std::cout << "[GameServer] Wave " << (m_currentWaveIndex + 1) << " starting!" << std::endl;
            }
            return;
        }
        
        // Spawn enemies for current wave
        const auto& enemies = currentWave.enemies;
        
        if (currentWave.simultaneous) {
            // Spawn all enemies at once
            if (m_enemiesSpawnedInWave == 0) {
                for (const auto& enemyConfig : enemies) {
                    spawnEnemy(enemyConfig);
                    m_enemiesAlive++;
                }
                m_enemiesSpawnedInWave = enemies.size();
            }
        } else {
            // Sequential spawning
            m_enemySpawnTimer += dt;
            
            while (m_enemiesSpawnedInWave < enemies.size() && 
                   m_enemySpawnTimer >= currentWave.spawnInterval) {
                spawnEnemy(enemies[m_enemiesSpawnedInWave]);
                m_enemiesAlive++;
                m_enemiesSpawnedInWave++;
                m_enemySpawnTimer -= currentWave.spawnInterval;
            }
        }
        
        // Check if wave is complete (all enemies spawned)
        if (m_enemiesSpawnedInWave >= enemies.size()) {
            // Move to next wave
            m_currentWaveIndex++;
            m_waveActive = false;
            m_waveTimer = 0.0f;
            
            if (m_currentWaveIndex < waves.size()) {
                size_t nextEnemyCount = waves[m_currentWaveIndex].enemies.size();
                broadcastWaveStart(static_cast<uint8_t>(m_currentWaveIndex + 1), 
                                   static_cast<uint8_t>(nextEnemyCount));
            } else {
                std::cout << "[GameServer] All waves complete!" << std::endl;
                // TODO: Spawn boss if enabled
            }
        }
    }

    void GameServer::updatePowerupSpawns(float dt) {
        if (!m_currentLevelConfig) return;
        
        // Check powerup spawns based on level timer
        for (auto& spawn : m_currentLevelConfig->powerupSpawns) {
            if (!spawn.spawned && m_levelTimer >= spawn.triggerTime) {
                spawnPowerup(spawn.type, spawn.x, spawn.y);
                spawn.spawned = true;
                std::cout << "[GameServer] Spawned powerup type " << spawn.type 
                          << " at (" << spawn.x << ", " << spawn.y << ")" << std::endl;
            }
        }
        
        // Check bomb spawns
        for (auto& spawn : m_currentLevelConfig->bombSpawns) {
            if (!spawn.spawned && m_levelTimer >= spawn.triggerTime) {
                spawnPowerup(6, spawn.x, spawn.y);  // PowerupType::BOMB = 6
                spawn.spawned = true;
                std::cout << "[GameServer] Spawned bomb powerup at (" << spawn.x << ", " << spawn.y << ")" << std::endl;
            }
        }
    }

    void GameServer::spawnPowerup(int type, float x, float y) {
        ecs::Entity powerup = m_registry.createEntity();
        
        // Transform - powerups drift left slowly
        m_registry.addComponent(powerup, ecs::TransformComponent(x, y));
        m_registry.addComponent(powerup, ecs::VelocityComponent(-50.0f, 30.0f, 100.0f));
        
        // Powerup component
        ecs::PowerupComponent powerupComp;
        powerupComp.type = static_cast<ecs::PowerupType>(type);
        powerupComp.isCollected = false;
        m_registry.addComponent(powerup, powerupComp);
        
        // Collider for pickup detection
        ecs::ColliderComponent collider;
        collider.width = 24.0f;
        collider.height = 24.0f;
        collider.layer = ecs::CollisionLayer::Powerup;
        collider.mask = ecs::CollisionLayer::Player;
        collider.isTrigger = true;
        m_registry.addComponent(powerup, collider);
        
        // Lifetime - despawn after 20 seconds
        m_registry.addComponent(powerup, ecs::LifetimeComponent(20.0f));
        
        // Network ID
        uint32_t networkId = m_networkIdManager->allocate(powerup);
        m_registry.addComponent(powerup, ecs::NetworkComponent(networkId, false));
        
        // Broadcast spawn to clients
        network::EntitySpawnMessage spawnMsg{};
        spawnMsg.networkId = networkId;
        spawnMsg.entityType = network::EntityType::POWERUP;
        spawnMsg.x = x;
        spawnMsg.y = y;
        spawnMsg.rotation = 0.0f;
        spawnMsg.vx = -50.0f;
        spawnMsg.vy = 30.0f;
        spawnMsg.colliderWidth = 24.0f;
        spawnMsg.colliderHeight = 24.0f;
        spawnMsg.collisionLayer = static_cast<uint32_t>(ecs::CollisionLayer::Powerup);
        spawnMsg.collisionMask = static_cast<uint32_t>(ecs::CollisionLayer::Player);
        spawnMsg.trajectoryParam1 = static_cast<float>(type);  // Encode powerup type in trajectoryParam1
        
        m_networkManager->broadcastEntitySpawn(spawnMsg);
    }

    void GameServer::spawnEnemy(const ecs::EnemySpawnConfig& config) {
        ecs::Entity enemy = m_registry.createEntity();
        
        // Transform
        m_registry.addComponent(enemy, ecs::TransformComponent(config.x, config.y));
        
        // Velocity
        m_registry.addComponent(enemy, ecs::VelocityComponent(config.vx, config.vy, 300.0f));
        
        // Health
        int scaledHealth = static_cast<int>(config.health * m_difficultyMultiplier);
        m_registry.addComponent(enemy, ecs::HealthComponent(scaledHealth));
        
        // Enemy component
        ecs::EnemyComponent enemyComp;
        enemyComp.type = config.type;
        enemyComp.scoreValue = config.scoreValue;
        m_registry.addComponent(enemy, enemyComp);
        
        // Collider
        ecs::ColliderComponent collider;
        collider.width = 32.0f;
        collider.height = 32.0f;
        collider.layer = ecs::CollisionLayer::Enemy;
        collider.mask = static_cast<ecs::CollisionLayer>(
            static_cast<uint32_t>(ecs::CollisionLayer::Player) |
            static_cast<uint32_t>(ecs::CollisionLayer::PlayerShot)
        );
        m_registry.addComponent(enemy, collider);
        
        // Add trajectory based on enemy type
        // Chaser enemies follow sinusoidal path, others go linear
        if (config.type == ecs::EnemyType::Chaser) {
            ecs::TrajectoryComponent traj;
            traj.type = ecs::TrajectoryType::Sinusoidal;
            traj.initialized = false;
            m_registry.addComponent(enemy, traj);
        }
        
        // Network - Allocate network ID
        uint32_t networkId = m_networkIdManager->allocate(enemy);
        m_registry.addComponent(enemy, ecs::NetworkComponent(networkId, false));
        
        // Broadcast spawn to clients
        network::EntitySpawnMessage spawnMsg{};
        spawnMsg.networkId = networkId;
        spawnMsg.entityType = network::EntityType::ENEMY;
        spawnMsg.x = config.x;
        spawnMsg.y = config.y;
        spawnMsg.rotation = 0.0f;
        spawnMsg.vx = config.vx;
        spawnMsg.vy = config.vy;
        spawnMsg.colliderWidth = 32.0f;
        spawnMsg.colliderHeight = 32.0f;
        spawnMsg.collisionLayer = static_cast<uint32_t>(ecs::CollisionLayer::Enemy);
        spawnMsg.collisionMask = static_cast<uint32_t>(ecs::CollisionLayer::Player) |
                                 static_cast<uint32_t>(ecs::CollisionLayer::PlayerShot);
        
        // Set trajectory info based on enemy type
        if (config.type == ecs::EnemyType::Chaser) {
            spawnMsg.trajectoryType = static_cast<uint8_t>(ecs::TrajectoryType::Sinusoidal);
        } else {
            spawnMsg.trajectoryType = static_cast<uint8_t>(ecs::TrajectoryType::Linear);
        }
        
        m_networkManager->broadcastEntitySpawn(spawnMsg);
        
        std::cout << "[GameServer] Spawned enemy type " << static_cast<int>(config.type) 
                  << " at (" << config.x << ", " << config.y << ") [networkId=" << networkId << "]" << std::endl;
    }

    void GameServer::updateEnemies(float dt) {
        // Update all enemies with EnemyComponent
        auto enemyEntities = m_registry.getEntitiesWith<ecs::EnemyComponent>();
        
        // Collect enemies to destroy (can't destroy during iteration)
        std::vector<ecs::EntityId> enemiesToDestroy;
        
        for (ecs::EntityId eid : enemyEntities) {
            auto* enemy = m_registry.tryGetComponent<ecs::EnemyComponent>(eid);
            auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(eid);
            
            if (!enemy || !transform) continue;
            
            // Remove enemies that go off-screen (left side) - check first
            if (transform->x < -50.0f) {
                enemiesToDestroy.push_back(eid);
                continue;
            }
            
            // All enemies can shoot, but at different rates based on type
            enemy->fireTimer += dt;
            
            // Determine fire interval based on enemy type
            float fireInterval = 3.0f;  // Basic enemies shoot every 3 seconds
            if (enemy->type == ecs::EnemyType::Shooter || enemy->type == ecs::EnemyType::Turret) {
                fireInterval = 1.5f;  // Shooter/Turret enemies shoot faster
            } else if (enemy->type == ecs::EnemyType::Boss) {
                fireInterval = 0.8f;  // Boss shoots fastest
            }
            
            if (enemy->fireTimer >= fireInterval) {
                enemy->fireTimer = 0.0f;
                
                // Spawn projectile moving left (toward players)
                float projX = transform->x - 20.0f;
                float projY = transform->y;
                float projVx = -enemy->projectileSpeed;
                float projVy = 0.0f;
                
                spawnEnemyProjectile(projX, projY, projVx, projVy);
            }
        }
        
        // Now destroy collected enemies safely
        for (ecs::EntityId eid : enemiesToDestroy) {
            auto* network = m_registry.tryGetComponent<ecs::NetworkComponent>(eid);
            if (network) {
                m_networkIdManager->remove(ecs::Entity(eid));
                m_networkManager->broadcastEntityDestroy(network->networkId);
            }
            m_registry.destroyEntity(eid);
            if (m_enemiesAlive > 0) {
                m_enemiesAlive--;
            }
        }
        
        // Check level completion after destroying enemies
        if (!enemiesToDestroy.empty()) {
            checkLevelComplete();
        }
    }

    void GameServer::spawnEnemyProjectile(float x, float y, float vx, float vy) {
        ecs::Entity projectile = m_registry.createEntity();
        
        // Transform
        m_registry.addComponent(projectile, ecs::TransformComponent(x, y));
        
        // Velocity
        m_registry.addComponent(projectile, ecs::VelocityComponent(vx, vy, 500.0f));
        
        // Projectile component (enemy projectile)
        m_registry.addComponent(projectile, ecs::ProjectileComponent(ecs::NULL_ENTITY, 10, false));
        
        // Collider
        ecs::ColliderComponent collider;
        collider.width = 16.0f;
        collider.height = 16.0f;
        collider.layer = ecs::CollisionLayer::EnemyShot;
        collider.mask = static_cast<ecs::CollisionLayer>(
            static_cast<uint32_t>(ecs::CollisionLayer::Player) |
            static_cast<uint32_t>(ecs::CollisionLayer::Wall)
        );
        m_registry.addComponent(projectile, collider);
        
        // Lifetime (auto-destroy after 5 seconds)
        m_registry.addComponent(projectile, ecs::LifetimeComponent(5.0f));
        
        // Network - Allocate network ID
        uint32_t networkId = m_networkIdManager->allocate(projectile);
        m_registry.addComponent(projectile, ecs::NetworkComponent(networkId, false));
        
        // Broadcast spawn to clients
        network::EntitySpawnMessage spawnMsg{};
        spawnMsg.networkId = networkId;
        spawnMsg.entityType = network::EntityType::PROJECTILE;
        spawnMsg.x = x;
        spawnMsg.y = y;
        spawnMsg.rotation = 0.0f;
        spawnMsg.vx = vx;
        spawnMsg.vy = vy;
        spawnMsg.trajectoryType = 0;  // Linear
        spawnMsg.trajectoryParam1 = 0.0f;
        spawnMsg.trajectoryParam2 = 0.0f;
        spawnMsg.spinSpeed = 0.0f;
        spawnMsg.maxLifetime = 5.0f;
        spawnMsg.colliderWidth = 16.0f;
        spawnMsg.colliderHeight = 16.0f;
        spawnMsg.collisionLayer = static_cast<uint32_t>(ecs::CollisionLayer::EnemyShot);
        spawnMsg.collisionMask = static_cast<uint32_t>(ecs::CollisionLayer::Player) |
                                 static_cast<uint32_t>(ecs::CollisionLayer::Wall);
        
        m_networkManager->broadcastEntitySpawn(spawnMsg);
    }

} // namespace rtype::server
