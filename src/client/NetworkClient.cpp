/*
** R-Type Client - NetworkClient Implementation
*/

#include "NetworkClient.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "engine/ecs/components/NetworkComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/components/SpritesheetComponent.hpp"
#include "game/components/bullets/TrajectoryComponent.hpp"
#include "game/components/bullets/SpinComponent.hpp"
#include "game/components/PlayerShipComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/WeaponConstants.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/PowerupComponent.hpp"

namespace rtype::client {

    NetworkClient::NetworkClient(ecs::Registry& registry)
        : m_registry(registry)
        , m_ioContext()
        , m_socket(m_ioContext)
        , m_running(false)
        , m_connected(false)
        , m_welcomeReceived(false)
        , m_clientId(0)
        , m_inputSequence(0)
    {
        std::cout << "[NetworkClient] Initialized" << std::endl;
    }

    NetworkClient::~NetworkClient() {
        disconnect();
    }

    bool NetworkClient::connect(const std::string& serverIp, uint16_t serverPort) {
        try {
            std::cout << "[NetworkClient] Connecting to " << serverIp << ":" << serverPort << "..." << std::endl;

            // Resolve server endpoint
            asio::ip::udp::resolver resolver(m_ioContext);
            auto endpoints = resolver.resolve(asio::ip::udp::v4(), serverIp, std::to_string(serverPort));
            m_serverEndpoint = *endpoints.begin();

            // Open socket
            m_socket.open(asio::ip::udp::v4());

            m_running = true;
            m_connected = true;

            // Send CLIENT_HELLO
            sendHello();

            // Start receive thread
            m_receiveThread = std::thread(&NetworkClient::receiveLoop, this);

            std::cout << "[NetworkClient] Connection initiated" << std::endl;
            return true;

        } catch (const std::exception& e) {
            std::cerr << "[NetworkClient] Connection failed: " << e.what() << std::endl;
            m_connected = false;
            return false;
        }
    }

    void NetworkClient::disconnect() {
        if (!m_running) {
            return;
        }

        std::cout << "[NetworkClient] Disconnecting..." << std::endl;

        // Send CLIENT_DISCONNECT
        if (m_connected && m_clientId != 0) {
            network::ClientDisconnectMessage msg;
            msg.clientId = m_clientId;
            auto buffer = network::serializeMessage(network::MessageType::CLIENT_DISCONNECT, msg);
            sendToServer(buffer);

            // Give the message time to be sent before closing socket
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        m_running = false;
        m_connected = false;
        m_welcomeReceived = false;

        // Clear pending state updates to prevent memory leaks
        m_pendingStateUpdates.clear();

        // Close socket to interrupt blocking receive
        asio::error_code ec;
        m_socket.close(ec);

        // Join receive thread
        if (m_receiveThread.joinable()) {
            m_receiveThread.join();
        }

        std::cout << "[NetworkClient] Disconnected" << std::endl;
    }

    void NetworkClient::update() {
        std::lock_guard<std::mutex> lock(m_queueMutex);

        while (!m_messageQueue.empty()) {
            PendingMessage& pending = m_messageQueue.front();

            // Deserialize and handle based on type
            switch (pending.type) {
                case network::MessageType::SERVER_WELCOME: {
                    network::ServerWelcomeMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleServerWelcome(msg);
                    }
                    break;
                }

                case network::MessageType::ENTITY_SPAWN: {
                    network::EntitySpawnMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleEntitySpawn(msg);
                    }
                    break;
                }

                case network::MessageType::ENTITY_STATE: {
                    network::EntityStateMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleEntityState(msg);
                    }
                    break;
                }

                case network::MessageType::ENTITY_DESTROY: {
                    network::EntityDestroyMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleEntityDestroy(msg);
                    }
                    break;
                }

