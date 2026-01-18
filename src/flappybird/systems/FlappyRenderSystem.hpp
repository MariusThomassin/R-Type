/*
** Flappy Bird - FlappyRenderSystem
** Handles rendering for all Flappy Bird entities
** Collects IRenderable components and renders them in layer order
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/graphics/IRenderable.hpp"

#include "../components/BirdComponent.hpp"
#include "../components/PipeComponent.hpp"
#include "../components/GroundComponent.hpp"

#include <raylib.h>
#include <vector>
#include <algorithm>

namespace flappy {

    /**
     * @brief Render system for Flappy Bird game
     * 
     * Collects all IRenderable components and renders them in layer order.
     * Also handles UI rendering (score display).
     */
    class FlappyRenderSystem : public rtype::ecs::ISystem {
    public:
        /**
         * @brief Construct a new Flappy Render System
         * @param screenWidth Game screen width
         * @param screenHeight Game screen height
         */
        FlappyRenderSystem(int screenWidth, int screenHeight)
            : m_screenWidth(screenWidth)
            , m_screenHeight(screenHeight)
        {}
        
        /**
         * @brief Destroy the Flappy Render System
         */
        ~FlappyRenderSystem() override = default;
        
        /**
         * @brief Update (render) the system
         * @param dt Delta time since last update
         */
        void update(float dt) override {
            if (!m_registry) return;
            
            m_animTime += dt;
            
            // Build render context
            rtype::ecs::RenderContext ctx;
            ctx.screenWidth = m_screenWidth;
            ctx.screenHeight = m_screenHeight;
            ctx.animTime = m_animTime;
            ctx.deltaTime = dt;
            
            // Collect all renderables by entity ID (like R-Type's RenderSystem)
            struct RenderItem {
                rtype::ecs::EntityId entity;
                int layer;
                enum class Type { Bird, Pipe, Ground } type;
            };
            std::vector<RenderItem> renderItems;
            renderItems.reserve(32);
            
            // Collect birds
            m_registry->forEach<BirdComponent, rtype::ecs::TransformComponent>(
                [this, &renderItems](rtype::ecs::EntityId entity) {
                    const auto& bird = m_registry->getComponent<BirdComponent>(entity);
                    if (bird.isRenderable()) {
                        renderItems.push_back({entity, bird.getRenderLayer(), RenderItem::Type::Bird});
                    }
                }
            );
            
            // Collect pipes
            m_registry->forEach<PipeComponent, rtype::ecs::TransformComponent>(
                [this, &renderItems](rtype::ecs::EntityId entity) {
                    const auto& pipe = m_registry->getComponent<PipeComponent>(entity);
                    if (pipe.isRenderable()) {
                        renderItems.push_back({entity, pipe.getRenderLayer(), RenderItem::Type::Pipe});
                    }
                }
            );
            
            // Collect ground
            m_registry->forEach<GroundComponent, rtype::ecs::TransformComponent>(
                [this, &renderItems](rtype::ecs::EntityId entity) {
                    const auto& ground = m_registry->getComponent<GroundComponent>(entity);
                    if (ground.isRenderable()) {
                        renderItems.push_back({entity, ground.getRenderLayer(), RenderItem::Type::Ground});
                    }
                }
            );
            
            // Sort by layer (ascending - lower layers first)
            std::sort(renderItems.begin(), renderItems.end(),
                [](const RenderItem& a, const RenderItem& b) {
                    return a.layer < b.layer;
                }
            );
            
            // Render all items by looking up components at render time
            for (const auto& item : renderItems) {
                if (!m_registry->entityExists(item.entity)) continue;
                
                const auto& transform = m_registry->getComponent<rtype::ecs::TransformComponent>(item.entity);
                
                switch (item.type) {
                    case RenderItem::Type::Bird: {
                        const auto& bird = m_registry->getComponent<BirdComponent>(item.entity);
                        bird.render(transform, ctx);
                        break;
                    }
                    case RenderItem::Type::Pipe: {
                        const auto& pipe = m_registry->getComponent<PipeComponent>(item.entity);
                        pipe.render(transform, ctx);
                        break;
                    }
                    case RenderItem::Type::Ground: {
                        const auto& ground = m_registry->getComponent<GroundComponent>(item.entity);
                        ground.render(transform, ctx);
                        break;
                    }
                }
            }
        }
        
        /**
         * @brief Get the system phase (Render)
         * @return SystemPhase
         */
        rtype::ecs::SystemPhase getPhase() const override {
            return rtype::ecs::SystemPhase::Render;
        }
        
        /**
         * @brief Draw the score display
         */
        void drawScore(int score, int highScore, bool isGameOver, bool isWaiting) {
            // Score at top center
            const char* scoreText = TextFormat("%d", score);
            int fontSize = 64;
            int textWidth = MeasureText(scoreText, fontSize);
            
            // Score shadow
            DrawText(scoreText, m_screenWidth / 2 - textWidth / 2 + 3, 53, fontSize, BLACK);
            // Score
            DrawText(scoreText, m_screenWidth / 2 - textWidth / 2, 50, fontSize, WHITE);
            
            if (isWaiting) {
                // Instructions
                const char* instructions = "Press SPACE or CLICK to start";
                int instrWidth = MeasureText(instructions, 24);
                DrawText(instructions, m_screenWidth / 2 - instrWidth / 2 + 2, m_screenHeight / 2 + 102, 24, BLACK);
                DrawText(instructions, m_screenWidth / 2 - instrWidth / 2, m_screenHeight / 2 + 100, 24, WHITE);
            }
            
            if (isGameOver) {
                // Game Over panel
                int panelWidth = 300;
                int panelHeight = 200;
                int panelX = m_screenWidth / 2 - panelWidth / 2;
                int panelY = m_screenHeight / 2 - panelHeight / 2;
                
                DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(BLACK, 0.7f));
                DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, WHITE);
                
                // Game Over text
                const char* gameOverText = "GAME OVER";
                int goWidth = MeasureText(gameOverText, 36);
                DrawText(gameOverText, m_screenWidth / 2 - goWidth / 2, panelY + 20, 36, RED);
                
                // Score
                const char* finalScoreText = TextFormat("Score: %d", score);
                int fsWidth = MeasureText(finalScoreText, 28);
                DrawText(finalScoreText, m_screenWidth / 2 - fsWidth / 2, panelY + 70, 28, WHITE);
                
                // High score
                const char* highScoreText = TextFormat("Best: %d", highScore);
                int hsWidth = MeasureText(highScoreText, 28);
                DrawText(highScoreText, m_screenWidth / 2 - hsWidth / 2, panelY + 105, 28, GOLD);
                
                // Restart instructions
                const char* restartText = "Press SPACE or R to restart";
                int restartWidth = MeasureText(restartText, 20);
                DrawText(restartText, m_screenWidth / 2 - restartWidth / 2, panelY + 155, 20, LIGHTGRAY);
            }
        }
        
    private:
        int m_screenWidth;
        int m_screenHeight;
        float m_animTime = 0.0f;
    };

} // namespace flappy
