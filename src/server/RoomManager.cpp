/*
** R-Type Server - RoomManager Implementation
*/

#include "RoomManager.hpp"
#include <iostream>
#include <algorithm>

namespace rtype::server {

    RoomManager::RoomManager() = default;

    network::RoomError RoomManager::createRoom(uint32_t clientId, const std::string& roomName,
                                                const std::string& playerName, uint8_t maxPlayers) {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Validate room name
        if (roomName.empty() || roomName.length() > 31) {
            return network::RoomError::INVALID_ROOM_NAME;
        }

        // Check if client already in a room
        if (m_clientToRoom.count(clientId) > 0) {
            return network::RoomError::ALREADY_IN_ROOM;
        }

        // Check if room name already exists
        if (m_rooms.count(roomName) > 0) {
            return network::RoomError::INVALID_ROOM_NAME; // Room name taken
        }

        // Clamp max players
        maxPlayers = std::clamp(maxPlayers, uint8_t(2), uint8_t(4));

        // Create room with creator as host
        Room room(roomName, clientId, maxPlayers);
        room.players.emplace_back(clientId, playerName, 0); // Host gets slot 0
        room.players.back().isReady = false;

        m_rooms.emplace(roomName, std::move(room));
        m_clientToRoom[clientId] = roomName;

        std::cout << "[RoomManager] Room '" << roomName << "' created by client " 
                  << clientId << " (host)" << std::endl;

        // Send ROOM_CREATED to the creator
        network::RoomCreatedMessage createdMsg{};
        std::strncpy(createdMsg.roomName, roomName.c_str(), sizeof(createdMsg.roomName) - 1);
        createdMsg.hostClientId = clientId;
        auto buffer = network::serializeMessage(network::MessageType::ROOM_CREATED, createdMsg);
        sendToClient(clientId, buffer);

        // Send ROOM_JOINED to confirm
        network::RoomJoinedMessage joinedMsg{};
        std::strncpy(joinedMsg.roomName, roomName.c_str(), sizeof(joinedMsg.roomName) - 1);
        joinedMsg.hostClientId = clientId;
        joinedMsg.playerCount = 1;
        joinedMsg.maxPlayers = maxPlayers;
        joinedMsg.yourSlot = 0;
        joinedMsg.youAreHost = true;
        auto joinBuffer = network::serializeMessage(network::MessageType::ROOM_JOINED, joinedMsg);
        sendToClient(clientId, joinBuffer);

        return network::RoomError::NONE;
    }

    network::RoomError RoomManager::joinRoom(uint32_t clientId, const std::string& roomName,
                                              const std::string& playerName) {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Check if client already in a room
        if (m_clientToRoom.count(clientId) > 0) {
            return network::RoomError::ALREADY_IN_ROOM;
        }

        // Find the room, or auto-create it if it doesn't exist
        auto it = m_rooms.find(roomName);
        if (it == m_rooms.end()) {
            // Auto-create the room with this player as host
            std::cout << "[RoomManager] Room '" << roomName << "' not found, auto-creating..." << std::endl;
            
            // Use emplace to construct Room in-place with proper constructor
            auto result = m_rooms.emplace(roomName, Room(roomName, clientId, 4));
            Room& newRoom = result.first->second;
            newRoom.players.emplace_back(clientId, playerName, 0);  // Slot 0 for host
            m_clientToRoom[clientId] = roomName;

            std::cout << "[RoomManager] Room '" << roomName << "' created with host " << clientId << std::endl;

            // Send ROOM_JOINED to the creator (who is also the host)
            network::RoomJoinedMessage joinedMsg{};
            std::strncpy(joinedMsg.roomName, roomName.c_str(), sizeof(joinedMsg.roomName) - 1);
            joinedMsg.hostClientId = clientId;
            joinedMsg.playerCount = 1;
            joinedMsg.maxPlayers = 4;
            joinedMsg.yourSlot = 0;
            joinedMsg.youAreHost = true;
            auto buffer = network::serializeMessage(network::MessageType::ROOM_JOINED, joinedMsg);
            sendToClient(clientId, buffer);

            // Broadcast room info to the new host
            broadcastRoomInfo(newRoom);

            return network::RoomError::NONE;
        }

        Room& room = it->second;

        // Check if room is full
        if (room.isFull()) {
            return network::RoomError::ROOM_FULL;
        }

        // Check if game already started
        if (room.state != network::RoomState::LOBBY) {
            return network::RoomError::GAME_ALREADY_STARTED;
        }

        // Find available slot
        uint8_t slot = room.findAvailableSlot();
        if (slot == 255) {
            return network::RoomError::ROOM_FULL;
        }

        // Add player to room
        room.players.emplace_back(clientId, playerName, slot);
        m_clientToRoom[clientId] = roomName;

        std::cout << "[RoomManager] Client " << clientId << " joined room '" 
                  << roomName << "' (slot " << static_cast<int>(slot) << ")" << std::endl;

        // Send ROOM_JOINED to the joining player
        network::RoomJoinedMessage joinedMsg{};
        std::strncpy(joinedMsg.roomName, roomName.c_str(), sizeof(joinedMsg.roomName) - 1);
        joinedMsg.hostClientId = room.hostClientId;
        joinedMsg.playerCount = static_cast<uint8_t>(room.players.size());
        joinedMsg.maxPlayers = room.maxPlayers;
        joinedMsg.yourSlot = slot;
        joinedMsg.youAreHost = false;
        auto buffer = network::serializeMessage(network::MessageType::ROOM_JOINED, joinedMsg);
        sendToClient(clientId, buffer);

        // Broadcast updated room info to all players in room
        broadcastRoomInfo(room);

        return network::RoomError::NONE;
    }

