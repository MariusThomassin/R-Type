/*
** R-Type Server - NetworkManager Implementation
*/

#include "NetworkManager.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/NetworkComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "game/components/PlayerComponent.hpp"

namespace rtype::server {

    NetworkManager::NetworkManager(ecs::Registry& registry, uint16_t port)
        : m_registry(registry)
        , m_ioContext()
        , m_socket(m_ioContext, asio::ip::udp::endpoint(asio::ip::udp::v4(), port))
        , m_running(false)
        , m_nextClientId(1)
        , m_timeSinceLastUpdate(0.0f)
        , m_port(port)
    {
        std::cout << "[NetworkManager] Initialized on port " << port << std::endl;
    }

    NetworkManager::~NetworkManager() {
        stop();
    }

    void NetworkManager::start() {
        if (m_running) {
            return;
        }

        m_running = true;
        std::cout << "[NetworkManager] Starting network listening on 0.0.0.0:" << m_port << std::endl;

        // Start receive thread
        m_receiveThread = std::thread(&NetworkManager::receiveLoop, this);
    }

    void NetworkManager::stop() {
        if (!m_running) {
            return;
        }

        std::cout << "[NetworkManager] Stopping network..." << std::endl;
        m_running = false;

        // Close socket to interrupt blocking receive
        asio::error_code ec;
        m_socket.close(ec);

        // Join receive thread
        if (m_receiveThread.joinable()) {
            m_receiveThread.join();
        }

        std::cout << "[NetworkManager] Network stopped" << std::endl;
    }

    void NetworkManager::update(float dt) {
        m_timeSinceLastUpdate += dt;

        // Send state updates at 20 Hz (50ms interval)
        if (m_timeSinceLastUpdate >= NETWORK_UPDATE_RATE) {
            m_timeSinceLastUpdate = 0.0f;

            // Get all entities with NetworkComponent
            m_registry.forEach<ecs::NetworkComponent>([this](ecs::EntityId entity) {
                auto* networkComp = m_registry.tryGetComponent<ecs::NetworkComponent>(entity);
                auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(entity);
                auto* velocity = m_registry.tryGetComponent<ecs::VelocityComponent>(entity);

                if (!networkComp || !transform) {
                    return;
                }

                // Send state update
                network::EntityStateMessage stateMsg;
                stateMsg.networkId = static_cast<uint32_t>(networkComp->networkId);
                stateMsg.x = transform->x;
                stateMsg.y = transform->y;
                stateMsg.rotation = transform->rotation;

                if (velocity) {
                    stateMsg.vx = velocity->vx;
                    stateMsg.vy = velocity->vy;
                } else {
                    stateMsg.vx = 0.0f;
                    stateMsg.vy = 0.0f;
                }

                broadcastEntityState(stateMsg);
            });
        }
    }

    void NetworkManager::receiveLoop() {
        std::cout << "[NetworkManager] Receive loop started" << std::endl;

        while (m_running) {
            try {
                // Prepare receive buffer
                std::vector<uint8_t> buffer(1024);  // 1KB buffer

                // Blocking receive with timeout
                asio::error_code ec;
                size_t receivedBytes = m_socket.receive_from(
                    asio::buffer(buffer), m_remoteEndpoint, 0, ec);

                if (ec) {
                    if (ec == asio::error::operation_aborted || ec == asio::error::bad_descriptor) {
                        // Socket closed, exit loop
                        break;
                    }
                    // Other errors, log and continue
                    std::cerr << "[NetworkManager] Receive error: " << ec.message() << std::endl;
                    continue;
                }

                if (receivedBytes == 0) {
                    continue;
                }

                // Resize buffer to actual received size
                buffer.resize(receivedBytes);

                // Process message
                processMessage(buffer, m_remoteEndpoint);

            } catch (const std::exception& e) {
                if (m_running) {
                    std::cerr << "[NetworkManager] Exception in receive loop: " << e.what() << std::endl;
                }
            }
        }

        std::cout << "[NetworkManager] Receive loop stopped" << std::endl;
    }

