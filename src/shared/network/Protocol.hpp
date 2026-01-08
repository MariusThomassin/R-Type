/*
** R-Type Network Protocol
** Binary protocol for client-server communication
*/

#pragma once

#include <cstdint>
#include <vector>
#include <cstring>

namespace rtype::network {

    /**
     * @brief Message types for network communication
     */
    enum class MessageType : uint8_t {
        // Client → Server
        CLIENT_HELLO = 0,        // Initial connection
        CLIENT_INPUT = 1,        // Player input
        CLIENT_DISCONNECT = 2,   // Clean disconnect
        PLAYER_READY = 3,        // Player clicked Play button and is ready
        CHAT_MESSAGE = 4,        // Chat message from player
        PAUSE_REQUEST = 5,       // Request to pause/unpause

        // Server → Client
        SERVER_WELCOME = 10,     // Connection response (assigns clientId)
        ENTITY_SPAWN = 11,       // Spawn new entity
        ENTITY_STATE = 12,       // Update position/velocity
        ENTITY_DESTROY = 13,     // Destroy entity
        SERVER_SNAPSHOT = 14,    // Full world state snapshot
        PLAYER_SPAWN = 15,       // Spawn player entity
        PLAYER_HIT = 16,         // Player took damage
        PLAYER_DEATH = 17,       // Player died (will respawn)
        PLAYER_RESPAWN = 18,     // Player respawned
        GAME_OVER = 19,          // Game over (all players dead)
        CHAT_BROADCAST = 20,     // Chat message broadcast to all
        PAUSE_STATE = 21,        // Server pause state change
        SCORE_UPDATE = 22        // Score update for a player
    };

    /**
     * @brief Entity types for network spawning
     */
    enum class EntityType : uint8_t {
        PROJECTILE = 0,
        PLAYER = 1,
        ENEMY = 2,
        POWERUP = 3
    };

    /**
     * @brief Message header (5 bytes)
     * Format: [MessageType:1][PayloadSize:4]
     */
    struct MessageHeader {
        MessageType type;
        uint32_t payloadSize;
    };

    // ============================================================
    // Client → Server Messages
    // ============================================================

    /**
     * @brief CLIENT_HELLO: Initial connection request
     */
    struct ClientHelloMessage {
        uint32_t protocolVersion = 1;
        char playerName[32] = {0};  // For future use
    };

    /**
     * @brief CLIENT_DISCONNECT: Clean disconnect notification
     */
    struct ClientDisconnectMessage {
        uint32_t clientId;
    };

    /**
     * @brief Input flags for player controls (bitfield)
     */
    enum InputFlags : uint8_t {
        INPUT_NONE  = 0,        // 0x00
        INPUT_UP    = 1 << 0,   // 0x01
        INPUT_DOWN  = 1 << 1,   // 0x02
        INPUT_LEFT  = 1 << 2,   // 0x04
        INPUT_RIGHT = 1 << 3,   // 0x08
        INPUT_SHOOT = 1 << 4    // 0x10
    };

    /**
     * @brief CLIENT_INPUT: Player input state
     */
    struct ClientInputMessage {
        uint32_t sequenceNumber;    // For prediction/reconciliation (future)
        uint8_t inputFlags;          // Bitfield: UP|DOWN|LEFT|RIGHT|SHOOT
        float deltaTime;             // For lag compensation
    };

    /**
     * @brief PLAYER_READY: Player has clicked Play and is ready to start
     * 
     * This message is sent when the player clicks the Play button in the main menu.
     * The game starts only when at least 2 clients have sent this message.
     * This ensures that the game begins with enough players for multiplayer gameplay.
     */
    struct PlayerReadyMessage {
        uint32_t clientId;          // Client ID that is ready
    };

    /**
     * @brief CHAT_MESSAGE: Chat message from client
     */
    struct ChatMessagePacket {
        uint32_t clientId;          // Sender client ID
        char message[128];          // Message text (null-terminated)
    };

    /**
     * @brief PAUSE_REQUEST: Client requests pause state change
     */
    struct PauseRequestMessage {
        uint32_t clientId;          // Requesting client
        bool pause;                 // true = pause, false = unpause
    };

    // ============================================================
    // Server → Client Messages
    // ============================================================

    /**
     * @brief SERVER_WELCOME: Connection acknowledgment
     */
    struct ServerWelcomeMessage {
        uint32_t clientId;          // Assigned client ID
        uint32_t protocolVersion;
        float serverTime;           // For time synchronization
    };

    /**
     * @brief ENTITY_SPAWN: Spawn new entity on clients
     */
    struct EntitySpawnMessage {
        uint32_t networkId;
        EntityType entityType;

        // Transform
        float x, y;
        float rotation;

        // Velocity
        float vx, vy;

        // Trajectory (for projectiles)
        uint8_t trajectoryType;     // 0=None, 1=Linear, 2=Sinusoidal, etc.
        float trajectoryParam1;     // Amplitude, radius, etc.
        float trajectoryParam2;     // Frequency, speed, etc.

        // Spin
        float spinSpeed;

        // Lifetime
        float maxLifetime;

        // Collider
        float colliderWidth;
        float colliderHeight;
        uint32_t collisionLayer;
        uint32_t collisionMask;
    };

    /**
     * @brief ENTITY_STATE: Update entity transform/velocity
     */
    struct EntityStateMessage {
        uint32_t networkId;
        float x, y;
        float vx, vy;
        float rotation;
    };

    /**
     * @brief ENTITY_DESTROY: Remove entity from clients
     */
    struct EntityDestroyMessage {
        uint32_t networkId;
    };

