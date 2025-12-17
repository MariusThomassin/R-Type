/*
** R-Type ECS - NetworkComponent
** Network synchronization data
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Component holding network synchronization data
     *
     * Used by entities that need to be synchronized across
     * client/server in multiplayer.
     */
    struct NetworkComponent : public IComponent {
        uint32_t networkId = 0;       // Unique network identifier (0 = not assigned)
        bool isOwned = false;         // Is this client the owner?
        bool needsSync = false;       // Dirty flag for sync
        float lastSyncTime = 0.0f;    // Last time entity was synced
        float syncInterval = 0.05f;   // Minimum time between syncs (20 Hz)

        // Interpolation data (for client-side smoothing)
        float interpX = 0.0f;
        float interpY = 0.0f;
        float interpProgress = 1.0f;

        NetworkComponent() = default;

        explicit NetworkComponent(uint32_t netId)
            : networkId(netId) {}

        NetworkComponent(uint32_t netId, bool owned)
            : networkId(netId), isOwned(owned) {}

        /**
         * @brief Check if sync is needed based on interval
         */
        bool shouldSync(float currentTime) const {
            return needsSync && (currentTime - lastSyncTime >= syncInterval);
        }

        /**
         * @brief Mark entity as needing synchronization
         */
        void markDirty() {
            needsSync = true;
        }

        std::string getTypeName() const override {
            return "NetworkComponent";
        }
    };

} // namespace rtype::ecs
