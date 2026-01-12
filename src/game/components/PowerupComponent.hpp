/*
** R-Type ECS - PowerupComponent
** Component for powerup items that can be collected by players
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Types of powerups available in the game
     */
    enum class PowerupType : uint8_t {
        SPREAD_SHOT = 0,    // Fire multiple projectiles at once
        SPEED_BOOST = 1,    // Move faster temporarily
        HEALTH_UP = 2,      // Restore health
        SHIELD = 3,         // Temporary invincibility
        WEAPON_UPGRADE = 4, // Upgrade weapon power level
        FORCE_ORB = 5,      // Spawn/upgrade Force orb attachment
        BOMB = 6            // Screen-clearing bomb
    };

    /**
     * @brief Component for powerup items
     * 
     * Powerups spawn from defeated enemies or at specific locations.
     * When a player collides with a powerup, the effect is applied.
     */
    struct PowerupComponent : public IComponent {
        PowerupType type = PowerupType::SPREAD_SHOT;
        float duration = 10.0f;     // Duration for temporary effects (seconds)
        float value = 1.0f;         // Effect magnitude (health amount, speed multiplier, etc.)
        bool isCollected = false;   // Has this powerup been picked up?
        
        // Visual properties
        float bobOffset = 0.0f;     // For floating animation
        float glowIntensity = 1.0f; // For pulsing glow effect

        PowerupComponent() = default;

        explicit PowerupComponent(PowerupType t, float dur = 10.0f, float val = 1.0f)
            : type(t), duration(dur), value(val) {}

        /**
         * @brief Get color for this powerup type (for rendering)
         */
        static constexpr uint32_t getColor(PowerupType type) {
            switch (type) {
                case PowerupType::SPREAD_SHOT:    return 0xFF00FFFF;  // Cyan
                case PowerupType::SPEED_BOOST:    return 0xFFFF00FF;  // Yellow
                case PowerupType::HEALTH_UP:      return 0x00FF00FF;  // Green
                case PowerupType::SHIELD:         return 0x00FFFFFF;  // Blue
                case PowerupType::WEAPON_UPGRADE: return 0xFF8800FF;  // Orange
                case PowerupType::FORCE_ORB:      return 0x8800FFFF;  // Purple
                case PowerupType::BOMB:           return 0xFF0000FF;  // Red
                default:                          return 0xFFFFFFFF;  // White
            }
        }

        /**
         * @brief Get name string for this powerup type
         */
        static const char* getName(PowerupType type) {
            switch (type) {
                case PowerupType::SPREAD_SHOT:    return "Spread Shot";
                case PowerupType::SPEED_BOOST:    return "Speed Boost";
                case PowerupType::HEALTH_UP:      return "Health Up";
                case PowerupType::SHIELD:         return "Shield";
                case PowerupType::WEAPON_UPGRADE: return "Weapon Upgrade";
                case PowerupType::FORCE_ORB:      return "Force Orb";
                case PowerupType::BOMB:           return "Bomb";
                default:                          return "Unknown";
            }
        }

        std::string getTypeName() const override {
            return "PowerupComponent";
        }
    };

    /**
     * @brief Component tracking active powerup effects on an entity
     */
    struct ActivePowerupsComponent : public IComponent {
        // Spread shot
        bool hasSpreadShot = false;
        float spreadShotTimer = 0.0f;

        // Speed boost
        bool hasSpeedBoost = false;
        float speedBoostTimer = 0.0f;
        float originalMaxSpeed = 0.0f;  // To restore after effect ends

        // Shield
        bool hasShield = false;
        float shieldTimer = 0.0f;

        ActivePowerupsComponent() = default;

        /**
         * @brief Update all active powerup timers
         * @param dt Delta time
         */
        void update(float dt) {
            if (hasSpreadShot) {
                spreadShotTimer -= dt;
                if (spreadShotTimer <= 0) {
                    hasSpreadShot = false;
                    spreadShotTimer = 0.0f;
                }
            }

            if (hasSpeedBoost) {
                speedBoostTimer -= dt;
                if (speedBoostTimer <= 0) {
                    hasSpeedBoost = false;
                    speedBoostTimer = 0.0f;
                }
            }

            if (hasShield) {
                shieldTimer -= dt;
                if (shieldTimer <= 0) {
                    hasShield = false;
                    shieldTimer = 0.0f;
                }
            }
        }

        /**
         * @brief Check if any powerup is active
         */
        bool hasAnyActive() const {
            return hasSpreadShot || hasSpeedBoost || hasShield;
        }

        std::string getTypeName() const override {
            return "ActivePowerupsComponent";
        }
    };

} // namespace rtype::ecs
