/*
** R-Type ECS - EnemyComponent
** Enemy-specific data
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include <string>

namespace rtype::ecs {

    /**
     * @brief Enemy types for behavior differentiation
     */
    enum class EnemyType {
        Basic,      // Simple straight-line enemy
        Chaser,     // Follows player
        Shooter,    // Fires projectiles
        Boss,       // Boss enemy
        Turret      // Stationary shooter
    };

    /**
     * @brief Component holding enemy-specific data
     *
     * Used to identify and configure enemy entities.
     */
    struct EnemyComponent : public IComponent {
        int enemyId = 0;
        EnemyType type = EnemyType::Basic;
        int difficultyLevel = 1;      // Affects behavior/stats
        int scoreValue = 100;         // Points when destroyed
        
        // Shooting behavior
        bool canShoot = true;         // Whether this enemy can shoot
        float fireRate = 2.0f;        // Shots per second
        float fireTimer = 0.0f;       // Current timer
        float projectileSpeed = 300.0f; // Speed of projectiles

        EnemyComponent() = default;

        explicit EnemyComponent(EnemyType t)
            : type(t) {
            // Set shooting based on type
            canShoot = (t == EnemyType::Shooter || t == EnemyType::Turret || t == EnemyType::Boss);
        }

        EnemyComponent(EnemyType t, int difficulty)
            : type(t), difficultyLevel(difficulty) {
            canShoot = (t == EnemyType::Shooter || t == EnemyType::Turret || t == EnemyType::Boss);
        }

        EnemyComponent(EnemyType t, int difficulty, int score)
            : type(t), difficultyLevel(difficulty), scoreValue(score) {
            canShoot = (t == EnemyType::Shooter || t == EnemyType::Turret || t == EnemyType::Boss);
        }

        std::string getTypeName() const override {
            return "EnemyComponent";
        }
    };

} // namespace rtype::ecs
