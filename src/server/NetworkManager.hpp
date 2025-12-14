/*
** R-Type Server - NetworkManager
** Handles UDP network communication with clients
*/

#pragma once

#include "shared/network/Protocol.hpp"
#include "engine/ecs/core/Registry.hpp"
#include <asio.hpp>
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <chrono>
#include <functional>

namespace rtype::server {

    /**
     * @brief Information about a connected client
     */
    struct ClientInfo {
        uint32_t clientId;
        asio::ip::udp::endpoint endpoint;
        float lastHeartbeat;
        bool isActive;

        ClientInfo(uint32_t id, const asio::ip::udp::endpoint& ep)
            : clientId(id), endpoint(ep), lastHeartbeat(0.0f), isActive(true) {}
    };

    /**
     * @brief Manages network communication for the server
     *
     * Responsibilities:
     * - Listen for client connections (UDP)
     * - Maintain list of connected clients
     * - Broadcast entity state to all clients (20 Hz)
     * - Receive and process client messages
     */
    class NetworkManager {
    public:
        /**
         * @brief Constructor
         *
         * @param registry Reference to ECS registry
         * @param port Server port (default 4242)
         */
        explicit NetworkManager(ecs::Registry& registry, uint16_t port = 4242);

        /**
         * @brief Destructor - stops network threads
         */
        ~NetworkManager();

        /**
         * @brief Start network listening thread
         */
        void start();

        /**
         * @brief Stop network threads
         */
        void stop();

        /**
         * @brief Update network state (called from game loop)
         *
         * @param dt Delta time since last frame
         */
        void update(float dt);

        /**
         * @brief Broadcast entity spawn to all clients
         *
         * @param message Entity spawn data
         */
        void broadcastEntitySpawn(const network::EntitySpawnMessage& message);

        /**
         * @brief Broadcast entity state update to all clients
         *
         * @param message Entity state data
         */
        void broadcastEntityState(const network::EntityStateMessage& message);

        /**
         * @brief Broadcast entity destruction to all clients
         *
         * @param networkId Network ID of entity to destroy
         */
        void broadcastEntityDestroy(uint32_t networkId);

        /**
         * @brief Send full snapshot to a specific client
         *
         * @param client Client to send snapshot to
         */
        void sendSnapshotToClient(const ClientInfo& client);

        /**
         * @brief Get number of connected clients
         */
        size_t getClientCount() const;

        /**
         * @brief Set callback for client connection events
         * @param callback Function called when a client connects (clientId)
         */
        void setOnClientConnected(std::function<void(uint32_t)> callback) {
            m_onClientConnected = callback;
        }

        /**
         * @brief Set callback for client disconnection events
         * @param callback Function called when a client disconnects (clientId)
         */
        void setOnClientDisconnected(std::function<void(uint32_t)> callback) {
            m_onClientDisconnected = callback;
        }

        /**
         * @brief Set callback for client input events
         * @param callback Function called when client input is received
         */
        void setOnClientInput(std::function<void(uint32_t, const network::ClientInputMessage&)> callback) {
            m_onClientInput = callback;
        }

        /**
         * @brief Broadcast to all clients (public for PlayerManager)
         */
        void broadcast(const std::vector<uint8_t>& buffer);

    private:
        /**
         * @brief Reception loop (runs in separate thread)
         */
        void receiveLoop();

        /**
         * @brief Process received message
         *
         * @param buffer Received data
         * @param senderEndpoint Sender endpoint
         */
        void processMessage(const std::vector<uint8_t>& buffer,
                          const asio::ip::udp::endpoint& senderEndpoint);

        /**
         * @brief Handle CLIENT_HELLO message
         */
        void handleClientHello(const network::ClientHelloMessage& message,
                             const asio::ip::udp::endpoint& senderEndpoint);

        /**
         * @brief Handle CLIENT_DISCONNECT message
         */
        void handleClientDisconnect(const network::ClientDisconnectMessage& message);

        /**
         * @brief Handle CLIENT_INPUT message
         */
        void handleClientInput(const network::ClientInputMessage& message,
                              const asio::ip::udp::endpoint& senderEndpoint);

        /**
         * @brief Send message to specific endpoint
         *
         * @param buffer Serialized message
         * @param endpoint Destination endpoint
         */
        void sendTo(const std::vector<uint8_t>& buffer,
                   const asio::ip::udp::endpoint& endpoint);

        /**
         * @brief Find client by endpoint
         *
         * @return Pointer to client info, or nullptr if not found
         */
        ClientInfo* findClient(const asio::ip::udp::endpoint& endpoint);

        /**
         * @brief Find client by ID
         *
         * @return Pointer to client info, or nullptr if not found
         */
        ClientInfo* findClient(uint32_t clientId);

        // ECS
        ecs::Registry& m_registry;

        // ASIO networking
        asio::io_context m_ioContext;
        asio::ip::udp::socket m_socket;
        asio::ip::udp::endpoint m_remoteEndpoint;

        // Threading
        std::thread m_receiveThread;
        std::mutex m_clientsMutex;
        bool m_running;

        // Clients
        std::vector<ClientInfo> m_clients;
        uint32_t m_nextClientId;

        // Network update rate
        static constexpr float NETWORK_UPDATE_RATE = 0.05f;  // 20 Hz (50ms)
        float m_timeSinceLastUpdate;

        // Server port
        uint16_t m_port;

        // Callbacks
        std::function<void(uint32_t)> m_onClientConnected;
        std::function<void(uint32_t)> m_onClientDisconnected;
        std::function<void(uint32_t, const network::ClientInputMessage&)> m_onClientInput;
    };

} // namespace rtype::server
