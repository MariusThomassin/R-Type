/*
** R-Type ECS - ColliderComponent
** Collision detection data
*/

#pragma once

#include "../core/IComponent.hpp"

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

    inline CollisionLayer operator|(CollisionLayer a, CollisionLayer b) {
        return static_cast<CollisionLayer>(
            static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
    }

    inline CollisionLayer operator&(CollisionLayer a, CollisionLayer b) {
        return static_cast<CollisionLayer>(
            static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
    }

    /**
     * @brief Component holding collision hitbox data
     *
     * Defines a rectangular hitbox relative to the entity's transform.
     */
    struct ColliderComponent : public IComponent {
        float offsetX = 0.0f;
        float offsetY = 0.0f;

        float width = 0.0f;
        float height = 0.0f;

        bool isTrigger = false;  // If true, detects but doesn't resolve
        CollisionLayer layer = CollisionLayer::None;
        CollisionLayer mask = CollisionLayer::All;  // What layers to collide with

        ColliderComponent() = default;

        ColliderComponent(float w, float h)
            : width(w), height(h) {}

        ColliderComponent(float w, float h, CollisionLayer l)
            : width(w), height(h), layer(l) {}

        ColliderComponent(float offX, float offY, float w, float h)
            : offsetX(offX), offsetY(offY), width(w), height(h) {}

        std::string getTypeName() const override {
            return "ColliderComponent";
        }
    };

} // namespace rtype::ecs
