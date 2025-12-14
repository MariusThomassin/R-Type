/*
** R-Type Server - NetworkIdManager
** Manages unique network identifiers for entities
*/

#pragma once

#include "engine/ecs/core/Entity.hpp"
#include <unordered_map>
#include <cstdint>

namespace rtype::server {

    /**
     * @brief Manages bidirectional mapping between Entity IDs and Network IDs
     *
     * Network IDs are unique identifiers sent over the network to synchronize
     * entities across client and server. They remain stable even if the local
     * Entity ID changes.
     */
    class NetworkIdManager {
    public:
        NetworkIdManager() = default;

        /**
         * @brief Allocate a new network ID for an entity
         *
         * @param entity Local entity ID
         * @return Assigned network ID (starting from 1)
         *
         * IMPORTANT: If an entity is reused by the Registry (entity ID recycled after destruction),
         * we MUST detect this and assign a NEW networkId, not reuse the old one.
         * We do this by checking if the entity already has a mapping, and if so,
         * removing it first to ensure a fresh allocation.
         */
        uint32_t allocate(ecs::Entity entity) {
            // Check if this entity ID was previously used (entity recycling)
            auto it = m_entityToNetworkId.find(entity);
            if (it != m_entityToNetworkId.end()) {
                // Entity ID was recycled! Remove the old mapping to prevent conflicts.
                // This can happen when the Registry reuses entity IDs after destruction.
                uint32_t oldNetworkId = it->second;
                std::cerr << "[NetworkIdManager] WARNING: Entity " << entity
                         << " was recycled with old networkId=" << oldNetworkId
                         << ". Removing stale mapping and allocating new ID." << std::endl;

                // Remove old mappings
                m_networkIdToEntity.erase(oldNetworkId);
                m_entityToNetworkId.erase(it);
            }

            // Allocate new ID (always increment, never reuse networkIds)
            uint32_t networkId = m_nextId++;
            m_entityToNetworkId[entity] = networkId;
            m_networkIdToEntity[networkId] = entity;

            return networkId;
        }

        /**
         * @brief Get network ID for an entity
         *
         * @param entity Local entity ID
         * @return Network ID, or 0 if not found
         */
        uint32_t getNetworkId(ecs::Entity entity) const {
            auto it = m_entityToNetworkId.find(entity);
            return (it != m_entityToNetworkId.end()) ? it->second : 0;
        }

        /**
         * @brief Get entity for a network ID
         *
         * @param networkId Network ID
         * @return Local entity ID, or ecs::NULL_ENTITY if not found
         */
        ecs::Entity getEntity(uint32_t networkId) const {
            auto it = m_networkIdToEntity.find(networkId);
            return (it != m_networkIdToEntity.end()) ? it->second : ecs::Entity(ecs::NULL_ENTITY);
        }

        /**
         * @brief Check if an entity has a network ID
         */
        bool hasNetworkId(ecs::Entity entity) const {
            return m_entityToNetworkId.find(entity) != m_entityToNetworkId.end();
        }

        /**
         * @brief Remove entity from network ID mapping
         *
         * @param entity Entity to remove
         */
        void remove(ecs::Entity entity) {
            auto it = m_entityToNetworkId.find(entity);
            if (it != m_entityToNetworkId.end()) {
                uint32_t networkId = it->second;
                m_networkIdToEntity.erase(networkId);
                m_entityToNetworkId.erase(it);
            }
        }

        /**
         * @brief Remove by network ID
         *
         * @param networkId Network ID to remove
         */
        void removeByNetworkId(uint32_t networkId) {
            auto it = m_networkIdToEntity.find(networkId);
            if (it != m_networkIdToEntity.end()) {
                ecs::Entity entity = it->second;
                m_entityToNetworkId.erase(entity);
                m_networkIdToEntity.erase(it);
            }
        }

        /**
         * @brief Get count of managed entities
         */
        size_t getEntityCount() const {
            return m_entityToNetworkId.size();
        }

        /**
         * @brief Clear all mappings (for server reset)
         */
        void clear() {
            m_entityToNetworkId.clear();
            m_networkIdToEntity.clear();
            m_nextId = 1;
        }

    private:
        uint32_t m_nextId = 1;  // Start from 1 (0 is reserved for "invalid")
        std::unordered_map<ecs::Entity, uint32_t> m_entityToNetworkId;
        std::unordered_map<uint32_t, ecs::Entity> m_networkIdToEntity;
    };

} // namespace rtype::server
