/*
** R-Type ECS - Collision Events
** Events related to physics and collisions
*/

#pragma once

#include "engine/ecs/core/Types.hpp"

namespace rtype::ecs::events {

    /**
     * @brief Collision layers for filtering
     */
    enum class CollisionLayer : unsigned int {
        None        = 0,
        Player      = 1 << 0,
        Enemy       = 1 << 1,
        PlayerBullet = 1 << 2,
        EnemyBullet = 1 << 3,
        Pickup      = 1 << 4,
        Wall        = 1 << 5,
        Trigger     = 1 << 6,
        All         = 0xFFFFFFFF
    };

    /**
     * @brief Emitted when two entities start colliding
     */
    struct CollisionEnter {
        EntityId entityA;
        EntityId entityB;
        CollisionLayer layerA;
        CollisionLayer layerB;
        float contactX, contactY;     // Contact point
        float normalX, normalY;        // Collision normal
        float penetration;             // Overlap depth
    };

    /**
     * @brief Emitted while two entities are still colliding
     */
    struct CollisionStay {
        EntityId entityA;
        EntityId entityB;
        float contactX, contactY;
        float normalX, normalY;
    };

    /**
     * @brief Emitted when two entities stop colliding
     */
    struct CollisionExit {
        EntityId entityA;
        EntityId entityB;
    };

    /**
     * @brief Emitted when entity enters a trigger zone
     */
    struct TriggerEnter {
        EntityId entity;
        EntityId trigger;
        int triggerId;  // Custom trigger identifier
    };

    /**
     * @brief Emitted when entity exits a trigger zone
     */
    struct TriggerExit {
        EntityId entity;
        EntityId trigger;
        int triggerId;
    };

    /**
     * @brief Emitted when an entity hits a boundary
     */
    struct BoundaryHit {
        EntityId entity;
        enum class Side { Left, Right, Top, Bottom } side;
        float position;  // X or Y depending on side
    };

} // namespace rtype::ecs::events