    bool RoomManager::leaveRoom(uint32_t clientId) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto clientIt = m_clientToRoom.find(clientId);
        if (clientIt == m_clientToRoom.end()) {
            return false; // Not in a room
        }

        std::string roomName = clientIt->second;
        auto roomIt = m_rooms.find(roomName);
        if (roomIt == m_rooms.end()) {
            m_clientToRoom.erase(clientIt);
            return false;
        }

        Room& room = roomIt->second;
        bool wasHost = (room.hostClientId == clientId);

        // Remove player from room
        room.players.erase(
            std::remove_if(room.players.begin(), room.players.end(),
                [clientId](const RoomPlayer& p) { return p.clientId == clientId; }),
            room.players.end()
        );
        m_clientToRoom.erase(clientId);

        std::cout << "[RoomManager] Client " << clientId << " left room '" << roomName << "'" << std::endl;

        // Send ROOM_LEFT to the leaving player
        network::RoomLeftMessage leftMsg{};
        leftMsg.clientId = clientId;
        auto buffer = network::serializeMessage(network::MessageType::ROOM_LEFT, leftMsg);
        sendToClient(clientId, buffer);

        // If room is empty, delete it
        if (room.isEmpty()) {
            std::cout << "[RoomManager] Room '" << roomName << "' is empty, removing" << std::endl;
            m_rooms.erase(roomIt);
            return true;
        }

        // If host left, promote next player
        if (wasHost) {
            promoteNextHost(room);
        }

        // Broadcast updated room info
        broadcastRoomInfo(room);

