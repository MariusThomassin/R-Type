/*
** R-Type ECS - WeaponComponent
** Weapon and shooting data
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include <string>

namespace rtype::ecs {

    /**
     * @brief Component holding weapon/shooting data
     *
     * Used by entities that can fire projectiles.
     */
    struct WeaponComponent : public IComponent {
        float fireRate = 0.2f;        // Seconds between shots
        int damage = 10;              // Damage per projectile
        std::string projectileType;   // Type of projectile to spawn
        float lastFiredTime = 0.0f;   // Time since last shot
        bool canFire = true;          // Ready to fire flag

        float projectileSpeed = 800.0f;
        int projectileCount = 1;      // For spread shots

        WeaponComponent() = default;

        WeaponComponent(float rate, int dmg)
            : fireRate(rate), damage(dmg) {}

        WeaponComponent(float rate, int dmg, const std::string& projType)
            : fireRate(rate), damage(dmg), projectileType(projType) {}

        /**
         * @brief Check if weapon can fire based on cooldown
         * @param currentTime Current game time
         */
        bool isReady(float currentTime) const {
            return canFire && (currentTime - lastFiredTime >= fireRate);
        }

        std::string getTypeName() const override {
            return "WeaponComponent";
        }
    };

} // namespace rtype::ecs
