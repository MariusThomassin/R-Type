/*
** R-Type ECS - ForceOrbComponent
** Component for the iconic R-Type Force orb power-up
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include "engine/ecs/core/Types.hpp"

namespace rtype::ecs {

    /**
     * @brief Which side of the player the Force orb is docked to
     */
    enum class OrbDockSide : uint8_t {
        Left = 0,   // In front of player (default for R-Type)
        Right = 1   // Behind player
    };

    /**
     * @brief Component for Force orb entities
     * 
     * The Force orb is an iconic R-Type power-up that:
     * - Attaches to the player's ship (left or right side)
     * - Automatically fires lasers at nearby enemies
     * - Can be upgraded through multiple levels
     * - Absorbs enemy projectiles (optional feature)
     */
    struct ForceOrbComponent : public IComponent {
        // Ownership
        EntityId ownerId = NULL_ENTITY;  // Player entity this orb belongs to
        
        // Docking state
        OrbDockSide dockSide = OrbDockSide::Left;
        float dockOffset = 40.0f;         // Distance from player center
        
        // Combat properties
        int level = 1;                    // 1-3, affects damage and fire pattern
        float fireRate = 0.15f;           // Seconds between shots
        float fireTimer = 0.0f;           // Current cooldown timer
        float damage = 10.0f;             // Base damage per laser
        float laserSpeed = 600.0f;        // Projectile speed
        
        // Targeting
        float targetRange = 400.0f;       // Max range to detect enemies
        EntityId currentTarget = NULL_ENTITY; // Currently tracked target
        
        // Visual
        float rotationSpeed = 180.0f;     // Degrees per second
        float currentRotation = 0.0f;     // Current visual rotation
        float glowIntensity = 1.0f;       // Pulsing glow effect
        float glowTimer = 0.0f;           // Timer for glow animation
        
        // State
        bool isActive = true;             // Whether orb is firing
        bool canAbsorbProjectiles = false; // Level 3+ can absorb enemy bullets

        ForceOrbComponent() = default;

        explicit ForceOrbComponent(EntityId owner, OrbDockSide side = OrbDockSide::Left, int lvl = 1)
            : ownerId(owner)
            , dockSide(side)
            , level(lvl) {
            updateForLevel();
        }

        /**
         * @brief Update orb properties based on current level
         */
        void updateForLevel() {
            switch (level) {
                case 1:
                    fireRate = 0.18f;
                    damage = 8.0f;
                    targetRange = 350.0f;
                    canAbsorbProjectiles = false;
                    break;
                case 2:
                    fireRate = 0.12f;
                    damage = 15.0f;
                    targetRange = 450.0f;
                    canAbsorbProjectiles = false;
                    break;
                case 3:
                default:
                    fireRate = 0.08f;
                    damage = 25.0f;
                    targetRange = 550.0f;
                    canAbsorbProjectiles = true;
                    break;
            }
        }

        /**
         * @brief Upgrade the orb to the next level
         * @return true if upgraded, false if already max level
         */
        bool upgrade() {
            if (level >= 3) return false;
            level++;
            updateForLevel();
            return true;
        }

        /**
         * @brief Toggle which side the orb is docked to
         */
        void toggleSide() {
            dockSide = (dockSide == OrbDockSide::Left) ? OrbDockSide::Right : OrbDockSide::Left;
        }

        /**
         * @brief Get the X offset for the orb based on dock side
         */
        float getDockOffsetX() const {
            return (dockSide == OrbDockSide::Left) ? -dockOffset : dockOffset;
        }

        /**
         * @brief Get color based on level (for rendering)
         */
        static constexpr uint32_t getLevelColor(int level) {
            switch (level) {
                case 1: return 0x00AAFFFF;  // Light blue
                case 2: return 0x00FF88FF;  // Cyan-green
                case 3: return 0xFFAA00FF;  // Orange
                default: return 0xFFFFFFFF; // White
            }
        }

        std::string getTypeName() const override {
            return "ForceOrbComponent";
        }
    };

} // namespace rtype::ecs
