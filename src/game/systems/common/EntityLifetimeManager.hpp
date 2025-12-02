/*
** R-Type ECS - Entity Lifetime Manager
** Handles entity lifetime updates and cleanup
** Used for projectiles, effects, particles, temporary entities
** 
** NOTE: This is a legacy static helper. For new code, prefer using
** the LifetimeSystem from src/engine/ecs/systems/LifetimeSystem.hpp
** which integrates with SystemManager.
*/

#pragma once

#include "../../../engine/ecs/core/Registry.hpp"
#include "../../../engine/ecs/components/LifetimeComponent.hpp"
#include "../../../engine/ecs/components/TransformComponent.hpp"
#include "../../components/ProjectileComponent.hpp"
#include <vector>

namespace rtype::ecs {

    /**
     * @brief Manages entity lifetimes and cleanup
     * Can be used for projectiles, effects, particles, and any temporary entities
     * 
     * @deprecated For new code, prefer using LifetimeSystem which integrates
     * with SystemManager and uses zero-allocation iteration.
     */
    struct EntityLifetimeManager {

        /**
         * @brief Update lifetimes and collect expired entities with a specific component
         */
        template<typename ComponentType>
        static void updateLifetimes(Registry& registry, float dt,
                                    std::vector<EntityId>& expiredEntities) {
            expiredEntities.clear();

            // Use the new forEach API for better performance
            registry.forEach<ComponentType, LifetimeComponent>([&](EntityId entity) {
                auto& lifetime = registry.getComponent<LifetimeComponent>(entity);

                if (lifetime.update(dt)) {
                    expiredEntities.push_back(entity);
                }
            });
        }

        /**
         * @brief Update projectile lifetimes specifically (backward compatibility)
         */
        static void updateLifetimes(Registry& registry, float dt,
                                    std::vector<EntityId>& expiredProjectiles) {
            updateLifetimes<ProjectileComponent>(registry, dt, expiredProjectiles);
        }

        /**
         * @brief Destroy expired entities
         */
        static void destroyExpired(Registry& registry,
                                   const std::vector<EntityId>& expiredEntities) {
            for (EntityId entity : expiredEntities) {
                if (registry.entityExists(entity)) {
                    registry.destroyEntity(entity);
                }
            }
        }

        /**
         * @brief Destroy entities that are off-screen with a specific component
         */
        template<typename ComponentType>
        static void destroyOffscreen(Registry& registry,
                                     int screenWidth, int screenHeight,
                                     float margin = 50.0f) {
            std::vector<EntityId> toDestroy;

            registry.forEach<ComponentType, TransformComponent>([&](EntityId entity) {
                const auto& transform = registry.getComponent<TransformComponent>(entity);

                if (transform.x < -margin || transform.x > screenWidth + margin ||
                    transform.y < -margin || transform.y > screenHeight + margin) {
                    toDestroy.push_back(entity);
                }
            });

            for (EntityId entity : toDestroy) {
                if (registry.entityExists(entity)) {
                    registry.destroyEntity(entity);
                }
            }
        }

        /**
         * @brief Destroy projectiles off-screen (backward compatibility)
         */
        static void destroyOffscreen(Registry& registry,
                                     int screenWidth, int screenHeight,
                                     float margin = 50.0f) {
            destroyOffscreen<ProjectileComponent>(registry, screenWidth, screenHeight, margin);
        }
    };

    // Backward compatibility alias
    using ProjectileLifetimeManager = EntityLifetimeManager;

} // namespace rtype::ecs
