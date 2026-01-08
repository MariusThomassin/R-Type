/*
** R-Type Client - NetworkClient
** Handles network communication with the server
*/

#pragma once

#include "shared/network/Protocol.hpp"
#include "engine/ecs/core/Registry.hpp"
#include <asio.hpp>
#include <thread>
#include <mutex>
#include <queue>
#include <iostream>

namespace rtype::client {

    /**
     * @brief Message to be processed on main thread
     */
    struct PendingMessage {
        network::MessageType type;
        std::vector<uint8_t> data;
    };

    /**
     * @brief Manages network communication for the client
     *
     * Responsibilities:
     * - Connect to server via UDP
     * - Receive entity spawn/state/destroy messages
     * - Apply changes to local Registry
     * - Thread-safe message queue (receive thread → main thread)
     */
    class NetworkClient {
    public:
        /**
         * @brief Constructor
         *
         * @param registry Reference to ECS registry
         */
        explicit NetworkClient(ecs::Registry& registry);

        /**
         * @brief Destructor - disconnects from server
         */
        ~NetworkClient();

        /**
         * @brief Connect to server
         *
         * @param serverIp Server IP address
         * @param serverPort Server port
         * @return true if connection initiated successfully
         */
        bool connect(const std::string& serverIp, uint16_t serverPort);

        /**
         * @brief Disconnect from server
         */
        void disconnect();

        /**
         * @brief Update network state (called from main thread / game loop)
         *
         * Processes pending messages from the receive thread.
         * Call this every frame to apply network updates.
         */
        void update();

        /**
         * @brief Check if connected to server
         */
        bool isConnected() const { return m_connected && m_welcomeReceived; }

        /**
         * @brief Get assigned client ID
         */
        uint32_t getClientId() const { return m_clientId; }

        /**
         * @brief Send player input to server
         *
         * @param input Input message
         */
        void sendInput(const network::ClientInputMessage& input);

        /**
         * @brief Send player ready message to server
         *
         * Called when the player clicks Play button
         */
        void sendPlayerReady();

        /**
         * @brief Create a room on the server
         *
         * @param roomName Name of the room to create
         * @param maxPlayers Maximum players allowed
         * @param hasPassword Whether the room requires a password
         */
        void createRoom(const std::string& roomName, int maxPlayers = 4, bool hasPassword = false);

        /**
         * @brief Request list of available rooms from server
         *
         * @return Vector of room information (empty if not implemented on server)
         */
        std::vector<std::string> requestRoomList();

    private:
        /**
         * @brief Reception loop (runs in separate thread)
         */
        void receiveLoop();

        /**
         * @brief Process received message (called from receive thread)
         *
         * Pushes message to queue for main thread processing
         */
        void onMessageReceived(const std::vector<uint8_t>& buffer);

        /**
         * @brief Handle SERVER_WELCOME message (main thread)
         */
        void handleServerWelcome(const network::ServerWelcomeMessage& message);

        /**
         * @brief Handle ENTITY_SPAWN message (main thread)
         */
        void handleEntitySpawn(const network::EntitySpawnMessage& message);

        /**
         * @brief Handle ENTITY_STATE message (main thread)
         */
        void handleEntityState(const network::EntityStateMessage& message);

        /**
         * @brief Handle ENTITY_DESTROY message (main thread)
         */
        void handleEntityDestroy(const network::EntityDestroyMessage& message);

        /**
         * @brief Handle PLAYER_SPAWN message (main thread)
         */
        void handlePlayerSpawn(const network::PlayerSpawnMessage& message);

        /**
         * @brief Handle PLAYER_HIT message (main thread)
         */
        void handlePlayerHit(const network::PlayerHitMessage& message);

        /**
         * @brief Send CLIENT_HELLO to server
         */
        void sendHello();

        /**
         * @brief Send message to server
         */
        void sendToServer(const std::vector<uint8_t>& buffer);

        // ECS
        ecs::Registry& m_registry;

        // ASIO networking
        asio::io_context m_ioContext;
        asio::ip::udp::socket m_socket;
        asio::ip::udp::endpoint m_serverEndpoint;

        // Threading
        std::thread m_receiveThread;
        std::mutex m_queueMutex;
        std::queue<PendingMessage> m_messageQueue;
        bool m_running;

        // Connection state
        bool m_connected;
        bool m_welcomeReceived;  // true if we've received SERVER_WELCOME from server
        uint32_t m_clientId;

        // Network ID mapping (networkId → local Entity)
        std::unordered_map<uint32_t, ecs::Entity> m_networkIdToEntity;

        // Input sequence number
        uint32_t m_inputSequence;
    };

} // namespace rtype::client
