/*
** R-Type ECS - Entity
** Entity representation and management
*/

#pragma once

#include "Types.hpp"

namespace rtype::ecs {

    /**
     * @brief Represents an entity in the ECS
     * 
     * An entity is essentially just an ID with an associated
     * signature (bitset) indicating which components it has.
     */
    struct Entity {
        EntityId id = NULL_ENTITY;
        Signature signature;

        Entity() = default;
        explicit Entity(EntityId entityId) : id(entityId) {}

        bool operator==(const Entity& other) const {
            return id == other.id;
        }

        bool operator!=(const Entity& other) const {
            return id != other.id;
        }

        bool operator<(const Entity& other) const {
            return id < other.id;
        }

        /**
         * @brief Check if this entity is valid (not null)
         */
        bool isValid() const {
            return id != NULL_ENTITY;
        }

        /**
         * @brief Implicit conversion to EntityId for convenience
         */
        operator EntityId() const {
            return id;
        }
    };

} // namespace rtype::ecs

namespace std {
    template <>
    struct hash<rtype::ecs::Entity> {
        std::size_t operator()(const rtype::ecs::Entity& entity) const {
            return std::hash<rtype::ecs::EntityId>{}(entity.id);
        }
    };
}
