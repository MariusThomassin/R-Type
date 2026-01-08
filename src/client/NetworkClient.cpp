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

namespace rtype::client {

    NetworkClient::NetworkClient(ecs::Registry& registry)
        : m_registry(registry)
        , m_ioContext()
        , m_socket(m_ioContext)
        , m_running(false)
        , m_connected(false)
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
        }

        m_running = false;
        m_connected = false;

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
        // Process all pending messages from receive thread
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

                default:
                    std::cerr << "[NetworkClient] Unknown message type: "
                             << static_cast<int>(pending.type) << std::endl;
                    break;
            }

            m_messageQueue.pop();
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
                        // Socket closed, exit loop
                        break;
                    }
                    std::cerr << "[NetworkClient] Receive error: " << ec.message() << std::endl;
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
                if (m_running) {
                    std::cerr << "[NetworkClient] Exception in receive loop: " << e.what() << std::endl;
                }
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
        std::cout << "[NetworkClient] Connected! Assigned clientId=" << m_clientId
                 << " (protocol v" << message.protocolVersion << ")" << std::endl;

        // Clear any stale entity mappings from previous sessions
        // This prevents networkId conflicts when reconnecting to a fresh server
        if (!m_networkIdToEntity.empty()) {
            std::cout << "[NetworkClient] Clearing " << m_networkIdToEntity.size()
                     << " stale entity mappings from previous session" << std::endl;
            m_networkIdToEntity.clear();
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
        // Transform
        m_registry.addComponent(entity, ecs::TransformComponent(message.x, message.y, message.rotation));

        // Velocity
        m_registry.addComponent(entity, ecs::VelocityComponent(message.vx, message.vy, 500.0f));

        // Network
        m_registry.addComponent(entity, ecs::NetworkComponent(message.networkId, false));

        // Entity type specific components
        if (message.entityType == network::EntityType::PROJECTILE) {
            // Determine if this is a player projectile based on collision layer
            bool isPlayerProjectile = (message.collisionLayer == static_cast<uint32_t>(ecs::CollisionLayer::PlayerShot));

            // Projectile
            m_registry.addComponent(entity, ecs::ProjectileComponent(ecs::NULL_ENTITY, 10, isPlayerProjectile));

            // Spritesheet (for rendering)
            ecs::SpritesheetComponent sprite;
            if (isPlayerProjectile) {
                // Player bullets: cyan rice bullets pointing right
                sprite.setBullet(ecs::BulletType::Rice, ecs::BulletColor::Cyan);
                sprite.rotation = -90.0f;  // Point to the right
                sprite.tintR = 80;
                sprite.tintG = 240;
                sprite.tintB = 255;
                sprite.layer = 100;  // Render on top
            } else {
                // Enemy bullets: red balls
                sprite.setBullet(ecs::BulletType::Ball, ecs::BulletColor::Red);
                sprite.layer = 99;
            }
            m_registry.addComponent(entity, sprite);

            // Trajectory
            if (message.trajectoryType != 0) {
                ecs::TrajectoryComponent traj;
                traj.type = static_cast<ecs::TrajectoryType>(message.trajectoryType);
                traj.initialized = false;
                m_registry.addComponent(entity, traj);
            }

            // Spin
            if (message.spinSpeed != 0.0f) {
                ecs::SpinComponent spin;
                spin.spinSpeed = message.spinSpeed;
                m_registry.addComponent(entity, spin);
            }

            // Lifetime
            if (message.maxLifetime > 0.0f) {
                m_registry.addComponent(entity, ecs::LifetimeComponent(message.maxLifetime));
            }

            // Collider
            if (message.colliderWidth > 0.0f && message.colliderHeight > 0.0f) {
                ecs::ColliderComponent collider;
                collider.width = message.colliderWidth;
                collider.height = message.colliderHeight;
                collider.layer = static_cast<ecs::CollisionLayer>(message.collisionLayer);
                collider.mask = static_cast<ecs::CollisionLayer>(message.collisionMask);
                m_registry.addComponent(entity, collider);
            }
        }

        // Map networkId → entity
        m_networkIdToEntity[message.networkId] = entity;

        std::cout << "[NetworkClient] Created entity " << entity << " for networkId " << message.networkId << std::endl;
    }

    void NetworkClient::handleEntityState(const network::EntityStateMessage& message) {
        // Find entity by network ID
        auto it = m_networkIdToEntity.find(message.networkId);
        if (it == m_networkIdToEntity.end()) {
            // Entity not found (might have been destroyed or not yet spawned)
            return;
        }

        ecs::Entity entity = it->second;

        // Update transform
        auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(entity);
        if (transform) {
            transform->x = message.x;
            transform->y = message.y;
            transform->rotation = message.rotation;
        }

        // Update velocity
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

    void NetworkClient::handlePlayerSpawn(const network::PlayerSpawnMessage& message) {
        std::cout << "[NetworkClient] Received PLAYER_SPAWN (networkId=" << message.networkId
                  << ", clientId=" << message.clientId << ", slot=" << (int)message.playerSlot << ")" << std::endl;

        // Check if entity already exists
        auto it = m_networkIdToEntity.find(message.networkId);
        if (it != m_networkIdToEntity.end()) {
            std::cerr << "[NetworkClient] Player already exists for networkId " << message.networkId << std::endl;
            return;
        }

        // Determine if this is OUR player
        bool isLocal = (message.clientId == m_clientId);

        // Create entity
        ecs::Entity entity = m_registry.createEntity();

        // Add components
        ecs::PlayerComponent playerComp(message.playerSlot, 3);
        playerComp.isLocal = isLocal;
        m_registry.addComponent(entity, playerComp);

        m_registry.addComponent(entity, ecs::TransformComponent(message.x, message.y, 0.0f));
        m_registry.addComponent(entity, ecs::VelocityComponent(0.0f, 0.0f, 200.0f));
        m_registry.addComponent(entity, ecs::HealthComponent(static_cast<int>(message.health), 100));
        m_registry.addComponent(entity, ecs::NetworkComponent(message.networkId, isLocal));

        // Add weapon component (for shooting missiles with Space key)
        ecs::WeaponComponent weapon(ecs::WeaponConstants::DEFAULT_FIRE_RATE, ecs::WeaponConstants::DEFAULT_DAMAGE);
        weapon.projectileSpeed = ecs::WeaponConstants::DEFAULT_PROJECTILE_SPEED;
        m_registry.addComponent(entity, weapon);

        // Add visual component (PlayerShipComponent for rendering)
        ecs::PlayerShipComponent shipComp(ecs::PlayerShipComponent::ShipStyle::Classic);
        shipComp.layer = 10;
        m_registry.addComponent(entity, shipComp);

        // Map networkId → entity
        m_networkIdToEntity[message.networkId] = entity;

        std::cout << "[NetworkClient] Created player entity " << entity
                  << " (networkId=" << message.networkId << ", isLocal=" << isLocal << ")" << std::endl;
    }

    void NetworkClient::handlePlayerHit(const network::PlayerHitMessage& message) {
        std::cout << "[NetworkClient] Received PLAYER_HIT (networkId=" << message.networkId
                  << ", newHealth=" << message.newHealth << ")" << std::endl;

        // Find entity
        auto it = m_networkIdToEntity.find(message.networkId);
        if (it == m_networkIdToEntity.end()) {
            std::cerr << "[NetworkClient] Player not found for networkId " << message.networkId << std::endl;
            return;
        }

        ecs::Entity entity = it->second;

        // Update health
        auto* health = m_registry.tryGetComponent<ecs::HealthComponent>(entity);
        if (health) {
            health->currentHealth = static_cast<int>(message.newHealth);
        }

        // Spawn hit effect at impact position
        ecs::Entity hitEffect = m_registry.createEntity();

        // Position at hit location
        m_registry.addComponent(hitEffect, ecs::TransformComponent(message.hitX, message.hitY, 0.0f));

        // Visual effect - red circle with strong glow that fades out
        ecs::SpritesheetComponent sprite;
        sprite.setBullet(ecs::BulletType::Ball, ecs::BulletColor::Red);
        sprite.hasGlow = true;
        sprite.glowIntensity = 1.0f;  // Maximum glow for visibility
        sprite.frameWidth = 32;  // Larger size
        sprite.frameHeight = 32;
        m_registry.addComponent(hitEffect, sprite);

        // Auto-destroy after 0.3 seconds
        m_registry.addComponent(hitEffect, ecs::LifetimeComponent(0.3f));

        // Float upward slightly
        m_registry.addComponent(hitEffect, ecs::VelocityComponent(0.0f, -50.0f, 0.0f));

        std::cout << "[NetworkClient] Player health updated to " << message.newHealth
                  << " - spawned hit effect" << std::endl;
    }

} // namespace rtype::client
