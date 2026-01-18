/*
** R-Type ECS - Boss Component
** Stores boss-specific state and mechanic configurations
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include <string>

namespace rtype::ecs {

    /**
     * @brief Boss mechanic types
     */
    enum class BossMechanic {
        None,
        ArcShot,        // Fires bullets in an arc pattern
        MinionSpawner,  // Spawns minion enemies periodically
        Teleporter      // Teleports around the arena
    };

    /**
     * @brief Component for boss-specific behavior and state
     */
    struct BossComponent : public IComponent {
        BossMechanic mechanic = BossMechanic::None;
        
        // Arc shot parameters
        float arcSpread = 45.0f;        // Spread angle in degrees
        int bulletsPerArc = 5;          // Number of bullets per arc
        float arcCooldown = 2.0f;       // Time between arc shots
        float arcTimer = 0.0f;          // Current timer
        
        // Minion spawner parameters
        float minionSpawnRate = 3.0f;   // Time between minion spawns
        float minionTimer = 0.0f;       // Current timer
        int maxMinions = 4;             // Maximum minions alive at once
        int currentMinions = 0;         // Currently alive minions
        
        // Teleporter parameters
        float teleportCooldown = 4.0f;  // Time between teleports
        float teleportTimer = 0.0f;     // Current timer
        float teleportWarning = 0.5f;   // Warning flash before teleport
        bool teleporting = false;       // Currently in teleport sequence
        
        // Phase tracking
        int currentPhase = 1;           // Current boss phase (1-3)
        float phaseMultiplier = 1.0f;   // Speed/rate multiplier based on phase
        
        // Movement pattern
        float moveTimer = 0.0f;         // For oscillating movement
        float baseY = 360.0f;           // Center Y position for oscillation
        float oscillateRange = 150.0f;  // How far to oscillate up/down
        float moveSpeed = 40.0f;        // Vertical oscillation speed
        
        // Max health for phase calculations
        int maxHealth = 30;
        
        /**
         * @brief Get the type name for this component
         */
        std::string getTypeName() const override { return "BossComponent"; }
        
        /**
         * @brief Parse mechanic type from string
         */
        static BossMechanic parseMechanic(const std::string& str) {
            if (str == "arc_shot") return BossMechanic::ArcShot;
            if (str == "minion_spawner") return BossMechanic::MinionSpawner;
            if (str == "teleporter") return BossMechanic::Teleporter;
            return BossMechanic::None;
        }
    };

} // namespace rtype::ecs
