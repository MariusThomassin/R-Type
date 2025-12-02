/*
** R-Type ECS - Entity Events
** Events related to entity lifecycle
*/

#pragma once

#include "../../core/Types.hpp"
#include <string>

namespace rtype::ecs::events {

    /**
     * @brief Emitted when an entity is created
     */
    struct EntityCreated {
        EntityId entity;
        std::string tag;  // Optional entity tag/name
    };

    /**
     * @brief Emitted when an entity is about to be destroyed
     */
    struct EntityDestroying {
        EntityId entity;
    };

    /**
     * @brief Emitted after an entity is destroyed
     */
    struct EntityDestroyed {
        EntityId entity;
    };

    /**
     * @brief Emitted when a component is added to an entity
     */
    struct ComponentAdded {
        EntityId entity;
        std::size_t componentTypeId;
    };

    /**
     * @brief Emitted when a component is removed from an entity
     */
    struct ComponentRemoved {
        EntityId entity;
        std::size_t componentTypeId;
    };

} // namespace rtype::ecs::events
