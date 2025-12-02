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

        EnemyComponent() = default;

        explicit EnemyComponent(EnemyType t)
            : type(t) {}

        EnemyComponent(EnemyType t, int difficulty)
            : type(t), difficultyLevel(difficulty) {}

        EnemyComponent(EnemyType t, int difficulty, int score)
            : type(t), difficultyLevel(difficulty), scoreValue(score) {}

        std::string getTypeName() const override {
            return "EnemyComponent";
        }
    };

} // namespace rtype::ecs
