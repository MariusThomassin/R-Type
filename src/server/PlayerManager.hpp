/*
** R-Type Server - PlayerManager
** Manages player entities and input handling
*/

#pragma once

#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/Entity.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "shared/network/Protocol.hpp"
#include "NetworkManager.hpp"
#include "NetworkIdManager.hpp"
#include <map>
#include <set>

namespace rtype::server {

    /**
     * @brief Manages player spawning, input handling, and lifecycle
     *
     * Responsibilities:
     * - Spawn players when clients connect
     * - Apply client inputs to player entities
     * - Remove players when clients disconnect
     * - Broadcast player state updates
     */
    class PlayerManager {
    public:
        /**
         * @brief Construct PlayerManager
         * @param registry ECS registry
         * @param networkManager Network manager for broadcasting
         * @param networkIdManager Network ID manager for entity mapping
         */
        PlayerManager(ecs::Registry& registry,
                     NetworkManager& networkManager,
                     NetworkIdManager& networkIdManager);

        /**
         * @brief Spawn a new player for a client
         * @param clientId Client ID that owns this player
         * @return Entity ID of spawned player, or NULL_ENTITY if failed (game full)
         */
        ecs::Entity spawnPlayer(uint32_t clientId);

        /**
         * @brief Apply client input to their player
         * @param clientId Client ID
         * @param input Input message
         */
        void applyInput(uint32_t clientId, const network::ClientInputMessage& input);

        /**
         * @brief Remove player when client disconnects
         * @param clientId Client ID
         */
        void removePlayer(uint32_t clientId);

        /**
         * @brief Update all players (called from GameServer::tick)
         * @param dt Delta time
         */
        void update(float dt);

        /**
         * @brief Get player entity for a client
         * @param clientId Client ID
         * @return Entity ID, or NULL_ENTITY if not found
         */
        ecs::Entity getPlayerEntity(uint32_t clientId) const;

        /**
         * @brief Get number of active players
         */
        size_t getPlayerCount() const { return m_clientIdToPlayerEntity.size(); }

    private:
        /**
         * @brief Find first available player slot (0-3)
         * @return Slot number, or 255 if all slots taken
         */
        uint8_t findAvailableSlot() const;

        /**
         * @brief Get spawn position for player slot
         * @param slot Player slot (0-3)
         * @return Spawn position {x, y}
         */
        struct Vec2 { float x, y; };
        Vec2 getSpawnPosition(uint8_t slot) const;

        /**
         * @brief Clamp player position to screen bounds
         * @param entity Player entity
         */
        void clampToScreen(ecs::Entity entity);

        /**
         * @brief Spawn a projectile from player
         * @param player Player entity
         * @param clientId Client ID of player
         * @param transform Player transform
         * @param weapon Player weapon component
         */
        void spawnPlayerProjectile(ecs::Entity player, uint32_t clientId,
                                   const ecs::TransformComponent& transform,
                                   const ecs::WeaponComponent& weapon);

        ecs::Registry& m_registry;
        NetworkManager& m_networkManager;
        NetworkIdManager& m_networkIdManager;

        // Mapping: clientId → playerEntity
        std::map<uint32_t, ecs::Entity> m_clientIdToPlayerEntity;

        // Max players
        static constexpr size_t MAX_PLAYERS = 4;

        // Screen bounds
        static constexpr float SCREEN_WIDTH = 1280.0f;
        static constexpr float SCREEN_HEIGHT = 720.0f;
        static constexpr float MARGIN = 30.0f;

        // Player speed
        static constexpr float PLAYER_SPEED = 200.0f;

        // Game time tracker for weapon cooldowns
        float m_gameTime = 0.0f;
    };

} // namespace rtype::server