    /**
     * @brief SERVER_SNAPSHOT: Complete world state (for new clients)
     */
    struct ServerSnapshotMessage {
        uint32_t entityCount;
        float serverTime;
        // Followed by entityCount × EntitySpawnMessage
    };

    /**
     * @brief PLAYER_SPAWN: Spawn player entity
     */
    struct PlayerSpawnMessage {
        uint32_t networkId;
        uint32_t clientId;      // Which client owns this player
        uint8_t playerSlot;     // 0-3 (up to 4 players)
        float x, y;
        float health;
    };

    /**
     * @brief PLAYER_HIT: Player took damage
     */
    struct PlayerHitMessage {
        uint32_t networkId;
        float newHealth;
        float hitX, hitY;       // Impact position (for visual effect)
    };

    /**
     * @brief PLAYER_DEATH: Player died (will respawn after delay)
     */
    struct PlayerDeathMessage {
        uint32_t networkId;
        uint8_t remainingLives;  // Lives left after death
        float deathX, deathY;    // Death position
    };

    /**
     * @brief PLAYER_RESPAWN: Player respawned
     */
    struct PlayerRespawnMessage {
        uint32_t networkId;
        uint8_t playerSlot;
        float x, y;
        float health;
    };

    /**
     * @brief GAME_OVER: All players are dead
     */
    struct GameOverMessage {
        uint8_t survivorCount;  // Number of players still alive (0 for game over)
    };

    /**
     * @brief CHAT_BROADCAST: Chat message broadcast to all clients
     */
    struct ChatBroadcastMessage {
        uint32_t senderId;          // Original sender client ID
        char senderName[32];        // Sender display name
        char message[128];          // Message text (null-terminated)
    };

    /**
     * @brief PAUSE_STATE: Server broadcasts pause state change
     */
    struct PauseStateMessage {
        bool isPaused;              // Current pause state
        uint32_t requesterId;       // Client who triggered the pause
    };

    /**
     * @brief SCORE_UPDATE: Server broadcasts score change
     */
    struct ScoreUpdateMessage {
        uint32_t clientId;          // Client whose score changed
        int32_t newScore;           // New total score
        int32_t delta;              // Points added
    };

    // ============================================================
    // Serialization utilities
    // ============================================================

    /**
     * @brief Serialize a message into a byte buffer
     *
     * @tparam T Message structure type
     * @param type Message type identifier
     * @param message Message data
     * @return Serialized byte buffer with header
     */
    template<typename T>
    std::vector<uint8_t> serializeMessage(MessageType type, const T& message) {
        std::vector<uint8_t> buffer;

        // Reserve space: header (5 bytes) + payload
        buffer.resize(sizeof(MessageHeader) + sizeof(T));

        // Write header
        MessageHeader header;
        header.type = type;
        header.payloadSize = sizeof(T);

        std::memcpy(buffer.data(), &header, sizeof(MessageHeader));

        // Write payload
        std::memcpy(buffer.data() + sizeof(MessageHeader), &message, sizeof(T));

        return buffer;
    }

    /**
     * @brief Deserialize message header from byte buffer
     *
     * @param buffer Input byte buffer
     * @param outHeader Output header
     * @return true if successful
     */
    inline bool deserializeHeader(const std::vector<uint8_t>& buffer, MessageHeader& outHeader) {
        if (buffer.size() < sizeof(MessageHeader)) {
            return false;
        }

        std::memcpy(&outHeader, buffer.data(), sizeof(MessageHeader));
        return true;
    }

    /**
     * @brief Deserialize message payload from byte buffer
     *
     * @tparam T Message structure type
     * @param buffer Input byte buffer
     * @param outMessage Output message
     * @return true if successful
     */
    template<typename T>
    bool deserializeMessage(const std::vector<uint8_t>& buffer, T& outMessage) {
        if (buffer.size() < sizeof(MessageHeader) + sizeof(T)) {
            return false;
        }

        std::memcpy(&outMessage, buffer.data() + sizeof(MessageHeader), sizeof(T));
        return true;
    }

    /**
     * @brief Get message type name for logging
     */
    inline const char* getMessageTypeName(MessageType type) {
        switch (type) {
            case MessageType::CLIENT_HELLO: return "CLIENT_HELLO";
            case MessageType::CLIENT_INPUT: return "CLIENT_INPUT";
            case MessageType::CLIENT_DISCONNECT: return "CLIENT_DISCONNECT";
            case MessageType::PLAYER_READY: return "PLAYER_READY";
            case MessageType::CHAT_MESSAGE: return "CHAT_MESSAGE";
            case MessageType::PAUSE_REQUEST: return "PAUSE_REQUEST";
            case MessageType::SERVER_WELCOME: return "SERVER_WELCOME";
            case MessageType::ENTITY_SPAWN: return "ENTITY_SPAWN";
            case MessageType::ENTITY_STATE: return "ENTITY_STATE";
            case MessageType::ENTITY_DESTROY: return "ENTITY_DESTROY";
            case MessageType::SERVER_SNAPSHOT: return "SERVER_SNAPSHOT";
            case MessageType::PLAYER_SPAWN: return "PLAYER_SPAWN";
            case MessageType::PLAYER_HIT: return "PLAYER_HIT";
            case MessageType::PLAYER_DEATH: return "PLAYER_DEATH";
            case MessageType::PLAYER_RESPAWN: return "PLAYER_RESPAWN";
            case MessageType::GAME_OVER: return "GAME_OVER";
            case MessageType::CHAT_BROADCAST: return "CHAT_BROADCAST";
            case MessageType::PAUSE_STATE: return "PAUSE_STATE";
            case MessageType::SCORE_UPDATE: return "SCORE_UPDATE";
            default: return "UNKNOWN";
        }
    }

} // namespace rtype::network
