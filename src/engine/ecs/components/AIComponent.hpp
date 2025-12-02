/*
** R-Type ECS - AIComponent
** Artificial intelligence state data
*/

#pragma once

#include "../core/IComponent.hpp"
#include "../core/Types.hpp"

namespace rtype::ecs {

    /**
     * @brief AI behavior states
     */
    enum class AIState {
        Idle,       // Not doing anything
        Patrol,     // Moving along a path
        Chase,      // Following target
        Attack,     // Attacking target
        Flee,       // Running away
        Dead        // Destroyed, waiting for cleanup
    };

    /**
     * @brief Component holding AI behavior data
     *
     * Used by entities with autonomous behavior.
     */
    struct AIComponent : public IComponent {
        AIState state = AIState::Idle;
        EntityId targetEntityId = NULL_ENTITY;

        float detectionRange = 200.0f;
        float attackRange = 100.0f;
        float thinkInterval = 0.5f;    // Seconds between AI updates
        float lastThinkTime = 0.0f;

        int patternIndex = 0;          // Current step in movement pattern
        float patternTimer = 0.0f;

        AIComponent() = default;

        explicit AIComponent(AIState initialState)
            : state(initialState) {}

        AIComponent(AIState initialState, float detection, float attack)
            : state(initialState), detectionRange(detection), attackRange(attack) {}

        /**
         * @brief Check if it's time for an AI update
         */
        bool shouldThink(float currentTime) const {
            return (currentTime - lastThinkTime >= thinkInterval);
        }

        std::string getTypeName() const override {
            return "AIComponent";
        }
    };

} // namespace rtype::ecs
