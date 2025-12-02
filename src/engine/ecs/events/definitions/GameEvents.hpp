/*
** R-Type ECS - Game Events
** Game-specific events for R-Type
*/

#pragma once

#include "engine/ecs/core/Types.hpp"

namespace rtype::ecs::events {

    // ==================== Projectile Events ====================

    /**
     * @brief Request to spawn a projectile
     */
    struct SpawnProjectile {
        EntityId shooter;
        float x, y;
        float directionX, directionY;
        float speed;
        int damage;
        bool isPlayerBullet;
    };

    /**
     * @brief Emitted when projectile is spawned
     */
    struct ProjectileSpawned {
        EntityId projectile;
        EntityId shooter;
    };

    /**
     * @brief Emitted when projectile hits something
     */
    struct ProjectileHit {
        EntityId projectile;
        EntityId target;  // NULL_ENTITY if hit wall/boundary
        float hitX, hitY;
    };

    // ==================== Player Events ====================

    /**
     * @brief Player shot request
     */
    struct PlayerShoot {
        EntityId player;
        int weaponSlot = 0;
    };

    /**
     * @brief Player charged shot released
     */
    struct PlayerChargedShot {
        EntityId player;
        float chargeLevel;  // 0.0 to 1.0
    };

    /**
     * @brief Player picked up an item
     */
    struct ItemPickup {
        EntityId player;
        EntityId item;
        int itemType;
    };

    /**
     * @brief Player power level changed
     */
    struct PowerChanged {
        EntityId player;
        int oldPower;
        int newPower;
    };

    // ==================== Enemy Events ====================

    /**
     * @brief Enemy spawned
     */
    struct EnemySpawned {
        EntityId enemy;
        int enemyType;
        float x, y;
    };

    /**
     * @brief Enemy destroyed (not just died - full cleanup)
     */
    struct EnemyDestroyed {
        EntityId enemy;
        EntityId killer;
        int pointValue;
    };

    /**
     * @brief Boss phase changed
     */
    struct BossPhaseChanged {
        EntityId boss;
        int oldPhase;
        int newPhase;
    };

    // ==================== Wave/Level Events ====================

    /**
     * @brief Wave started
     */
    struct WaveStarted {
        int waveNumber;
        int enemyCount;
    };

    /**
     * @brief Wave completed
     */
    struct WaveCompleted {
        int waveNumber;
        float timeTaken;
    };

    /**
     * @brief Level completed
     */
    struct LevelCompleted {
        int levelNumber;
        int score;
        float timeTaken;
    };

    // ==================== Score Events ====================

    /**
     * @brief Score changed
     */
    struct ScoreChanged {
        int playerId;
        int oldScore;
        int newScore;
        int delta;
        std::string reason;  // "enemy_kill", "pickup", "bonus", etc.
    };

    /**
     * @brief High score achieved
     */
    struct HighScoreAchieved {
        int playerId;
        int score;
        int rank;
    };

    // ==================== UI/Demo Events ====================

    /**
     * @brief Showoff/demo mode started
     */
    struct ShowoffStart {};

    /**
     * @brief Showoff/demo mode ended
     */
    struct ShowoffEnd {};

    /**
     * @brief Danmaku pattern trigger (debug/demo)
     */
    struct DanmakuTrigger {
        float x, y;
        int patternId;
    };

} // namespace rtype::ecs::events
