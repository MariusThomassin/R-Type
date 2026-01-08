/*
** R-Type ECS - PlayerComponent
** Player-specific data
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Component holding player-specific data
     *
     * Used to identify and track player entities.
     */
    struct PlayerComponent : public IComponent {
        int playerId = 0;       // Player slot (1-4 for multiplayer)
        int score = 0;          // Current score
        int lives = 3;          // Remaining lives
        bool isLocal = true;    // Is this the local player?
        uint32_t networkClientId = 0;  // Network client ID (for server-side tracking)

        uint8_t slot = 0;       // Network slot (0-3) - used for spawn position

        PlayerComponent() = default;

        explicit PlayerComponent(int id)
            : playerId(id) {}

        PlayerComponent(int id, int startLives)
            : playerId(id), lives(startLives) {}

        PlayerComponent(uint8_t playerSlot, int startLives)
            : playerId(playerSlot), lives(startLives), slot(playerSlot) {}

        /**
         * @brief Add to player score
         */
        void addScore(int points) {
            score += points;
        }

        /**
         * @brief Lose a life
         * @return true if player had a life to lose, false if already at 0
         */
        bool loseLife() {
            if (lives > 0) {
                lives--;
                return true;
            }
            return false;
        }

        std::string getTypeName() const override {
            return "PlayerComponent";
        }
    };

} // namespace rtype::ecs