    void NetworkManager::processMessage(const std::vector<uint8_t>& buffer,
                                       const asio::ip::udp::endpoint& senderEndpoint) {
        // Deserialize header
        network::MessageHeader header;
        if (!network::deserializeHeader(buffer, header)) {
            std::cerr << "[NetworkManager] Failed to deserialize header" << std::endl;
            return;
        }

        // Process based on message type
        switch (header.type) {
            case network::MessageType::CLIENT_HELLO: {
                network::ClientHelloMessage msg;
                if (network::deserializeMessage(buffer, msg)) {
                    handleClientHello(msg, senderEndpoint);
                }
                break;
            }

            case network::MessageType::CLIENT_DISCONNECT: {
                network::ClientDisconnectMessage msg;
                if (network::deserializeMessage(buffer, msg)) {
                    handleClientDisconnect(msg);
                }
                break;
            }

            case network::MessageType::CLIENT_INPUT: {
                network::ClientInputMessage msg;
                if (network::deserializeMessage(buffer, msg)) {
                    handleClientInput(msg, senderEndpoint);
                }
                break;
            }

            default:
                std::cerr << "[NetworkManager] Unknown message type: "
                         << static_cast<int>(header.type) << std::endl;
                break;
        }
    }

    void NetworkManager::handleClientHello(const network::ClientHelloMessage& message,
                                          const asio::ip::udp::endpoint& senderEndpoint) {
        uint32_t clientId;

        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);

            // Check if client already connected
            ClientInfo* existing = findClient(senderEndpoint);
            if (existing) {
                std::cout << "[NetworkManager] Client already connected: "
                         << senderEndpoint << " (clientId=" << existing->clientId << ")" << std::endl;
                return;
            }

            // Assign client ID
            clientId = m_nextClientId++;
            m_clients.emplace_back(clientId, senderEndpoint);

            std::cout << "[NetworkManager] Client connected: " << senderEndpoint
                     << " (clientId=" << clientId << ", protocol=" << message.protocolVersion << ")" << std::endl;

            // Send welcome message
            network::ServerWelcomeMessage welcome;
            welcome.clientId = clientId;
            welcome.protocolVersion = 1;
            welcome.serverTime = 0.0f;  // TODO: Actual server time

            auto welcomeBuffer = network::serializeMessage(network::MessageType::SERVER_WELCOME, welcome);
            sendTo(welcomeBuffer, senderEndpoint);

            // Send full snapshot to new client
            sendSnapshotToClient(m_clients.back());