        return true;
    }

    void RoomManager::handleClientDisconnect(uint32_t clientId) {
        // leaveRoom handles everything including host migration
        leaveRoom(clientId);
    }

    void RoomManager::setPlayerReady(uint32_t clientId, bool ready) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto clientIt = m_clientToRoom.find(clientId);
        if (clientIt == m_clientToRoom.end()) {
            return;
        }

        auto roomIt = m_rooms.find(clientIt->second);
        if (roomIt == m_rooms.end()) {
            return;
        }

        Room& room = roomIt->second;
        RoomPlayer* player = room.findPlayer(clientId);
        if (player) {
            player->isReady = ready;
            std::cout << "[RoomManager] Client " << clientId << " is " 
                      << (ready ? "ready" : "not ready") << std::endl;
            broadcastRoomInfo(room);
        }
    }

    network::RoomError RoomManager::hostStartGame(uint32_t clientId, uint8_t levelIndex) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto clientIt = m_clientToRoom.find(clientId);
        if (clientIt == m_clientToRoom.end()) {
            return network::RoomError::ROOM_NOT_FOUND;
        }

        auto roomIt = m_rooms.find(clientIt->second);
        if (roomIt == m_rooms.end()) {
            return network::RoomError::ROOM_NOT_FOUND;
        }

        Room& room = roomIt->second;

        // Only host can start
        if (room.hostClientId != clientId) {
            std::cout << "[RoomManager] Client " << clientId 
                      << " tried to start game but is not host" << std::endl;
            return network::RoomError::NOT_HOST;
        }

        // Check if already started
        if (room.state != network::RoomState::LOBBY) {
            return network::RoomError::GAME_ALREADY_STARTED;
        }

        // Start the game
        room.state = network::RoomState::PLAYING;
        std::cout << "[RoomManager] Host " << clientId << " started game in room '" 
                  << room.name << "' (level " << static_cast<int>(levelIndex) << ")" << std::endl;

        // Broadcast room info with new state
        broadcastRoomInfo(room);

        // Notify GameServer via callback
        if (m_hostStartCallback) {
            m_hostStartCallback(room.name, clientId, levelIndex);
        }

        return network::RoomError::NONE;
    }

    Room* RoomManager::getRoomForClient(uint32_t clientId) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto clientIt = m_clientToRoom.find(clientId);
        if (clientIt == m_clientToRoom.end()) {
            return nullptr;
        }

        auto roomIt = m_rooms.find(clientIt->second);
        return (roomIt != m_rooms.end()) ? &roomIt->second : nullptr;
    }

    const Room* RoomManager::getRoomForClient(uint32_t clientId) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        auto clientIt = m_clientToRoom.find(clientId);
        if (clientIt == m_clientToRoom.end()) {
            return nullptr;
        }

        auto roomIt = m_rooms.find(clientIt->second);
        return (roomIt != m_rooms.end()) ? &roomIt->second : nullptr;
    }

    Room* RoomManager::getRoom(const std::string& roomName) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_rooms.find(roomName);
        return (it != m_rooms.end()) ? &it->second : nullptr;
    }

    const Room* RoomManager::getRoom(const std::string& roomName) const {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_rooms.find(roomName);
        return (it != m_rooms.end()) ? &it->second : nullptr;
    }

    bool RoomManager::isHost(uint32_t clientId) const {
        const Room* room = getRoomForClient(clientId);
        return room && room->hostClientId == clientId;
    }

    std::vector<const Room*> RoomManager::getAvailableRooms() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<const Room*> result;
        for (const auto& [name, room] : m_rooms) {
            if (!room.isFull() && room.state == network::RoomState::LOBBY) {
                result.push_back(&room);
            }
        }
        return result;
    }

    std::vector<const Room*> RoomManager::getAllRooms() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        std::vector<const Room*> result;
        for (const auto& [name, room] : m_rooms) {
            result.push_back(&room);
        }
        return result;
    }

    network::RoomListMessage RoomManager::buildRoomListMessage() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        network::RoomListMessage msg{};
        msg.roomCount = 0;

        for (const auto& [name, room] : m_rooms) {
            if (msg.roomCount >= 16) break;

            network::RoomSummary& summary = msg.rooms[msg.roomCount];
            std::strncpy(summary.roomName, name.c_str(), sizeof(summary.roomName) - 1);
            summary.playerCount = static_cast<uint8_t>(room.players.size());
            summary.maxPlayers = room.maxPlayers;
            summary.state = room.state;
            msg.roomCount++;
        }

        return msg;
    }

    network::RoomInfoMessage RoomManager::buildRoomInfoMessage(const Room& room) const {
        network::RoomInfoMessage msg{};
        std::strncpy(msg.roomName, room.name.c_str(), sizeof(msg.roomName) - 1);
        msg.hostClientId = room.hostClientId;
        msg.playerCount = static_cast<uint8_t>(room.players.size());
        msg.maxPlayers = room.maxPlayers;
        msg.state = room.state;

        for (size_t i = 0; i < room.players.size() && i < 4; i++) {
            const RoomPlayer& p = room.players[i];
            network::RoomPlayerInfo& info = msg.players[i];
            info.clientId = p.clientId;
            std::strncpy(info.playerName, p.playerName.c_str(), sizeof(info.playerName) - 1);
            info.slot = p.slot;
            info.isReady = p.isReady;
            info.isHost = (p.clientId == room.hostClientId);
        }

        return msg;
    }

    bool RoomManager::promoteNextHost(Room& room) {
        if (room.players.empty()) {
            return false;
        }

        // Find next player (first in list becomes new host)
        uint32_t newHostId = room.players[0].clientId;
        room.hostClientId = newHostId;

        std::cout << "[RoomManager] Host migrated to client " << newHostId 
                  << " in room '" << room.name << "'" << std::endl;

        // Send HOST_CHANGED to all players in room
        network::HostChangedMessage hostMsg{};
        hostMsg.newHostClientId = newHostId;
        std::strncpy(hostMsg.reason, "Previous host left", sizeof(hostMsg.reason) - 1);
        auto buffer = network::serializeMessage(network::MessageType::HOST_CHANGED, hostMsg);

        for (const auto& player : room.players) {
            sendToClient(player.clientId, buffer);
        }

        return true;
    }

    void RoomManager::broadcastRoomInfo(const Room& room) {
        network::RoomInfoMessage infoMsg = buildRoomInfoMessage(room);
        auto buffer = network::serializeMessage(network::MessageType::ROOM_INFO, infoMsg);

        for (const auto& player : room.players) {
            sendToClient(player.clientId, buffer);
        }
    }

    void RoomManager::sendToClient(uint32_t clientId, const std::vector<uint8_t>& data) {
        if (m_broadcastCallback) {
            m_broadcastCallback(clientId, data);
        }
    }

    void RoomManager::cleanupEmptyRooms() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto it = m_rooms.begin(); it != m_rooms.end(); ) {
            if (it->second.isEmpty()) {
                std::cout << "[RoomManager] Cleaning up empty room '" << it->first << "'" << std::endl;
                it = m_rooms.erase(it);
            } else {
                ++it;
            }
        }
    }

} // namespace rtype::server
