/*
** Flappy Bird - FlappyScoreComponent
** Tracks player score and high score
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"

#include <string>

namespace flappy {

    /**
     * @brief Score tracking component for Flappy Bird
     * 
     * Attached to a score entity to track:
     * - Current score
     * - High score
     * - Game state
     */
    struct FlappyScoreComponent : public rtype::ecs::IComponent {
        int score = 0;
        int highScore = 0;
        
        FlappyScoreComponent() = default;
        
        void addPoint() {
            score++;
            if (score > highScore) {
                highScore = score;
            }
        }
        
        void reset() {
            score = 0;
        }
        
        std::string getTypeName() const override {
            return "FlappyScoreComponent";
        }
    };

} // namespace flappy