                case network::MessageType::PLAYER_SPAWN: {
                    network::PlayerSpawnMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handlePlayerSpawn(msg);
                    }
                    break;
                }

                case network::MessageType::PLAYER_HIT: {
                    network::PlayerHitMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handlePlayerHit(msg);
                    }
                    break;
                }

                case network::MessageType::PLAYER_DEATH: {
                    network::PlayerDeathMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handlePlayerDeath(msg);
                    }
                    break;
                }

                case network::MessageType::PLAYER_RESPAWN: {
                    network::PlayerRespawnMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handlePlayerRespawn(msg);
                    }
                    break;
                }

                // Level management messages
                case network::MessageType::LEVEL_INFO: {
                    network::LevelInfoMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleLevelInfo(msg);
                    }
                    break;
                }

                case network::MessageType::LEVEL_START: {
                    network::LevelStartMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleLevelStart(msg);
                    }
                    break;
                }

                case network::MessageType::LEVEL_COMPLETE: {
                    network::LevelCompleteMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleLevelComplete(msg);
                    }
                    break;
                }

                case network::MessageType::WAVE_START: {
                    network::WaveStartMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleWaveStart(msg);
                    }
                    break;
                }

                case network::MessageType::WAVE_COMPLETE: {
                    network::WaveCompleteMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleWaveComplete(msg);
                    }
                    break;
                }

                case network::MessageType::BOSS_START: {
                    network::BossStartMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleBossStart(msg);
                    }
                    break;
                }

                case network::MessageType::BOSS_DEFEATED: {
                    network::BossDefeatedMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleBossDefeated(msg);
                    }
                    break;
                }

                case network::MessageType::DIFFICULTY_CHANGE: {
                    network::DifficultyChangeMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleDifficultyChange(msg);
                    }
                    break;
                }

                case network::MessageType::SCORE_UPDATE: {
                    network::ScoreUpdateMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleScoreUpdate(msg);
                    }
                    break;
                }

                // Room/Lobby messages
                case network::MessageType::ROOM_CREATED: {
                    network::RoomCreatedMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleRoomCreated(msg);
                    }
                    break;
                }

                case network::MessageType::ROOM_JOINED: {
                    network::RoomJoinedMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleRoomJoined(msg);
                    }
                    break;
                }

                case network::MessageType::ROOM_LEFT: {
                    network::RoomLeftMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleRoomLeft(msg);
                    }
                    break;
                }

                case network::MessageType::ROOM_LIST: {
                    network::RoomListMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleRoomList(msg);
                    }
                    break;
                }

                case network::MessageType::ROOM_INFO: {
                    network::RoomInfoMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleRoomInfo(msg);
                    }
                    break;
                }

                case network::MessageType::HOST_CHANGED: {
                    network::HostChangedMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleHostChanged(msg);
                    }
                    break;
                }

                case network::MessageType::ROOM_ERROR: {
                    network::RoomErrorMessage msg;
                    if (network::deserializeMessage(pending.data, msg)) {
                        handleRoomError(msg);
                    }
                    break;
                }

                default:
                    break;
            }

            m_messageQueue.pop();
        }
    }

    void NetworkClient::updateAnimations(float dt) {
        // Process respawn slide-in animations
        std::vector<uint32_t> completedAnimations;
        
        for (auto& [networkId, anim] : m_respawnAnimations) {
            anim.elapsed += dt;
            
            // Find the entity
            auto it = m_networkIdToEntity.find(networkId);
            if (it == m_networkIdToEntity.end()) {
                completedAnimations.push_back(networkId);
                continue;
            }
            
            ecs::Entity entity = it->second;
            auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(entity);
            
            if (!transform) {
                completedAnimations.push_back(networkId);
                continue;
            }
            
            if (anim.elapsed >= anim.duration) {
                // Animation complete - snap to final position
                transform->x = anim.targetX;
                transform->y = anim.targetY;
                
                // Turn off invincibility visual after slide-in (health timer still controls actual invincibility)
                auto* ship = m_registry.tryGetComponent<ecs::PlayerShipComponent>(entity);
                if (ship) {
                    ship->isInvincible = false;  // Stop the component-level flicker
                }
                
                completedAnimations.push_back(networkId);
            } else {
                // Ease-out interpolation for smooth slide-in
                float t = anim.elapsed / anim.duration;
                float easeOut = 1.0f - (1.0f - t) * (1.0f - t);  // Quadratic ease-out
                transform->x = anim.startX + (anim.targetX - anim.startX) * easeOut;
                transform->y = anim.targetY;  // Y is already at target
            }
        }
        
        // Remove completed animations
        for (uint32_t networkId : completedAnimations) {
            m_respawnAnimations.erase(networkId);
        }
    }

    void NetworkClient::receiveLoop() {
        std::cout << "[NetworkClient] Receive loop started" << std::endl;

        while (m_running) {
            try {
                // Prepare receive buffer
                std::vector<uint8_t> buffer(2048);  // 2KB buffer
                asio::ip::udp::endpoint senderEndpoint;

                // Blocking receive
                asio::error_code ec;
                size_t receivedBytes = m_socket.receive_from(
                    asio::buffer(buffer), senderEndpoint, 0, ec);

                if (ec) {
                    if (ec == asio::error::operation_aborted || ec == asio::error::bad_descriptor) {
                        break;
                    }
                    continue;
                }

                if (receivedBytes == 0) {
                    continue;
                }

                // Resize buffer to actual received size
                buffer.resize(receivedBytes);

                // Process message (push to queue)
                onMessageReceived(buffer);

            } catch (const std::exception& e) {
            }
        }

        std::cout << "[NetworkClient] Receive loop stopped" << std::endl;
    }

    void NetworkClient::onMessageReceived(const std::vector<uint8_t>& buffer) {
        // Deserialize header to get message type
        network::MessageHeader header;
        if (!network::deserializeHeader(buffer, header)) {
            std::cerr << "[NetworkClient] Failed to deserialize header" << std::endl;
            return;
        }

        // Push to queue for main thread processing
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_messageQueue.push({header.type, buffer});
    }

    void NetworkClient::handleServerWelcome(const network::ServerWelcomeMessage& message) {
        m_clientId = message.clientId;
        m_welcomeReceived = true;  // Mark as truly connected
        std::cout << "[NetworkClient] Connected! Assigned clientId=" << m_clientId
                 << " (protocol v" << message.protocolVersion << ")" << std::endl;

        // Clear any stale entity mappings from previous sessions
        // This prevents networkId conflicts when reconnecting to a fresh server
        if (!m_networkIdToEntity.empty()) {
            std::cout << "[NetworkClient] Clearing " << m_networkIdToEntity.size()
                     << " stale entity mappings from previous session" << std::endl;
            m_networkIdToEntity.clear();
        }
        
        // Notify that we're fully connected with a valid client ID
        if (m_callbacks.onWelcome) {
            m_callbacks.onWelcome(m_clientId);
        }
    }

    void NetworkClient::handleEntitySpawn(const network::EntitySpawnMessage& message) {
        std::cout << "[NetworkClient] Received ENTITY_SPAWN (networkId=" << message.networkId << ")" << std::endl;

        // Check if entity already exists
        auto it = m_networkIdToEntity.find(message.networkId);
        if (it != m_networkIdToEntity.end()) {
            std::cerr << "[NetworkClient] Entity already exists for networkId " << message.networkId << std::endl;
            return;
        }

        // Create entity
        ecs::Entity entity = m_registry.createEntity();

        // Add components based on message data
        m_registry.addComponent(entity, ecs::TransformComponent(message.x, message.y, message.rotation));

        m_registry.addComponent(entity, ecs::VelocityComponent(message.vx, message.vy, 500.0f));

        m_registry.addComponent(entity, ecs::NetworkComponent(message.networkId, false));

        if (message.entityType == network::EntityType::PROJECTILE) {
            bool isPlayerProjectile = (message.collisionLayer == static_cast<uint32_t>(ecs::CollisionLayer::PlayerShot));

            m_registry.addComponent(entity, ecs::ProjectileComponent(ecs::NULL_ENTITY, 10, isPlayerProjectile));

            ecs::SpritesheetComponent sprite;
            if (isPlayerProjectile) {
                sprite.setBullet(ecs::BulletType::Rice, ecs::BulletColor::Cyan);
                sprite.rotation = -90.0f;
                sprite.tintR = 80;
                sprite.tintG = 240;
                sprite.tintB = 255;
                sprite.layer = 100;
            } else {
                sprite.setBullet(ecs::BulletType::Ball, ecs::BulletColor::Red);
                sprite.layer = 99;
            }
            m_registry.addComponent(entity, sprite);

            if (message.trajectoryType != 0) {
                ecs::TrajectoryComponent traj;
                traj.type = static_cast<ecs::TrajectoryType>(message.trajectoryType);
                traj.initialized = false;
                m_registry.addComponent(entity, traj);
            }

            if (message.spinSpeed != 0.0f) {
                ecs::SpinComponent spin;
                spin.spinSpeed = message.spinSpeed;
                m_registry.addComponent(entity, spin);
            }

            if (message.maxLifetime > 0.0f) {
                m_registry.addComponent(entity, ecs::LifetimeComponent(message.maxLifetime));
            }

            if (message.colliderWidth > 0.0f && message.colliderHeight > 0.0f) {
                ecs::ColliderComponent collider;
                collider.width = message.colliderWidth;
                collider.height = message.colliderHeight;
                collider.layer = static_cast<ecs::CollisionLayer>(message.collisionLayer);
                collider.mask = static_cast<ecs::CollisionLayer>(message.collisionMask);
                m_registry.addComponent(entity, collider);
            }
        } else if (message.entityType == network::EntityType::ENEMY) {
            // Add enemy component with correct type from server
            ecs::EnemyComponent enemyComp;
            enemyComp.type = static_cast<ecs::EnemyType>(message.enemyType);
            enemyComp.scoreValue = 100;
            m_registry.addComponent(entity, enemyComp);

            // Add health component
            m_registry.addComponent(entity, ecs::HealthComponent(1, 1));

            // Add sprite component based on enemy type
            ecs::SpritesheetComponent sprite;
            sprite.textureId = "r-typesheet5";  // Default enemy spritesheet
            sprite.isVisible = true;
            sprite.hasGlow = true;
            sprite.glowIntensity = 0.4f;
            
            switch (enemyComp.type) {
                case ecs::EnemyType::Basic:
                    sprite.frameX = 0; sprite.frameY = 0;
                    sprite.frameWidth = 32; sprite.frameHeight = 32;
                    sprite.tintR = 100; sprite.tintG = 200; sprite.tintB = 255;
                    break;
                case ecs::EnemyType::Shooter:
                    sprite.frameX = 1; sprite.frameY = 0;
                    sprite.frameWidth = 32; sprite.frameHeight = 32;
                    sprite.tintR = 255; sprite.tintG = 150; sprite.tintB = 50;
                    break;
                case ecs::EnemyType::Chaser:
                    sprite.frameX = 2; sprite.frameY = 0;
                    sprite.frameWidth = 32; sprite.frameHeight = 32;
                    sprite.tintR = 200; sprite.tintG = 100; sprite.tintB = 255;
                    break;
                case ecs::EnemyType::Turret:
                    sprite.frameX = 3; sprite.frameY = 0;
                    sprite.frameWidth = 32; sprite.frameHeight = 32;
                    sprite.tintR = 100; sprite.tintG = 255; sprite.tintB = 100;
                    break;
                case ecs::EnemyType::Boss:
                    sprite.textureId = "r-typesheet1";  // Boss uses different sheet
                    sprite.frameX = 0; sprite.frameY = 0;
                    sprite.frameWidth = 96; sprite.frameHeight = 96;
                    sprite.tintR = 255; sprite.tintG = 50; sprite.tintB = 50;
                    sprite.glowIntensity = 0.6f;
                    break;
                default:
                    break;
            }
            sprite.layer = (enemyComp.type == ecs::EnemyType::Boss) ? 8 : 6;
            m_registry.addComponent(entity, sprite);

            // Add collider for enemies
            if (message.colliderWidth > 0.0f && message.colliderHeight > 0.0f) {
                ecs::ColliderComponent collider;
                collider.width = message.colliderWidth;
                collider.height = message.colliderHeight;
                collider.layer = static_cast<ecs::CollisionLayer>(message.collisionLayer);
                collider.mask = static_cast<ecs::CollisionLayer>(message.collisionMask);
                m_registry.addComponent(entity, collider);
            }

            // Add trajectory if present
            if (message.trajectoryType != 0) {
                ecs::TrajectoryComponent traj;
                traj.type = static_cast<ecs::TrajectoryType>(message.trajectoryType);
                traj.initialized = false;
                m_registry.addComponent(entity, traj);
            }

            std::cout << "[NetworkClient] Created ENEMY entity type " << static_cast<int>(enemyComp.type) 
                      << " at (" << message.x << ", " << message.y << ")" << std::endl;
        } else if (message.entityType == network::EntityType::POWERUP) {
            // Add powerup component
            ecs::PowerupComponent powerup;
            powerup.type = static_cast<ecs::PowerupType>(static_cast<int>(message.trajectoryParam1));  // trajectoryParam1 encodes powerup type
            powerup.isCollected = false;
            m_registry.addComponent(entity, powerup);

            // Add collider for powerups
            if (message.colliderWidth > 0.0f && message.colliderHeight > 0.0f) {
                ecs::ColliderComponent collider;
                collider.width = message.colliderWidth;
                collider.height = message.colliderHeight;
                collider.layer = static_cast<ecs::CollisionLayer>(message.collisionLayer);
                collider.mask = static_cast<ecs::CollisionLayer>(message.collisionMask);
                m_registry.addComponent(entity, collider);
            }

            std::cout << "[NetworkClient] Created POWERUP entity type " << static_cast<int>(powerup.type) 
                      << " at (" << message.x << ", " << message.y << ")" << std::endl;
        }

        m_networkIdToEntity[message.networkId] = entity;

        std::cout << "[NetworkClient] Created entity " << entity << " for networkId " << message.networkId << std::endl;
    }

    void NetworkClient::handleEntityState(const network::EntityStateMessage& message) {
        auto it = m_networkIdToEntity.find(message.networkId);
        if (it == m_networkIdToEntity.end()) {
            std::cout << "[NetworkClient] Entity " << message.networkId
                      << " not found, queueing state update" << std::endl;
            m_pendingStateUpdates[message.networkId] = message;
            return;
        }

        ecs::Entity entity = it->second;

        // Skip position updates for local player - we control our own position
        // This prevents the "rollback" effect where server corrections fight with local input
        if (m_registry.hasComponent<ecs::PlayerComponent>(entity)) {
            const auto& player = m_registry.getComponent<ecs::PlayerComponent>(entity);
            if (player.isLocal) {
                // Only update velocity, not position for local player
                auto* velocity = m_registry.tryGetComponent<ecs::VelocityComponent>(entity);
                if (velocity) {
                    velocity->vx = message.vx;
                    velocity->vy = message.vy;
                }
                return;
            }
        }

        auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(entity);
        if (transform) {
            transform->x = message.x;
            transform->y = message.y;
            transform->rotation = message.rotation;
        }

        auto* velocity = m_registry.tryGetComponent<ecs::VelocityComponent>(entity);
        if (velocity) {
            velocity->vx = message.vx;
            velocity->vy = message.vy;
        }
    }

    void NetworkClient::handleEntityDestroy(const network::EntityDestroyMessage& message) {
        std::cout << "[NetworkClient] Received ENTITY_DESTROY (networkId=" << message.networkId << ")" << std::endl;

        // Find entity
        auto it = m_networkIdToEntity.find(message.networkId);
        if (it == m_networkIdToEntity.end()) {
            std::cerr << "[NetworkClient] Entity not found for networkId " << message.networkId << std::endl;
            return;
        }

        ecs::Entity entity = it->second;

        // Destroy entity
        m_registry.destroyEntity(entity);

        // Remove from mapping
        m_networkIdToEntity.erase(it);

        std::cout << "[NetworkClient] Destroyed entity " << entity << " (networkId=" << message.networkId << ")" << std::endl;
    }

    void NetworkClient::sendHello() {
        network::ClientHelloMessage msg;
        msg.protocolVersion = 1;
        std::strncpy(msg.playerName, "Player", sizeof(msg.playerName) - 1);

        auto buffer = network::serializeMessage(network::MessageType::CLIENT_HELLO, msg);
        sendToServer(buffer);

        std::cout << "[NetworkClient] Sent CLIENT_HELLO" << std::endl;
    }

    void NetworkClient::sendToServer(const std::vector<uint8_t>& buffer) {
        try {
            asio::error_code ec;
            m_socket.send_to(asio::buffer(buffer), m_serverEndpoint, 0, ec);

            if (ec) {
                std::cerr << "[NetworkClient] Send error: " << ec.message() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[NetworkClient] Exception during send: " << e.what() << std::endl;
        }
    }

    void NetworkClient::sendInput(const network::ClientInputMessage& input) {
        if (!m_connected) {
            return;
        }

        auto buffer = network::serializeMessage(network::MessageType::CLIENT_INPUT, input);
        sendToServer(buffer);
    }

    void NetworkClient::sendPlayerReady() {
        if (!m_connected) {
            std::cerr << "[NetworkClient] Cannot send PLAYER_READY - not connected!" << std::endl;
            return;
        }

        network::PlayerReadyMessage msg;
        msg.clientId = m_clientId;

        auto buffer = network::serializeMessage(network::MessageType::PLAYER_READY, msg);
        sendToServer(buffer);

        std::cout << "[NetworkClient] Sent PLAYER_READY to server (clientId=" << m_clientId << ")" << std::endl;
    }

    void NetworkClient::createRoom(const std::string& roomName, uint8_t maxPlayers) {
        if (!m_connected) {
            std::cerr << "[NetworkClient] Cannot create room - not connected!" << std::endl;
            return;
        }

        std::cout << "[NetworkClient] Creating room: '" << roomName 
                  << "' (max players: " << static_cast<int>(maxPlayers) << ")" << std::endl;
        
        network::CreateRoomMessage msg{};
        msg.clientId = m_clientId;
        std::strncpy(msg.roomName, roomName.c_str(), sizeof(msg.roomName) - 1);
        msg.maxPlayers = maxPlayers;
        auto buffer = network::serializeMessage(network::MessageType::CREATE_ROOM, msg);
        sendToServer(buffer);
    }

    void NetworkClient::joinRoom(const std::string& roomName) {
        if (!m_connected) {
            std::cerr << "[NetworkClient] Cannot join room - not connected!" << std::endl;
            return;
        }

        std::cout << "[NetworkClient] Joining room: '" << roomName << "'" << std::endl;

        network::JoinRoomMessage msg{};
        msg.clientId = m_clientId;
        std::strncpy(msg.roomName, roomName.c_str(), sizeof(msg.roomName) - 1);
        auto buffer = network::serializeMessage(network::MessageType::JOIN_ROOM, msg);
        sendToServer(buffer);
    }

    void NetworkClient::leaveRoom() {
        if (!m_connected) {
            return;
        }

        std::cout << "[NetworkClient] Leaving room: '" << m_currentRoomName << "'" << std::endl;

        network::LeaveRoomMessage msg{};
        msg.clientId = m_clientId;
        auto buffer = network::serializeMessage(network::MessageType::LEAVE_ROOM, msg);
        sendToServer(buffer);
    }

    void NetworkClient::requestRoomList() {
        if (!m_connected) {
            std::cerr << "[NetworkClient] Cannot request room list - not connected!" << std::endl;
            return;
        }

        std::cout << "[NetworkClient] Requesting room list from server..." << std::endl;

        network::RoomListRequestMessage msg{};
        msg.clientId = m_clientId;
        auto buffer = network::serializeMessage(network::MessageType::ROOM_LIST_REQUEST, msg);
        sendToServer(buffer);
    }

    void NetworkClient::hostStartGame(uint8_t levelIndex) {
        if (!m_connected) {
            std::cerr << "[NetworkClient] Cannot start game - not connected!" << std::endl;
            return;
        }

        if (!m_isHost) {
            std::cerr << "[NetworkClient] Cannot start game - not the host!" << std::endl;
            return;
        }

        std::cout << "[NetworkClient] Host starting game (level " << static_cast<int>(levelIndex) << ")" << std::endl;

        network::HostStartGameMessage msg{};
        msg.clientId = m_clientId;
        msg.levelIndex = levelIndex;
        auto buffer = network::serializeMessage(network::MessageType::HOST_START_GAME, msg);
        sendToServer(buffer);
    }

    // Room message handlers
    void NetworkClient::handleRoomCreated(const network::RoomCreatedMessage& message) {
        std::cout << "[NetworkClient] Room created: '" << message.roomName 
                  << "' (host: " << message.hostClientId << ")" << std::endl;

        if (m_callbacks.onRoomCreated) {
            m_callbacks.onRoomCreated(message);
        }
    }

    void NetworkClient::handleRoomJoined(const network::RoomJoinedMessage& message) {
        std::cout << "[NetworkClient] Joined room: '" << message.roomName 
                  << "' (slot: " << static_cast<int>(message.yourSlot) 
                  << ", isHost: " << (message.youAreHost ? "yes" : "no") << ")" << std::endl;

        m_currentRoomName = message.roomName;
        m_isHost = message.youAreHost;
        m_playerSlot = message.yourSlot;

        if (m_callbacks.onRoomJoined) {
            m_callbacks.onRoomJoined(message);
        }
    }

    void NetworkClient::handleRoomLeft(const network::RoomLeftMessage& message) {
        std::cout << "[NetworkClient] Left room (clientId: " << message.clientId << ")" << std::endl;

        if (message.clientId == m_clientId) {
            m_currentRoomName.clear();
            m_isHost = false;
            m_playerSlot = 0;
        }

        if (m_callbacks.onRoomLeft) {
            m_callbacks.onRoomLeft(message);
        }
    }

    void NetworkClient::handleRoomList(const network::RoomListMessage& message) {
        std::cout << "[NetworkClient] Received room list (" << static_cast<int>(message.roomCount) << " rooms)" << std::endl;

        if (m_callbacks.onRoomList) {
            m_callbacks.onRoomList(message);
        }
    }

    void NetworkClient::handleRoomInfo(const network::RoomInfoMessage& message) {
        std::cout << "[NetworkClient] Room info update: '" << message.roomName 
                  << "' (" << static_cast<int>(message.playerCount) << "/" 
                  << static_cast<int>(message.maxPlayers) << " players)" << std::endl;

        // Update host status if we're in this room
        if (m_currentRoomName == message.roomName) {
            m_isHost = (message.hostClientId == m_clientId);
        }

        if (m_callbacks.onRoomInfo) {
            m_callbacks.onRoomInfo(message);
        }
    }

    void NetworkClient::handleHostChanged(const network::HostChangedMessage& message) {
        std::cout << "[NetworkClient] Host changed to: " << message.newHostClientId 
                  << " (reason: " << message.reason << ")" << std::endl;

        m_isHost = (message.newHostClientId == m_clientId);

        if (m_isHost) {
            std::cout << "[NetworkClient] You are now the host!" << std::endl;
        }

        if (m_callbacks.onHostChanged) {
            m_callbacks.onHostChanged(message);
        }
    }

    void NetworkClient::handleRoomError(const network::RoomErrorMessage& message) {
        std::cerr << "[NetworkClient] Room error: " << message.message << std::endl;

        if (m_callbacks.onRoomError) {
            m_callbacks.onRoomError(message);
        }
    }

    void NetworkClient::handlePlayerSpawn(const network::PlayerSpawnMessage& message) {
        std::cout << "[NetworkClient] Received PLAYER_SPAWN (networkId=" << message.networkId
                  << ", clientId=" << message.clientId << ", slot=" << (int)message.playerSlot << ")" << std::endl;

        auto it = m_networkIdToEntity.find(message.networkId);
        if (it != m_networkIdToEntity.end()) {
            std::cerr << "[NetworkClient] Player already exists for networkId " << message.networkId << std::endl;
            return;
        }

        bool isLocal = (message.clientId == m_clientId);

        // Store local player network ID for respawn handling
        if (isLocal) {
            m_localPlayerNetworkId = message.networkId;
        }

        ecs::Entity entity = m_registry.createEntity();

        ecs::PlayerComponent playerComp(message.playerSlot, 3);
        playerComp.isLocal = isLocal;
        m_registry.addComponent(entity, playerComp);

        m_registry.addComponent(entity, ecs::TransformComponent(message.x, message.y, 0.0f));
        m_registry.addComponent(entity, ecs::VelocityComponent(0.0f, 0.0f, 200.0f));
        m_registry.addComponent(entity, ecs::HealthComponent(static_cast<int>(message.health), 100));
        m_registry.addComponent(entity, ecs::NetworkComponent(message.networkId, isLocal));

        ecs::WeaponComponent weapon(ecs::WeaponConstants::DEFAULT_FIRE_RATE, ecs::WeaponConstants::DEFAULT_DAMAGE);
        weapon.projectileSpeed = ecs::WeaponConstants::DEFAULT_PROJECTILE_SPEED;
        m_registry.addComponent(entity, weapon);

        ecs::PlayerShipComponent shipComp(ecs::PlayerShipComponent::ShipStyle::Classic);
        shipComp.layer = 10;
        m_registry.addComponent(entity, shipComp);

        std::cout << "[NetworkClient] PlayerShipComponent state: isVisible=" << shipComp.isVisible
                  << ", isInvincible=" << shipComp.isInvincible << ", layer=" << shipComp.layer << std::endl;

        auto pendingIt = m_pendingStateUpdates.find(message.networkId);
        if (pendingIt != m_pendingStateUpdates.end()) {
            std::cout << "[NetworkClient] Applying pending state update for entity "
                      << message.networkId << std::endl;
            auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(entity);
            if (transform) {
                transform->x = pendingIt->second.x;
                transform->y = pendingIt->second.y;
                transform->rotation = pendingIt->second.rotation;
            }
            auto* velocity = m_registry.tryGetComponent<ecs::VelocityComponent>(entity);
            if (velocity) {
                velocity->vx = pendingIt->second.vx;
                velocity->vy = pendingIt->second.vy;
            }
            m_pendingStateUpdates.erase(pendingIt);
        }

        m_networkIdToEntity[message.networkId] = entity;

        std::cout << "[NetworkClient] Created player entity " << entity
                  << " (networkId=" << message.networkId << ", isLocal=" << isLocal << ")" << std::endl;
    }

    void NetworkClient::handlePlayerHit(const network::PlayerHitMessage& message) {
        std::cout << "[NetworkClient] Received PLAYER_HIT (networkId=" << message.networkId
                  << ", newHealth=" << message.newHealth 
                  << ", invincible=" << message.isInvincible
                  << ", timer=" << message.invincibilityTimer << ")" << std::endl;

        auto it = m_networkIdToEntity.find(message.networkId);
        if (it == m_networkIdToEntity.end()) {
            std::cerr << "[NetworkClient] Player not found for networkId " << message.networkId << std::endl;
            return;
        }

        ecs::Entity entity = it->second;

        auto* health = m_registry.tryGetComponent<ecs::HealthComponent>(entity);
        if (health) {
            health->currentHealth = static_cast<int>(message.newHealth);
            // Sync invincibility state from server
            health->isInvincible = message.isInvincible;
            health->invincibilityTimer = message.invincibilityTimer;
        }
        
        // Also update PlayerShipComponent for visual flicker
        auto* ship = m_registry.tryGetComponent<ecs::PlayerShipComponent>(entity);
        if (ship) {
            ship->isInvincible = message.isInvincible;
        }

        ecs::Entity hitEffect = m_registry.createEntity();

        m_registry.addComponent(hitEffect, ecs::TransformComponent(message.hitX, message.hitY, 0.0f));

        ecs::SpritesheetComponent sprite;
        sprite.setBullet(ecs::BulletType::Ball, ecs::BulletColor::Red);
        sprite.hasGlow = true;
        sprite.glowIntensity = 1.0f;  // Maximum glow for visibility
        sprite.frameWidth = 32;  // Larger size
        sprite.frameHeight = 32;
        m_registry.addComponent(hitEffect, sprite);

        m_registry.addComponent(hitEffect, ecs::LifetimeComponent(0.3f));
        m_registry.addComponent(hitEffect, ecs::VelocityComponent(0.0f, -50.0f, 0.0f));

        std::cout << "[NetworkClient] Player health updated to " << message.newHealth
                  << " - spawned hit effect" << std::endl;
    }

    void NetworkClient::handlePlayerDeath(const network::PlayerDeathMessage& message) {
        std::cout << "[NetworkClient] Received PLAYER_DEATH (networkId=" << message.networkId
                  << ", remainingLives=" << static_cast<int>(message.remainingLives) << ")" << std::endl;

        auto it = m_networkIdToEntity.find(message.networkId);
        if (it == m_networkIdToEntity.end()) {
            std::cerr << "[NetworkClient] Player not found for networkId " << message.networkId << std::endl;
            return;
        }

        ecs::Entity entity = it->second;

        // Update player lives
        auto* player = m_registry.tryGetComponent<ecs::PlayerComponent>(entity);
        if (player) {
            player->lives = static_cast<int>(message.remainingLives);
            std::cout << "[NetworkClient] Updated player lives to " << player->lives << std::endl;
        }

        // Create death explosion effect at death position
        ecs::Entity deathEffect = m_registry.createEntity();
        m_registry.addComponent(deathEffect, ecs::TransformComponent(message.deathX, message.deathY, 0.0f, 2.0f, 2.0f));
        
        ecs::SpritesheetComponent sprite;
        sprite.setBullet(ecs::BulletType::Ball, ecs::BulletColor::Red);
        sprite.hasGlow = true;
        sprite.glowIntensity = 1.5f;
        sprite.frameWidth = 48;
        sprite.frameHeight = 48;
        m_registry.addComponent(deathEffect, sprite);
        
        m_registry.addComponent(deathEffect, ecs::LifetimeComponent(0.5f));

        // If player has no lives remaining, mark them as dead (will be respawned by server or game over)
        if (message.remainingLives == 0) {
            std::cout << "[NetworkClient] Player has no lives remaining - game over for this player" << std::endl;
        }

        // Hide player by setting visibility to false (not moving off-screen which causes sync issues)
        auto* ship = m_registry.tryGetComponent<ecs::PlayerShipComponent>(entity);
        if (ship) {
            ship->isVisible = false;
        }

        // Store death position for respawn animation
        auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(entity);
        if (transform) {
            m_deathPositions[message.networkId] = {transform->x, transform->y};
        }
    }

    void NetworkClient::handlePlayerRespawn(const network::PlayerRespawnMessage& message) {
        std::cout << "[NetworkClient] Received PLAYER_RESPAWN (networkId=" << message.networkId
                  << ", slot=" << static_cast<int>(message.playerSlot) 
                  << ", pos=" << message.x << "," << message.y << ")" << std::endl;

        auto it = m_networkIdToEntity.find(message.networkId);
        if (it == m_networkIdToEntity.end()) {
            // Player might have been destroyed, need to recreate
            std::cout << "[NetworkClient] Creating new entity for respawned player" << std::endl;
            
            ecs::Entity newPlayer = m_registry.createEntity();
            
            // Start off-screen left, will slide in
            m_registry.addComponent(newPlayer, ecs::TransformComponent(-50.0f, message.y, 0.0f));
            m_registry.addComponent(newPlayer, ecs::VelocityComponent(0.0f, 0.0f, 350.0f));
            
            ecs::PlayerComponent playerComp(message.playerSlot, 3);
            playerComp.isLocal = (message.networkId == m_localPlayerNetworkId);
            m_registry.addComponent(newPlayer, playerComp);
            
            m_registry.addComponent(newPlayer, ecs::HealthComponent(100, 100));
            m_registry.addComponent(newPlayer, ecs::NetworkComponent(message.networkId, playerComp.isLocal));
            
            // Add PlayerShipComponent for rendering
            ecs::PlayerShipComponent shipComp(ecs::PlayerShipComponent::ShipStyle::Classic);
            shipComp.layer = 10;
            shipComp.isVisible = true;
            shipComp.isInvincible = message.isInvincible;  // Visual invincibility indicator
            m_registry.addComponent(newPlayer, shipComp);
            
            // Add WeaponComponent for shooting
            ecs::WeaponComponent weapon(ecs::WeaponConstants::DEFAULT_FIRE_RATE, ecs::WeaponConstants::DEFAULT_DAMAGE);
            weapon.projectileSpeed = ecs::WeaponConstants::DEFAULT_PROJECTILE_SPEED;
            m_registry.addComponent(newPlayer, weapon);
            
            // Add invincibility on respawn from server values
            auto* health = m_registry.tryGetComponent<ecs::HealthComponent>(newPlayer);
            if (health) {
                health->isInvincible = message.isInvincible;
                health->invincibilityTimer = message.invincibilityTimer;
            }
            
            // Store respawn animation state
            m_respawnAnimations[message.networkId] = {-50.0f, message.x, message.y, 0.0f, 0.5f};
            
            m_networkIdToEntity[message.networkId] = newPlayer;
            return;
        }

        ecs::Entity entity = it->second;

        // Make player visible again
        auto* ship = m_registry.tryGetComponent<ecs::PlayerShipComponent>(entity);
        if (ship) {
            ship->isVisible = true;
            ship->isInvincible = true;  // Visual indicator
        }

        // Start position off-screen left for slide-in animation
        auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(entity);
        if (transform) {
            transform->x = -50.0f;
            transform->y = message.y;
        }

        // Reset health and add invincibility from server
        auto* health = m_registry.tryGetComponent<ecs::HealthComponent>(entity);
        if (health) {
            health->currentHealth = static_cast<int>(message.health);
            health->isInvincible = message.isInvincible;
            health->invincibilityTimer = message.invincibilityTimer;
        }

        // Store respawn animation state (startX, targetX, targetY, elapsed, duration)
        m_respawnAnimations[message.networkId] = {-50.0f, message.x, message.y, 0.0f, 0.5f};

        std::cout << "[NetworkClient] Player respawned with slide-in animation and invincibility!" << std::endl;
    }

    // ============================================================
    // Level and Game Event Handlers
    // ============================================================

    void NetworkClient::handleLevelInfo(const network::LevelInfoMessage& message) {
        std::cout << "[NetworkClient] Received LEVEL_INFO: " << message.levelName 
                  << " (level " << static_cast<int>(message.levelIndex + 1) 
                  << ", loop " << static_cast<int>(message.loopCount)
                  << ", difficulty x" << message.difficultyMultiplier << ")" << std::endl;

        if (m_callbacks.onLevelInfo) {
            m_callbacks.onLevelInfo(message);
        }
    }

    void NetworkClient::handleLevelStart(const network::LevelStartMessage& message) {
        std::cout << "[NetworkClient] Received LEVEL_START: level " 
                  << static_cast<int>(message.levelIndex + 1) << std::endl;

        if (m_callbacks.onLevelStart) {
            m_callbacks.onLevelStart(message);
        }
    }

    void NetworkClient::handleLevelComplete(const network::LevelCompleteMessage& message) {
        std::cout << "[NetworkClient] Received LEVEL_COMPLETE: level " 
                  << static_cast<int>(message.levelIndex + 1)
                  << ", score " << message.totalScore
                  << ", next level " << static_cast<int>(message.nextLevelIndex + 1)
                  << (message.isLooping ? " (LOOPING!)" : "") << std::endl;

        if (m_callbacks.onLevelComplete) {
            m_callbacks.onLevelComplete(message);
        }
    }

    void NetworkClient::handleWaveStart(const network::WaveStartMessage& message) {
        std::cout << "[NetworkClient] Received WAVE_START: wave " 
                  << static_cast<int>(message.waveNumber) << "/" << static_cast<int>(message.totalWaves)
                  << " (" << static_cast<int>(message.enemyCount) << " enemies)" << std::endl;

        if (m_callbacks.onWaveStart) {
            m_callbacks.onWaveStart(message);
        }
    }

    void NetworkClient::handleWaveComplete(const network::WaveCompleteMessage& message) {
        std::cout << "[NetworkClient] Received WAVE_COMPLETE: wave " 
                  << static_cast<int>(message.waveNumber)
                  << ", time bonus " << message.timeBonus << std::endl;

        if (m_callbacks.onWaveComplete) {
            m_callbacks.onWaveComplete(message);
        }
    }

    void NetworkClient::handleBossStart(const network::BossStartMessage& message) {
        std::cout << "[NetworkClient] Received BOSS_START: networkId=" << message.bossNetworkId
                  << " (phases: " << static_cast<int>(message.totalPhases) << ")" << std::endl;

        if (m_callbacks.onBossStart) {
            m_callbacks.onBossStart(message);
        }
    }

    void NetworkClient::handleBossDefeated(const network::BossDefeatedMessage& message) {
        std::cout << "[NetworkClient] Received BOSS_DEFEATED: networkId=" << message.bossNetworkId
                  << ", score " << message.scoreValue << std::endl;

        if (m_callbacks.onBossDefeated) {
            m_callbacks.onBossDefeated(message);
        }
    }

    void NetworkClient::handleDifficultyChange(const network::DifficultyChangeMessage& message) {
        std::cout << "[NetworkClient] Received DIFFICULTY_CHANGE: " << message.displayName
                  << " (loop " << static_cast<int>(message.loopCount)
                  << ", multiplier x" << message.difficultyMultiplier << ")" << std::endl;

        if (m_callbacks.onDifficultyChange) {
            m_callbacks.onDifficultyChange(message);
        }
    }

    void NetworkClient::handleScoreUpdate(const network::ScoreUpdateMessage& message) {
        std::cout << "[NetworkClient] Received SCORE_UPDATE: client " << message.clientId
                  << ", score " << message.newScore << " (delta " << message.delta << ")" << std::endl;

        if (m_callbacks.onScoreUpdate) {
            m_callbacks.onScoreUpdate(message);
        }
    }

    // ============================================================
    // Additional Send Methods
    // ============================================================

    void NetworkClient::sendLoadLevelRequest(uint8_t levelIndex) {
        if (!m_connected) {
            std::cerr << "[NetworkClient] Cannot send LOAD_LEVEL_REQUEST - not connected!" << std::endl;
            return;
        }

        network::LoadLevelRequestMessage msg;
        msg.clientId = m_clientId;
        msg.levelIndex = levelIndex;

        auto buffer = network::serializeMessage(network::MessageType::LOAD_LEVEL_REQUEST, msg);
        sendToServer(buffer);

        std::cout << "[NetworkClient] Sent LOAD_LEVEL_REQUEST for level " 
                  << static_cast<int>(levelIndex + 1) << std::endl;
    }

    void NetworkClient::sendPlayerProfile(const char* name, uint8_t avatarId, uint8_t colorScheme) {
        if (!m_connected) {
            std::cerr << "[NetworkClient] Cannot send PLAYER_PROFILE - not connected!" << std::endl;
            return;
        }

        network::PlayerProfileMessage msg;
        msg.clientId = m_clientId;
        std::strncpy(msg.playerName, name, sizeof(msg.playerName) - 1);
        msg.playerName[sizeof(msg.playerName) - 1] = '\0';
        msg.avatarId = avatarId;
        msg.colorScheme = colorScheme;

        auto buffer = network::serializeMessage(network::MessageType::PLAYER_PROFILE, msg);
        sendToServer(buffer);

        std::cout << "[NetworkClient] Sent PLAYER_PROFILE (name=" << name 
                  << ", avatar=" << static_cast<int>(avatarId) << ")" << std::endl;
    }

} // namespace rtype::client
