/*
** R-Type ECS - IComponentArray Interface
** Abstract base for type-erased component storage
*/

#pragma once

#include "Types.hpp"

namespace rtype::ecs {

    /**
     * @brief Abstract interface for component storage arrays
     * 
     * This interface allows the Registry to store different
     * ComponentArray<T> instances in a single container while
     * still being able to notify them when entities are destroyed.
     */
    class IComponentArray {
    public:
        virtual ~IComponentArray() = default;

        /**
         * @brief Called when an entity is destroyed
         * @param entity The entity being destroyed
         * 
         * Implementations should remove any component data
         * associated with this entity.
         */
        virtual void entityDestroyed(EntityId entity) = 0;

        /**
         * @brief Check if the array has a component for an entity
         * @param entity The entity to check
         * @return true if a component exists for this entity
         */
        virtual bool hasComponent(EntityId entity) const = 0;

        /**
         * @brief Get the number of components stored
         * @return Number of active components
         */
        virtual std::size_t size() const = 0;
    };

} // namespace rtype::ecs
