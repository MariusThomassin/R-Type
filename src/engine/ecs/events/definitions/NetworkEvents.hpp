/*
** R-Type ECS - Network Events
** Events for multiplayer networking
*/

#pragma once

#include "engine/ecs/core/EntityTypes.hpp"
#include <vector>
#include <string>

namespace rtype::ecs::events {

    /**
     * @brief Connection established with server
     */
    struct Connected {
        std::string serverAddress;
        int port;
        int playerId;
    };

    /**
     * @brief Disconnected from server
     */
    struct Disconnected {
        enum class Reason { Normal, Timeout, Kicked, ServerClosed, Error } reason;
        std::string message;
    };

    /**
     * @brief Another player joined
     */
    struct PlayerJoined {
        int playerId;
        std::string playerName;
        EntityId entity;
    };

    /**
     * @brief Another player left
     */
    struct PlayerLeft {
        int playerId;
        std::string playerName;
        enum class Reason { Disconnected, Kicked, Timeout } reason;
    };

    /**
     * @brief Received entity state update from server
     */
    struct EntityStateReceived {
        EntityId entity;
        float x, y;
        float velocityX, velocityY;
        int health;
        uint32_t timestamp;
    };

    /**
     * @brief Request to send input to server
     */
    struct SendInput {
        int playerId;
        uint32_t inputMask;
        uint32_t sequenceNumber;
    };

    /**
     * @brief Server acknowledged our input
     */
    struct InputAcknowledged {
        uint32_t sequenceNumber;
        float serverX, serverY;  // Server's authoritative position
    };

    /**
     * @brief Chat message received
     */
    struct ChatReceived {
        int senderId;
        std::string senderName;
        std::string message;
    };

    /**
     * @brief Game state sync from server
     */
    struct GameStateSync {
        int waveNumber;
        int score;
        std::vector<EntityId> activeEnemies;
        float gameTime;
    };

    /**
     * @brief Latency/ping update
     */
    struct LatencyUpdate {
        int pingMs;
        int jitterMs;
        float packetLoss;  // 0.0 to 1.0
    };

} // namespace rtype::ecs::events
