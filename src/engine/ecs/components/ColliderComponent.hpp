/*
** R-Type ECS - ColliderComponent
** Collision detection data
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Collision layer flags for filtering collisions
     */
    enum class CollisionLayer : unsigned int {
        None       = 0,
        Player     = 1 << 0,
        Enemy      = 1 << 1,
        PlayerShot = 1 << 2,
        EnemyShot  = 1 << 3,
        Powerup    = 1 << 4,
        Wall       = 1 << 5,
        All        = 0xFFFFFFFF
    };

    // For collision layer operations, use CollisionUtils class
    // e.g., CollisionUtils::combineLayer(), CollisionUtils::intersectLayer()

    /**
     * @brief Component holding collision hitbox data
     *
     * Defines a rectangular hitbox relative to the entity's transform.
     */
    struct ColliderComponent : public IComponent {
        /**
         * @brief X offset from the entity's position
         */
        float offsetX = 0.0f;
        /**
         * @brief Y offset from the entity's position
         */
        float offsetY = 0.0f;

        /**
         * @brief Width of the collider
         */
        float width = 0.0f;
        /**
         * @brief Height of the collider
         */
        float height = 0.0f;

        /**
         * @brief If true, collider is a trigger (detects but doesn't resolve)
         */
        bool isTrigger = false;  // If true, detects but doesn't resolve
        /**
         * @brief Collision layer of this entity
         */
        CollisionLayer layer = CollisionLayer::None;
        /**
         * @brief Collision mask defining which layers this entity collides with
         */
        CollisionLayer mask = CollisionLayer::All;  // What layers to collide with

        /**
         * @brief Get the type name of this component
         * @return String representation of the component type
         */
        ColliderComponent() = default;

        /**
         * @brief Construct a new Collider Component object
         * @param w Width of the collider
         * @param h Height of the collider
         */
        ColliderComponent(float w, float h) : width(w), height(h) {}

        /**
         * @brief Construct a new Collider Component object
         * @param w Width of the collider
         * @param h Height of the collider
         * @param l Collision layer
         */
        ColliderComponent(float w, float h, CollisionLayer l) : width(w), height(h), layer(l) {}

        /**
         * @brief Construct a new Collider Component object
         * @param offX X offset from entity position
         * @param offY Y offset from entity position
         * @param w Width of the collider
         * @param h Height of the collider
         */
        ColliderComponent(float offX, float offY, float w, float h) : offsetX(offX), offsetY(offY), width(w), height(h) {}

        /**
         * @brief Get the type name of this component
         * @return String representation of the component type
         */
        std::string getTypeName() const override
        {
            return "ColliderComponent";
        }
    };
} // namespace rtype::ecs
