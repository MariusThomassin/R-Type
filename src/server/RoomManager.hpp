/*
** R-Type Server - RoomManager
** Manages game rooms/lobbies for multiplayer
*/

#pragma once

#include "shared/network/Protocol.hpp"
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <functional>
#include <cstring>

namespace rtype::server {

    /**
     * @brief Represents a player in a room
     */
    struct RoomPlayer {
        uint32_t clientId;
        std::string playerName;
        uint8_t slot;           // 0-3
        bool isReady;

        RoomPlayer(uint32_t id, const std::string& name, uint8_t s)
            : clientId(id), playerName(name), slot(s), isReady(false) {}
    };

    /**
     * @brief Represents a game room/lobby
     */
    struct Room {
        std::string name;
        uint32_t hostClientId;
        uint8_t maxPlayers;
        network::RoomState state;
        std::vector<RoomPlayer> players;

        Room(const std::string& roomName, uint32_t hostId, uint8_t max = 4)
            : name(roomName)
            , hostClientId(hostId)
            , maxPlayers(max)
            , state(network::RoomState::LOBBY)
        {}

        bool isFull() const { return players.size() >= maxPlayers; }
        bool isEmpty() const { return players.empty(); }
        
        RoomPlayer* findPlayer(uint32_t clientId) {
            for (auto& p : players) {
                if (p.clientId == clientId) return &p;
            }
            return nullptr;
        }

        const RoomPlayer* findPlayer(uint32_t clientId) const {
            for (const auto& p : players) {
                if (p.clientId == clientId) return &p;
            }
            return nullptr;
        }

        uint8_t findAvailableSlot() const {
            bool usedSlots[4] = {false, false, false, false};
            for (const auto& p : players) {
                if (p.slot < 4) usedSlots[p.slot] = true;
            }
            for (uint8_t i = 0; i < 4; i++) {
                if (!usedSlots[i]) return i;
            }
            return 255; // No slot available
        }

        bool allPlayersReady() const {
            if (players.empty()) return false;
            for (const auto& p : players) {
                if (!p.isReady) return false;
            }
            return true;
        }
    };

    /**
     * @brief Manages game rooms for multiplayer lobbying
     * 
     * Responsibilities:
     * - Create/destroy rooms
     * - Handle player join/leave
     * - Track host (first player to join)
     * - Handle host migration on disconnect
     * - Broadcast room state changes
     */
    class RoomManager {
    public:
        using BroadcastCallback = std::function<void(uint32_t clientId, const std::vector<uint8_t>& data)>;
        using HostStartCallback = std::function<void(const std::string& roomName, uint32_t hostClientId, uint8_t levelIndex)>;

        RoomManager();
        ~RoomManager() = default;

        /**
         * @brief Set callback for sending messages to specific clients
         */
        void setBroadcastCallback(BroadcastCallback callback) {
            m_broadcastCallback = callback;
        }

        /**
         * @brief Set callback for when host starts the game
         */
        void setHostStartCallback(HostStartCallback callback) {
            m_hostStartCallback = callback;
        }

        /**
         * @brief Create a new room
         * @param clientId Client creating the room (becomes host)
         * @param roomName Name for the room
         * @param maxPlayers Maximum players (2-4)
         * @return RoomError::NONE on success
         */
        network::RoomError createRoom(uint32_t clientId, const std::string& roomName, 
                                       const std::string& playerName, uint8_t maxPlayers = 4);

        /**
         * @brief Join an existing room
         * @param clientId Client joining
         * @param roomName Room to join
         * @param playerName Player's display name
         * @return RoomError::NONE on success
         */
        network::RoomError joinRoom(uint32_t clientId, const std::string& roomName,
                                     const std::string& playerName);

        /**
         * @brief Leave current room
         * @param clientId Client leaving
         * @return true if client was in a room
         */
        bool leaveRoom(uint32_t clientId);

        /**
         * @brief Handle client disconnect (calls leaveRoom + host migration)
         * @param clientId Disconnecting client
         */
        void handleClientDisconnect(uint32_t clientId);

        /**
         * @brief Set player ready state
         * @param clientId Client ID
         * @param ready Ready state
         */
        void setPlayerReady(uint32_t clientId, bool ready);

        /**
         * @brief Host starts the game
         * @param clientId Must be the host
         * @param levelIndex Level to start
         * @return RoomError::NONE on success
         */
        network::RoomError hostStartGame(uint32_t clientId, uint8_t levelIndex);

        /**
         * @brief Get room for a client
         * @param clientId Client ID
         * @return Room pointer or nullptr
         */
        Room* getRoomForClient(uint32_t clientId);
        const Room* getRoomForClient(uint32_t clientId) const;

        /**
         * @brief Get room by name
         * @param roomName Room name
         * @return Room pointer or nullptr
         */
        Room* getRoom(const std::string& roomName);
        const Room* getRoom(const std::string& roomName) const;

        /**
         * @brief Check if a client is the host of their room
         */
        bool isHost(uint32_t clientId) const;

        /**
         * @brief Get all available rooms (not full, in lobby state)
         */
        std::vector<const Room*> getAvailableRooms() const;

        /**
         * @brief Get all rooms
         */
        std::vector<const Room*> getAllRooms() const;

        /**
         * @brief Build room list message for a client
         */
        network::RoomListMessage buildRoomListMessage() const;

        /**
         * @brief Build room info message for a room
         */
        network::RoomInfoMessage buildRoomInfoMessage(const Room& room) const;

    private:
        /**
         * @brief Promote next player to host (host migration)
         * @param room Room to update
         * @return true if new host was assigned
         */
        bool promoteNextHost(Room& room);

        /**
         * @brief Broadcast room info to all players in room
         */
        void broadcastRoomInfo(const Room& room);

        /**
         * @brief Send message to a specific client
         */
        void sendToClient(uint32_t clientId, const std::vector<uint8_t>& data);

        /**
         * @brief Remove empty rooms
         */
        void cleanupEmptyRooms();

        // Room storage: roomName -> Room
        std::map<std::string, Room> m_rooms;

        // Quick lookup: clientId -> roomName
        std::map<uint32_t, std::string> m_clientToRoom;

        // Thread safety
        mutable std::mutex m_mutex;

        // Callbacks
        BroadcastCallback m_broadcastCallback;
        HostStartCallback m_hostStartCallback;
    };

} // namespace rtype::server
