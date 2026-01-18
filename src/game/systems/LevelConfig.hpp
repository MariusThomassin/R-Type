/*
** R-Type ECS - Level Configuration Types
** Shared data structures for level definitions
*/

#pragma once

#include "game/components/EnemyComponent.hpp"
#include <vector>
#include <string>

namespace rtype::ecs {

    /**
     * @brief Configuration for a single enemy spawn
     */
    struct EnemySpawnConfig {
        EnemyType type = EnemyType::Basic;
        float x = 0.0f, y = 0.0f;
        float vx = -100.0f, vy = 0.0f;
        int health = 1;
        int scoreValue = 100;
        float spawnDelay = 0.0f;        // Per-enemy delay offset for ordered spawning
        bool shootsAtPlayer = false;    // Override shooting behavior
        float fireRate = 1.5f;          // Custom fire rate if shootsAtPlayer
        
        // Boss mechanic fields
        std::string mechanic;           // Boss mechanic type: "arc_shot", "minion_spawner", "teleporter"
        float arcSpread = 45.0f;        // Arc shot: spread angle in degrees
        int bulletsPerArc = 5;          // Arc shot: number of bullets per arc
        float arcCooldown = 2.0f;       // Arc shot: cooldown between arcs
        
        // Minion spawner params
        float minionSpawnRate = 4.0f;   // Time between minion spawns
        int maxMinions = 4;             // Maximum minions alive
        
        // Teleporter params  
        float teleportCooldown = 5.0f;  // Time between teleports
    };

    /**
     * @brief Configuration for a wave of enemies
     */
    struct WaveConfig {
        float delayBefore = 0.0f;       // Delay before wave starts
        std::vector<EnemySpawnConfig> enemies;
        float spawnInterval = 0.5f;     // Time between each enemy spawn
        bool simultaneous = false;      // Spawn all at once vs sequential
        bool ordered = false;           // Respect spawnDelay offsets for precise ordering
    };

    /**
     * @brief Configuration for a powerup spawn in level
     */
    struct PowerupSpawnConfig {
        int type = 0;                   // PowerupType as int
        float x = 0.0f, y = 0.0f;
        float triggerTime = 0.0f;       // When to spawn (seconds from level start)
        bool spawned = false;           // Track if already spawned
    };

    /**
     * @brief Configuration for a bomb spawn in level
     */
    struct BombSpawnConfig {
        float x = 0.0f, y = 0.0f;
        float triggerTime = 0.0f;       // When to spawn (seconds from level start)
        bool spawned = false;           // Track if already spawned
    };

    /**
     * @brief Boss phase configuration
     */
    struct BossPhaseConfig {
        int phase = 1;                  // Phase number
        float healthThreshold = 1.0f;   // Health percentage to trigger (0.0-1.0)
        std::string pattern;            // Attack pattern identifier
        float moveSpeed = 50.0f;        // Movement speed in this phase
    };

    /**
     * @brief Boss section configuration
     */
    struct BossSectionConfig {
        bool enabled = false;
        EnemySpawnConfig boss;          // Boss spawn configuration
        std::vector<BossPhaseConfig> phases;
        float triggerDelay = 2.0f;      // Delay after waves complete before boss spawns
        bool musicChange = true;        // Switch to boss music when triggered
    };

    /**
     * @brief Configuration for an entire level
     */
    struct LevelConfig {
        std::vector<WaveConfig> waves;
        float waveDelay = 2.0f;         // Default delay between waves
        int difficulty = 1;
        
        // Level assets
        std::string name;               // Level name
        std::string background;         // Path to background image
        std::string stageMusic;         // Path to stage music
        std::string bossMusic;          // Path to boss music
        
        // Powerup and bomb spawns
        std::vector<PowerupSpawnConfig> powerupSpawns;
        std::vector<BombSpawnConfig> bombSpawns;
        
        // Boss section
        BossSectionConfig bossSection;
    };

} // namespace rtype::ecs
