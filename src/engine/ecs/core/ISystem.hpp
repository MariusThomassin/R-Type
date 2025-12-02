/*
** R-Type ECS - ISystem Interface
** Base interface for all ECS systems
*/

#pragma once

#include "Types.hpp"

namespace rtype::ecs {

    class Registry;

    /**
     * @brief Base interface for all systems in the ECS
     * 
     * Systems contain the game logic that operates on entities
     * with specific component combinations. Each system should
     * focus on a single responsibility.
     */
    class ISystem {
    public:
        virtual ~ISystem() = default;

        /**
         * @brief Update the system
         * @param dt Delta time since last update (in seconds)
         */
        virtual void update(float dt) = 0;

        /**
         * @brief Get the execution phase of this system
         * @return The phase determining update order
         */
        virtual SystemPhase getPhase() const = 0;

        /**
         * @brief Check if the system is enabled
         * @return true if the system should be updated
         */
        virtual bool isEnabled() const { return m_enabled; }

        /**
         * @brief Enable or disable the system
         * @param enabled The new enabled state
         */
        virtual void setEnabled(bool enabled) { m_enabled = enabled; }

        /**
         * @brief Set the registry reference for this system
         * @param registry Pointer to the main registry
         */
        void setRegistry(Registry* registry) { m_registry = registry; }

    protected:
        Registry* m_registry = nullptr;
        bool m_enabled = true;
    };

} // namespace rtype::ecs
