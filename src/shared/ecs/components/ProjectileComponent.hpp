/*
** R-Type ECS - ProjectileComponent
** Projectile/bullet specific data
*/

#pragma once

#include "../IComponent.hpp"
#include "../Types.hpp"

namespace rtype::ecs {

    /**
     * @brief Component marking an entity as a projectile
     *
     * Contains projectile-specific data like damage and owner.
     */
    struct ProjectileComponent : public IComponent {
        EntityId ownerId = NULL_ENTITY;  // Who fired this projectile
        int damage = 10;                  // Damage on hit
        bool isPlayerProjectile = true;   // Player or enemy projectile

        ProjectileComponent() = default;

        ProjectileComponent(EntityId owner, int dmg, bool fromPlayer = true)
            : ownerId(owner), damage(dmg), isPlayerProjectile(fromPlayer) {}

        std::string getTypeName() const override {
            return "ProjectileComponent";
        }
    };

} // namespace rtype::ecs
