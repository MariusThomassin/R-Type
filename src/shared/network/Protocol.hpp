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
        CLIENT_INPUT = 1,        // Player input (for later)
        CLIENT_DISCONNECT = 2,   // Clean disconnect

        // Server → Client
        SERVER_WELCOME = 10,     // Connection response (assigns clientId)
        ENTITY_SPAWN = 11,       // Spawn new entity
        ENTITY_STATE = 12,       // Update position/velocity
        ENTITY_DESTROY = 13,     // Destroy entity
        SERVER_SNAPSHOT = 14     // Full world state snapshot
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
            case MessageType::SERVER_WELCOME: return "SERVER_WELCOME";
            case MessageType::ENTITY_SPAWN: return "ENTITY_SPAWN";
            case MessageType::ENTITY_STATE: return "ENTITY_STATE";
            case MessageType::ENTITY_DESTROY: return "ENTITY_DESTROY";
            case MessageType::SERVER_SNAPSHOT: return "SERVER_SNAPSHOT";
            default: return "UNKNOWN";
        }
    }

} // namespace rtype::network
