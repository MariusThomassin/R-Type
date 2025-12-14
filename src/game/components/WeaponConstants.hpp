/*
** R-Type - Weapon Configuration Constants
** Shared constants for weapon configuration between client and server
*/

#pragma once

namespace rtype::ecs {

    /**
     * @brief Default weapon configuration constants
     * 
     * These constants define the default weapon behavior for player ships.
     * They are shared between the client and server to ensure consistency.
     */
    namespace WeaponConstants {
        // Default fire rate (seconds between shots)
        constexpr float DEFAULT_FIRE_RATE = 0.15f;
        
        // Default projectile damage
        constexpr int DEFAULT_DAMAGE = 10;
        
        // Default projectile speed (units per second)
        constexpr float DEFAULT_PROJECTILE_SPEED = 800.0f;
    }

} // namespace rtype::ecs
