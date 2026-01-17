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
#include <functional>

namespace rtype::client {

    /**
     * @brief Message to be processed on main thread
     */
    struct PendingMessage {
        network::MessageType type;
        std::vector<uint8_t> data;
    };

    /**
     * @brief Callbacks for level and game events
     */
    struct NetworkCallbacks {
        // Connection callback - called when client receives welcome from server
        std::function<void(uint32_t clientId)> onWelcome;
        
        std::function<void(const network::LevelInfoMessage&)> onLevelInfo;
        std::function<void(const network::LevelStartMessage&)> onLevelStart;
        std::function<void(const network::LevelCompleteMessage&)> onLevelComplete;
        std::function<void(const network::WaveStartMessage&)> onWaveStart;
        std::function<void(const network::WaveCompleteMessage&)> onWaveComplete;
        std::function<void(const network::BossStartMessage&)> onBossStart;
        std::function<void(const network::BossDefeatedMessage&)> onBossDefeated;
        std::function<void(const network::DifficultyChangeMessage&)> onDifficultyChange;
        std::function<void(const network::ScoreUpdateMessage&)> onScoreUpdate;
        
        // Room/Lobby callbacks
        std::function<void(const network::RoomCreatedMessage&)> onRoomCreated;
        std::function<void(const network::RoomJoinedMessage&)> onRoomJoined;
        std::function<void(const network::RoomLeftMessage&)> onRoomLeft;
        std::function<void(const network::RoomListMessage&)> onRoomList;
        std::function<void(const network::RoomInfoMessage&)> onRoomInfo;
        std::function<void(const network::HostChangedMessage&)> onHostChanged;
        std::function<void(const network::RoomErrorMessage&)> onRoomError;
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
         * @param maxPlayers Maximum players allowed (2-4)
         */
        void createRoom(const std::string& roomName, uint8_t maxPlayers = 4);

        /**
         * @brief Join an existing room
         *
         * @param roomName Name of the room to join
         */
        void joinRoom(const std::string& roomName);

        /**
         * @brief Leave current room
         */
        void leaveRoom();

        /**
         * @brief Request list of available rooms from server
         */
        void requestRoomList();

        /**
         * @brief Set callbacks for level and game events
         *
         * @param callbacks Struct containing callback functions
         */
        void setCallbacks(const NetworkCallbacks& callbacks) { m_callbacks = callbacks; }

        /**
         * @brief Send a LOAD_LEVEL_REQUEST to the server
         *
         * @param levelIndex Level to request (0-based)
         */
        void sendLoadLevelRequest(uint8_t levelIndex);

        /**
         * @brief Host starts the game (only works if this client is the host)
         *
         * @param levelIndex Level to start (0-based)
         */
        void hostStartGame(uint8_t levelIndex = 0);

        /**
         * @brief Check if this client is the host of the current room
         */
        bool isHost() const { return m_isHost; }

        /**
         * @brief Get current room name
         */
        const std::string& getCurrentRoomName() const { return m_currentRoomName; }

        /**
         * @brief Check if in a room
         */
        bool isInRoom() const { return !m_currentRoomName.empty(); }

        /**
         * @brief Send player profile to server
         *
         * @param name Player name (up to 9 chars)
         * @param avatarId Avatar index
         * @param colorScheme Color scheme index
         */
        void sendPlayerProfile(const char* name, uint8_t avatarId, uint8_t colorScheme);

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
         * @brief Handle level and game event messages (main thread)
         * These invoke the registered callbacks
         */
        void handleLevelInfo(const network::LevelInfoMessage& message);
        void handleLevelStart(const network::LevelStartMessage& message);
        void handleLevelComplete(const network::LevelCompleteMessage& message);
        void handleWaveStart(const network::WaveStartMessage& message);
        void handleWaveComplete(const network::WaveCompleteMessage& message);
        void handleBossStart(const network::BossStartMessage& message);
        void handleBossDefeated(const network::BossDefeatedMessage& message);
        void handleDifficultyChange(const network::DifficultyChangeMessage& message);
        void handleScoreUpdate(const network::ScoreUpdateMessage& message);

        // Room message handlers
        void handleRoomCreated(const network::RoomCreatedMessage& message);
        void handleRoomJoined(const network::RoomJoinedMessage& message);
        void handleRoomLeft(const network::RoomLeftMessage& message);
        void handleRoomList(const network::RoomListMessage& message);
        void handleRoomInfo(const network::RoomInfoMessage& message);
        void handleHostChanged(const network::HostChangedMessage& message);
        void handleRoomError(const network::RoomErrorMessage& message);

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

        // Pending state updates for entities that haven't spawned yet (networkId → EntityState)
        std::unordered_map<uint32_t, network::EntityStateMessage> m_pendingStateUpdates;

        // Input sequence number
        uint32_t m_inputSequence;

        // Callbacks for level/game events
        NetworkCallbacks m_callbacks;

        // Room/Lobby state
        std::string m_currentRoomName;
        bool m_isHost = false;
        uint8_t m_playerSlot = 0;
    };

} // namespace rtype::client
