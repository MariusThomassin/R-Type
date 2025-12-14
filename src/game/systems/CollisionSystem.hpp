/*
** R-Type ECS - CollisionSystem
** Handles AABB collision detection between entities
** Uses ColliderComponent with layer-based filtering
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"
#include "engine/ecs/components/CollisionUtils.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "game/components/PlayerComponent.hpp"

#include <vector>
#include <utility>

namespace rtype::ecs {

    // Note: CollisionEvent is already defined in EventBus.hpp

    /**
     * @brief Event emitted when a projectile hits an entity
     */
    struct ProjectileHitEvent {
        EntityId projectile;
        EntityId target;
        int damage;
    };

    /**
     * @brief System that detects and resolves collisions between entities
     *
     * Features:
     * - AABB (Axis-Aligned Bounding Box) collision detection
     * - Layer-based filtering (Player, Enemy, PlayerShot, EnemyShot)
     * - Trigger detection (no physics resolution)
     * - Emits CollisionEvent and ProjectileHitEvent via EventBus
     *
     * Runs during COLLISION phase (after Physics, before GameLogic)
     */
    class CollisionSystem : public ISystem {
    public:
        /**
         * @brief Construct a new Collision System object
         * @param eventBus Reference to EventBus for collision events
         */
        explicit CollisionSystem(EventBus& eventBus);

        /**
         * @brief Destroy the Collision System object
         */
        ~CollisionSystem() override = default;

        /**
         * @brief Update collision detection
         * @param dt Delta time since last update
         */
        void update(float dt) override;

        /**
         * @brief Get the system phase (Collision)
         * @return SystemPhase
         */
        SystemPhase getPhase() const override {
            return SystemPhase::Collision;
        }

        /**
         * @brief Enable or disable collision detection
         * @param enabled true to enable, false to disable
         */
        void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

    private:
        /**
         * @brief Check AABB collision between two colliders
         * @param transformA Transform of entity A
         * @param colliderA Collider of entity A
         * @param transformB Transform of entity B
         * @param colliderB Collider of entity B
         * @return true if colliding
         */
        bool checkAABB(const TransformComponent& transformA, const ColliderComponent& colliderA,
                      const TransformComponent& transformB, const ColliderComponent& colliderB) const;

        /**
         * @brief Calculate collision overlap
         * @param transformA Transform of entity A
         * @param colliderA Collider of entity A
         * @param transformB Transform of entity B
         * @param colliderB Collider of entity B
         * @return Pair of (overlapX, overlapY)
         */
        std::pair<float, float> calculateOverlap(
            const TransformComponent& transformA, const ColliderComponent& colliderA,
            const TransformComponent& transformB, const ColliderComponent& colliderB) const;

        /**
         * @brief Handle collision between two entities
         * @param entityA First entity
         * @param entityB Second entity
         */
        void handleCollision(EntityId entityA, EntityId entityB);

        /**
         * @brief Handle projectile-entity collision
         * @param projectile Projectile entity
         * @param target Target entity
         */
        void handleProjectileCollision(EntityId projectile, EntityId target);

    private:
        /**
         * @brief Reference to EventBus for emitting collision events
         */
        EventBus& m_eventBus;

        /**
         * @brief Whether collision detection is enabled
         */
        bool m_enabled = true;

        /**
         * @brief Collision pairs detected this frame (for debug)
         */
        std::vector<std::pair<EntityId, EntityId>> m_collisions;
    };

} // namespace rtype::ecs
