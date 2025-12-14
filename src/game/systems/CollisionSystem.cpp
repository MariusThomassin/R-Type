/*
** R-Type ECS - CollisionSystem Implementation
** AABB collision detection with layer-based filtering
*/

#include "CollisionSystem.hpp"
#include "engine/ecs/components/HealthComponent.hpp"

#include <algorithm>
#include <cmath>

namespace rtype::ecs {

    CollisionSystem::CollisionSystem(EventBus& eventBus)
        : m_eventBus(eventBus)
    {
    }

    void CollisionSystem::update(float dt) {
        (void)dt; // Not used for collision detection

        if (!m_registry || !m_enabled) return;

        m_collisions.clear();

        // Collect all entities with colliders
        std::vector<EntityId> entities;
        m_registry->forEach<TransformComponent, ColliderComponent>(
            [&entities](EntityId entity) {
                entities.push_back(entity);
            }
        );

        // Broad-phase: Check all pairs (N^2 - can be optimized with spatial partitioning later)
        for (size_t i = 0; i < entities.size(); ++i) {
            EntityId entityA = entities[i];

            const auto& transformA = m_registry->getComponent<TransformComponent>(entityA);
            const auto& colliderA = m_registry->getComponent<ColliderComponent>(entityA);

            for (size_t j = i + 1; j < entities.size(); ++j) {
                EntityId entityB = entities[j];

                const auto& transformB = m_registry->getComponent<TransformComponent>(entityB);
                const auto& colliderB = m_registry->getComponent<ColliderComponent>(entityB);

                // Layer filtering: Check if these layers should collide
                if (!CollisionUtils::canCollide(colliderA.layer, colliderA.mask,
                                               colliderB.layer, colliderB.mask)) {
                    continue;
                }

                // Narrow-phase: AABB collision test
                if (checkAABB(transformA, colliderA, transformB, colliderB)) {
                    m_collisions.push_back({entityA, entityB});
                    handleCollision(entityA, entityB);
                }
            }
        }
    }

    bool CollisionSystem::checkAABB(
        const TransformComponent& transformA, const ColliderComponent& colliderA,
        const TransformComponent& transformB, const ColliderComponent& colliderB) const
    {
        // Calculate bounding boxes
        float aLeft = transformA.x + colliderA.offsetX;
        float aRight = aLeft + colliderA.width;
        float aTop = transformA.y + colliderA.offsetY;
        float aBottom = aTop + colliderA.height;

        float bLeft = transformB.x + colliderB.offsetX;
        float bRight = bLeft + colliderB.width;
        float bTop = transformB.y + colliderB.offsetY;
        float bBottom = bTop + colliderB.height;

        // AABB overlap test
        return (aLeft < bRight && aRight > bLeft &&
                aTop < bBottom && aBottom > bTop);
    }

    std::pair<float, float> CollisionSystem::calculateOverlap(
        const TransformComponent& transformA, const ColliderComponent& colliderA,
        const TransformComponent& transformB, const ColliderComponent& colliderB) const
    {
        float aLeft = transformA.x + colliderA.offsetX;
        float aRight = aLeft + colliderA.width;
        float aTop = transformA.y + colliderA.offsetY;
        float aBottom = aTop + colliderA.height;

        float bLeft = transformB.x + colliderB.offsetX;
        float bRight = bLeft + colliderB.width;
        float bTop = transformB.y + colliderB.offsetY;
        float bBottom = bTop + colliderB.height;

        float overlapX = std::min(aRight, bRight) - std::max(aLeft, bLeft);
        float overlapY = std::min(aBottom, bBottom) - std::max(aTop, bTop);

        return {overlapX, overlapY};
    }

    void CollisionSystem::handleCollision(EntityId entityA, EntityId entityB) {
        // Emit generic collision event
        const auto& transformA = m_registry->getComponent<TransformComponent>(entityA);
        const auto& colliderA = m_registry->getComponent<ColliderComponent>(entityA);
        const auto& transformB = m_registry->getComponent<TransformComponent>(entityB);
        const auto& colliderB = m_registry->getComponent<ColliderComponent>(entityB);

        auto [overlapX, overlapY] = calculateOverlap(transformA, colliderA, transformB, colliderB);

        // CollisionEvent expects normalX, normalY, penetration
        m_eventBus.emit(CollisionEvent{entityA, entityB, 0.0f, 0.0f, std::min(overlapX, overlapY)});

        // Handle projectile collisions specifically
        bool aIsProjectile = m_registry->hasComponent<ProjectileComponent>(entityA);
        bool bIsProjectile = m_registry->hasComponent<ProjectileComponent>(entityB);

        if (aIsProjectile && !bIsProjectile) {
            handleProjectileCollision(entityA, entityB);
        } else if (bIsProjectile && !aIsProjectile) {
            handleProjectileCollision(entityB, entityA);
        }
    }

    void CollisionSystem::handleProjectileCollision(EntityId projectile, EntityId target) {
        const auto& projectileComp = m_registry->getComponent<ProjectileComponent>(projectile);

        // Don't collide with owner
        if (projectileComp.ownerId == target) {
            return;
        }

        // Emit projectile hit event
        m_eventBus.emit(ProjectileHitEvent{projectile, target, projectileComp.damage});

        // Apply damage if target has health
        if (m_registry->hasComponent<HealthComponent>(target)) {
            auto& health = m_registry->getComponent<HealthComponent>(target);
            health.currentHealth -= projectileComp.damage;
        }

        // Always destroy the projectile on hit (piercing not implemented yet)
        m_registry->destroyEntity(projectile);
    }

} // namespace rtype::ecs