            std::cout << "[NetworkManager] Sent welcome and snapshot to client " << clientId << std::endl;
        } // Release lock here

        // Notify via callback OUTSIDE the lock to avoid deadlock
        if (m_onClientConnected) {
            m_onClientConnected(clientId);
        }
    }

    void NetworkManager::handleClientDisconnect(const network::ClientDisconnectMessage& message) {
        bool found = false;

        {
            std::lock_guard<std::mutex> lock(m_clientsMutex);

            // Find and remove client
            auto it = std::find_if(m_clients.begin(), m_clients.end(),
                [&](const ClientInfo& client) { return client.clientId == message.clientId; });

            if (it != m_clients.end()) {
                std::cout << "[NetworkManager] Client disconnected: " << it->endpoint
                         << " (clientId=" << message.clientId << ")" << std::endl;
                m_clients.erase(it);
                found = true;
            }
        } // Release lock here

        // Notify via callback OUTSIDE the lock to avoid deadlock
        if (found && m_onClientDisconnected) {
            m_onClientDisconnected(message.clientId);
        }
    }

    void NetworkManager::handleClientInput(const network::ClientInputMessage& message,
                                          const asio::ip::udp::endpoint& senderEndpoint) {
        // Find client by endpoint
        ClientInfo* client = findClient(senderEndpoint);
        if (!client) {
            std::cerr << "[NetworkManager] Received input from unknown client: " << senderEndpoint << std::endl;
            return;
        }

        // Notify via callback
        if (m_onClientInput) {
            m_onClientInput(client->clientId, message);
        }
    }

    void NetworkManager::broadcastEntitySpawn(const network::EntitySpawnMessage& message) {
        auto buffer = network::serializeMessage(network::MessageType::ENTITY_SPAWN, message);
        broadcast(buffer);

        std::cout << "[NetworkManager] Broadcasting ENTITY_SPAWN (networkId=" << message.networkId
                 << ", type=" << static_cast<int>(message.entityType) << ")" << std::endl;
    }

    void NetworkManager::broadcastEntityState(const network::EntityStateMessage& message) {
        auto buffer = network::serializeMessage(network::MessageType::ENTITY_STATE, message);
        broadcast(buffer);

        // Note: State updates are very frequent, don't log each one
    }

    void NetworkManager::broadcastEntityDestroy(uint32_t networkId) {
        network::EntityDestroyMessage message;
        message.networkId = networkId;

        auto buffer = network::serializeMessage(network::MessageType::ENTITY_DESTROY, message);
        broadcast(buffer);

        std::cout << "[NetworkManager] Broadcasting ENTITY_DESTROY (networkId=" << networkId << ")" << std::endl;
    }

    void NetworkManager::sendSnapshotToClient(const ClientInfo& client) {
        // Count networked entities
        size_t entityCount = 0;
        m_registry.forEach<ecs::NetworkComponent>([&entityCount](ecs::EntityId) {
            entityCount++;
        });

        std::cout << "[NetworkManager] Sending snapshot to client " << client.clientId
                 << " (" << entityCount << " entities)" << std::endl;

        // Send each entity (use PLAYER_SPAWN for players, ENTITY_SPAWN for others)
        m_registry.forEach<ecs::NetworkComponent>([this, &client](ecs::EntityId entity) {
            auto* networkComp = m_registry.tryGetComponent<ecs::NetworkComponent>(entity);
            auto* transform = m_registry.tryGetComponent<ecs::TransformComponent>(entity);
            auto* playerComp = m_registry.tryGetComponent<ecs::PlayerComponent>(entity);

            if (!networkComp || !transform) {
                return;
            }

            // If it's a player, send PLAYER_SPAWN message
            if (playerComp) {
                auto* health = m_registry.tryGetComponent<ecs::HealthComponent>(entity);

                network::PlayerSpawnMessage playerMsg{};
                playerMsg.networkId = static_cast<uint32_t>(networkComp->networkId);
                playerMsg.clientId = playerComp->networkClientId;  // Use stored client ID
                playerMsg.playerSlot = playerComp->slot;
                playerMsg.x = transform->x;
                playerMsg.y = transform->y;
                playerMsg.health = health ? static_cast<float>(health->currentHealth) : 100.0f;

                auto buffer = network::serializeMessage(network::MessageType::PLAYER_SPAWN, playerMsg);
                sendTo(buffer, client.endpoint);
            }
            // Otherwise, send ENTITY_SPAWN (projectile, enemy, etc.)
            else {
                auto* velocity = m_registry.tryGetComponent<ecs::VelocityComponent>(entity);

                network::EntitySpawnMessage spawnMsg{};
                spawnMsg.networkId = static_cast<uint32_t>(networkComp->networkId);
                spawnMsg.entityType = network::EntityType::PROJECTILE;  // For now, all are projectiles
                spawnMsg.x = transform->x;
                spawnMsg.y = transform->y;
                spawnMsg.rotation = transform->rotation;

                if (velocity) {
                    spawnMsg.vx = velocity->vx;
                    spawnMsg.vy = velocity->vy;
                }

                // TODO: Add trajectory, spin, lifetime, collider data

                auto buffer = network::serializeMessage(network::MessageType::ENTITY_SPAWN, spawnMsg);
                sendTo(buffer, client.endpoint);
            }
        });
    }

    void NetworkManager::broadcast(const std::vector<uint8_t>& buffer) {
        std::lock_guard<std::mutex> lock(m_clientsMutex);

        for (const auto& client : m_clients) {
            if (client.isActive) {
                sendTo(buffer, client.endpoint);
            }
        }
    }

    void NetworkManager::sendTo(const std::vector<uint8_t>& buffer,
                               const asio::ip::udp::endpoint& endpoint) {
        try {
            asio::error_code ec;
            m_socket.send_to(asio::buffer(buffer), endpoint, 0, ec);

            if (ec) {
                std::cerr << "[NetworkManager] Send error to " << endpoint
                         << ": " << ec.message() << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[NetworkManager] Exception during send: " << e.what() << std::endl;
        }
    }

    ClientInfo* NetworkManager::findClient(const asio::ip::udp::endpoint& endpoint) {
        for (auto& client : m_clients) {
            if (client.endpoint == endpoint) {
                return &client;
            }
        }
        return nullptr;
    }

    ClientInfo* NetworkManager::findClient(uint32_t clientId) {
        for (auto& client : m_clients) {
            if (client.clientId == clientId) {
                return &client;
            }
        }
        return nullptr;
    }

    size_t NetworkManager::getClientCount() const {
        std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_clientsMutex));
        return m_clients.size();
    }

} // namespace rtype::server
