/*
** Flappy Bird - FlappyBirdSystem
** Main game system handling bird physics, pipe spawning, collision, and scoring
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/InputEvents.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"

#include "../components/BirdComponent.hpp"
#include "../components/PipeComponent.hpp"
#include "../components/FlappyScoreComponent.hpp"
#include "../components/GroundComponent.hpp"

#include <random>
#include <vector>

namespace flappy {

    /**
     * @brief Events for Flappy Bird game
     */
    struct FlappyGameOverEvent {
        int finalScore = 0;
        int highScore = 0;
    };
    
    struct FlappyScoreEvent {
        int score = 0;
    };

    /**
     * @brief Main game system for Flappy Bird
     * 
     * Handles:
     * - Bird gravity and flap physics
     * - Pipe spawning and movement
     * - Collision detection (bird vs pipes, bird vs ground)
     * - Score tracking
     * - Game state (waiting, playing, game over)
     */
    class FlappyBirdSystem : public rtype::ecs::ISystem {
    public:
        enum class GamePhase {
            Waiting,    // Waiting for first flap
            Playing,    // Game in progress
            GameOver    // Bird has died
        };
        
        /**
         * @brief Construct a new Flappy Bird System
         * @param eventBus Reference to EventBus for input handling
         * @param screenWidth Game screen width
         * @param screenHeight Game screen height
         */
        FlappyBirdSystem(rtype::ecs::EventBus& eventBus, int screenWidth, int screenHeight);
        
        /**
         * @brief Destroy the Flappy Bird System
         */
        ~FlappyBirdSystem() override;
        
        /**
         * @brief Update the game logic
         * @param dt Delta time since last update
         */
        void update(float dt) override;
        
        /**
         * @brief Get the system phase (GameLogic)
         * @return SystemPhase
         */
        rtype::ecs::SystemPhase getPhase() const override {
            return rtype::ecs::SystemPhase::GameLogic;
        }
        
        /**
         * @brief Initialize the game (create bird, ground, score entity)
         */
        void initializeGame();
        
        /**
         * @brief Reset the game to initial state
         */
        void resetGame();
        
        /**
         * @brief Get current game phase
         */
        GamePhase getGamePhase() const { return m_gamePhase; }
        
        /**
         * @brief Get current score
         */
        int getScore() const { return m_score; }
        
        /**
         * @brief Get high score
         */
        int getHighScore() const { return m_highScore; }
        
    private:
        /**
         * @brief Handle bird flap action
         */
        void handleFlap();
        
        /**
         * @brief Apply gravity to bird
         */
        void applyGravity(float dt);
        
        /**
         * @brief Update bird rotation based on velocity
         */
        void updateBirdRotation();
        
        /**
         * @brief Spawn a new pipe pair
         */
        void spawnPipePair();
        
        /**
         * @brief Update pipes (movement, cleanup)
         */
        void updatePipes(float dt);
        
        /**
         * @brief Check collisions (bird vs pipes, bird vs ground)
         */
        void checkCollisions();
        
        /**
         * @brief Check if bird has passed a pipe for scoring
         */
        void checkScoring();
        
        /**
         * @brief Update ground scrolling
         */
        void updateGround(float dt);
        
        /**
         * @brief Trigger game over
         */
        void triggerGameOver();
        
    private:
        rtype::ecs::EventBus& m_eventBus;
        int m_screenWidth;
        int m_screenHeight;
        
        // Game state
        GamePhase m_gamePhase = GamePhase::Waiting;
        int m_score = 0;
        int m_highScore = 0;
        
        // Entities
        rtype::ecs::EntityId m_birdEntity = rtype::ecs::NULL_ENTITY;
        rtype::ecs::EntityId m_groundEntity = rtype::ecs::NULL_ENTITY;
        rtype::ecs::EntityId m_scoreEntity = rtype::ecs::NULL_ENTITY;
        std::vector<std::pair<rtype::ecs::EntityId, rtype::ecs::EntityId>> m_pipePairs; // top, bottom
        
        // Pipe spawning
        float m_pipeSpawnTimer = 0.0f;
        float m_pipeSpawnInterval = 2.0f;  // Seconds between pipe spawns
        float m_pipeSpeed = 200.0f;        // Horizontal speed of pipes
        float m_pipeGapSize = 180.0f;      // Vertical gap between pipes
        float m_pipeWidth = 80.0f;
        
        // Ground
        float m_groundY = 0.0f;            // Y position of ground top
        
        // Random number generation
        std::mt19937 m_rng;
        std::uniform_real_distribution<float> m_gapDistribution;
        
        // Event subscriptions
        std::size_t m_keyPressedSubId = 0;
        std::size_t m_mouseClickSubId = 0;
        std::size_t m_keyStateSubId = 0;
        bool m_flapRequested = false;
        bool m_flapKeyWasDown = false;  // Track key state for edge detection
    };

} // namespace flappy
