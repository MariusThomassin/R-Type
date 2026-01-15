/*
** R-Type ECS - RenderSystem
** Lightweight renderer using self-rendering components via IRenderable
*/

#pragma once

#include "../../engine/ecs/core/ISystem.hpp"
#include "../../engine/graphics/IRenderable.hpp"
#include "../../engine/ecs/core/Registry.hpp"
#include "../../engine/ecs/components/TransformComponent.hpp"
#include "../../engine/ecs/components/SpriteComponent.hpp"
#include "../../engine/ecs/components/ImageBackgroundComponent.hpp"
#include "../../engine/ecs/components/ProceduralBackgroundComponent.hpp"
#include "../components/SpritesheetComponent.hpp"
#include "../components/PlayerShipComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/ProjectileComponent.hpp"
#include "../../engine/ui/UIManager.hpp"

#include <raylib.h>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <functional>

// Game state for conditional rendering
enum class GameState {
    MENU,
    MULTIPLAYER,
    LOBBY,
    PLAYING,
    PAUSED,
    SETTINGS
};

namespace rtype::ui {
    class UIManager;
}

namespace rtype::ecs {

    /**
     * @brief Lightweight render system using self-rendering components
     * 
     * Components implement IRenderable and handle their own drawing.
     * RenderSystem coordinates: sorting, context, background, and UI.
     */
    class RenderSystem : public ISystem {
        public:
            /**
             * @brief Construct a new Render System object
             * @param screenWidth Width of the game screen
             * @param screenHeight Height of the game screen
             */
            RenderSystem(int screenWidth = 1280, int screenHeight = 720)
                : m_screenWidth(screenWidth)
                , m_screenHeight(screenHeight)
                , m_animTime(0.0f) {
                loadTextures();
            }

            /**
             * @brief Destroy the Render System object
             */
            ~RenderSystem() override {
                if (IsWindowReady()) {
                    for (auto& [id, tex] : m_textures) {
                        UnloadTexture(tex);
                    }
                }
            }

            /**
             * @brief Set the UI Manager pointer
             * @param uiManager Pointer to the UIManager
             */
            void setUIManager(rtype::ui::UIManager* uiManager) {
                m_uiManager = uiManager;
            }
            /**
             * @brief Set the Game State pointer
             * @param gameState Pointer to the current GameState
             */
            void setGameStatePtr(const GameState* gameState) {
                m_gameState = gameState;
            }

            /**
             * @brief Update and render all IRenderable components
             * @param dt Delta time since last update
             */
            void update(float dt) override {
                if (!m_registry) {
                    static bool loggedNull = false;
                    if (!loggedNull) {
                        std::cerr << "[RenderSystem] ERROR: m_registry is null!" << std::endl;
                        loggedNull = true;
                    }
                    return;
                }

                m_animTime += dt;

                RenderContext ctx{&m_textures, m_screenWidth, m_screenHeight, m_animTime, dt};

                BeginDrawing();
                ClearBackground({8, 8, 20, 255});
                
                // Render procedural backgrounds first (layer -101)
                m_registry->forEach<TransformComponent, ProceduralBackgroundComponent>(
                    [this, &ctx](EntityId e) {
                        auto& proc = m_registry->getComponent<ProceduralBackgroundComponent>(e);
                        const auto& transform = m_registry->getComponent<TransformComponent>(e);
                        proc.render(transform, ctx);
                    }
                );
                
                // Render image backgrounds (layer -100)
                m_registry->forEach<TransformComponent, ImageBackgroundComponent>(
                    [this, &ctx](EntityId e) {
                        auto& imgBg = m_registry->getComponent<ImageBackgroundComponent>(e);
                        const auto& transform = m_registry->getComponent<TransformComponent>(e);
                        imgBg.render(transform, ctx);
                    }
                );

                // Optimization: Use struct to cache layer during collection (avoids repeated lookups during sort)
                struct RenderItem {
                    EntityId entity;
                    int layer;
                };
                
                std::vector<RenderItem> entities;
                entities.reserve(m_lastEntityCount > 0 ? m_lastEntityCount + 64 : 256);
                
                // Collect SpritesheetComponent entities (most common - bullets)
                m_registry->forEach<TransformComponent, SpritesheetComponent>(
                    [this, &entities](EntityId e) {
                        const auto& sheet = m_registry->getComponent<SpritesheetComponent>(e);
                        entities.push_back({e, sheet.getRenderLayer()});
                    }
                );
                
                // Collect SpriteComponent entities (fewer)
                m_registry->forEach<TransformComponent, SpriteComponent>(
                    [this, &entities](EntityId e) {
                        if (!m_registry->hasComponent<SpritesheetComponent>(e)) {
                            const auto& sprite = m_registry->getComponent<SpriteComponent>(e);
                            entities.push_back({e, sprite.getRenderLayer()});
                        }
                    }
                );
                
                // Collect PlayerShipComponent entities (rare)
                m_registry->forEach<TransformComponent, PlayerShipComponent>(
                    [this, &entities](EntityId e) {
                        if (!m_registry->hasComponent<SpriteComponent>(e) && 
                            !m_registry->hasComponent<SpritesheetComponent>(e)) {
                            const auto& ship = m_registry->getComponent<PlayerShipComponent>(e);
                            entities.push_back({e, ship.getRenderLayer()});
                        }
                    }
                );

                m_lastEntityCount = entities.size();

                // Log entity collection for debugging
                static size_t logCounter = 0;
                static bool hasLogged = false;
                if (++logCounter % 60 == 0) {
                    std::cout << "[RenderSystem] Collected " << entities.size() << " entities to render (registry size: " << m_registry->getEntityCount() << ")" << std::endl;
                    if (entities.size() > 0 && !hasLogged) {
                        std::cout << "[RenderSystem] First time seeing entities! Registry might have been empty before." << std::endl;
                        hasLogged = true;
                    }
                }

                // Sort using cached layer values (O(n log n) but with fast comparisons)
                std::sort(entities.begin(), entities.end(), [](const RenderItem& a, const RenderItem& b) {
                    return a.layer < b.layer;
                });

                for (const RenderItem& item : entities) {
                    EntityId e = item.entity;
                    const auto& transform = m_registry->getComponent<TransformComponent>(e);

                    if (m_registry->hasComponent<PlayerShipComponent>(e)) {
                        auto& ship = m_registry->getComponent<PlayerShipComponent>(e);
                        if (!ship.isRenderable()) {
                            static bool loggedShip = false;
                            if (!loggedShip) {
                                std::cerr << "[RenderSystem] PlayerShip entity " << e
                                          << " has isVisible=false, isInvincible=" << ship.isInvincible << std::endl;
                                loggedShip = true;
                            }
                        } else if (ship.isInvincible) {
                            // Log when ship is invincible (flashing)
                            static int invincibleCount = 0;
                            if (++invincibleCount <= 3) {  // Log first 3 times only
                                std::cout << "[RenderSystem] PlayerShip " << e << " is invincible (flashing)" << std::endl;
                            }
                            ship.updateAnimation(dt);
                            ship.render(transform, ctx);
                        } else {
                            ship.updateAnimation(dt);
                            ship.render(transform, ctx);
                        }
                    } else if (m_registry->hasComponent<SpritesheetComponent>(e)) {
                        auto& sheet = m_registry->getComponent<SpritesheetComponent>(e);
                        if (!sheet.isRenderable()) {
                            static bool loggedSheet = false;
                            if (!loggedSheet) {
                                std::cerr << "[RenderSystem] Spritesheet entity " << e << " has isVisible=false" << std::endl;
                                loggedSheet = true;
                            }
                        } else {
                            sheet.updateAnimation(dt);
                            sheet.render(transform, ctx);
                        }
                    } else if (m_registry->hasComponent<SpriteComponent>(e)) {
                        const auto& sprite = m_registry->getComponent<SpriteComponent>(e);
                        if (sprite.isRenderable()) {
                            sprite.render(transform, ctx);
                        }
                    }
                }

                drawUI();
                
                if (m_overlayCallback) m_overlayCallback();

                EndDrawing();
            }

            /**
             * @brief Get the system phase
             * @return SystemPhase
             */
            SystemPhase getPhase() const override { return SystemPhase::Render; }
            /**
             * @brief Get the screen width
             * @return Screen width
             */
            int getScreenWidth() const { return m_screenWidth; }
            /**
             * @brief Get the screen height
             * @return Screen height
             */
            int getScreenHeight() const { return m_screenHeight; }

            /**
             * @brief Get the loaded textures map
             * @return Pointer to textures map
             */
            const std::unordered_map<std::string, Texture2D>* getTextures() const {
                return &m_textures;
            }

            /**
             * @brief Set a callback function to draw overlay UI
             * @param callback Function to call after main rendering
             */
            void setOverlayCallback(std::function<void()> callback) {
                m_overlayCallback = callback;
            }

            /**
             * @brief Load a texture from file
             * @param id Texture identifier
             * @param path File path to load from
             * @return true if loaded successfully
             */
            bool loadTexture(const std::string& id, const std::string& path) {
                if (m_textures.count(id)) return true;
                Texture2D tex = LoadTexture(path.c_str());
                if (tex.id == 0) return false;
                m_textures[id] = tex;
                return true;
            }

            /**
             * @brief Get a texture by ID
             * @param id Texture identifier
             * @return Pointer to texture, or nullptr if not found
             */
            Texture2D* getTexture(const std::string& id) {
                auto it = m_textures.find(id);
                return (it != m_textures.end()) ? &it->second : nullptr;
            }

        private:
            /**
             * @brief Screen dimensions
             */
            int m_screenWidth, m_screenHeight;
            /**
             * @brief Animation time accumulator
             */
            float m_animTime;
            /**
             * @brief Cache of last entity count for optimization
             */
            std::size_t m_lastEntityCount = 0;  // Cache for vector reserve optimization
            /**
             * @brief Loaded textures map
             */
            std::unordered_map<std::string, Texture2D> m_textures;
            /**
             * @brief Optional overlay rendering callback
             */
            std::function<void()> m_overlayCallback;
            /**
             * @brief Pointer to UIManager for rendering UI
             */
            rtype::ui::UIManager* m_uiManager = nullptr;
            /**
             * @brief Pointer to current GameState
             */
            const GameState* m_gameState = nullptr;

            // ==================== Helpers ====================

            /**
             * @brief Get the render layer of an entity based on its components
             * @param e EntityId
             * @return Render layer (default 0 if none found)
             */
            int getLayer(EntityId e) const {
                if (m_registry->hasComponent<PlayerShipComponent>(e))
                    return m_registry->getComponent<PlayerShipComponent>(e).getRenderLayer();
                if (m_registry->hasComponent<SpriteComponent>(e))
                    return m_registry->getComponent<SpriteComponent>(e).getRenderLayer();
                if (m_registry->hasComponent<SpritesheetComponent>(e))
                    return m_registry->getComponent<SpritesheetComponent>(e).getRenderLayer();
                return 0;
            }

            /**
             * @brief Load essential textures at startup
             */
            void loadTextures() {
                const char* paths[] = {
                    "../assets/sprites/touhou_bullets.png",
                    "assets/sprites/touhou_bullets.png",
                    "./touhou_bullets.png"
                };
                for (const char* p : paths) {
                    if (loadTexture("touhou_bullets", p)) break;
                }
            }

            // ==================== UI ====================

            /**
             * @brief Draw the game UI elements
             */
            void drawUI() {
                // Show game UI only when playing or paused
                if (m_gameState && (*m_gameState == GameState::PLAYING || *m_gameState == GameState::PAUSED)) {
                    // Get player score from PlayerComponent
                    int playerScore = 0;
                    m_registry->forEach<PlayerComponent>(
                        [this, &playerScore](EntityId e) {
                            const auto& player = m_registry->getComponent<PlayerComponent>(e);
                            if (player.isLocal) {
                                playerScore = player.score;
                            }
                        }
                    );

                    // Score display
                    DrawText("SCORE", 20, 15, 20, {150, 150, 150, 255});
                    char scoreText[16];
                    snprintf(scoreText, sizeof(scoreText), "%08d", playerScore);
                    DrawText(scoreText, 20, 40, 28, WHITE);
                    
                    // Lives display
                    int playerLives = 0;
                    m_registry->forEach<PlayerComponent>(
                        [this, &playerLives](EntityId e) {
                            const auto& player = m_registry->getComponent<PlayerComponent>(e);
                            if (player.isLocal) {
                                playerLives = player.lives;
                            }
                        }
                    );
                    DrawText("LIVES", 180, 15, 20, {150, 150, 150, 255});
                    char livesText[8];
                    snprintf(livesText, sizeof(livesText), "%d", playerLives);
                    DrawText(livesText, 180, 40, 28, WHITE);

                    DrawText("R-TYPE", m_screenWidth - 90, 15, 20, {100, 100, 255, 255});
                    DrawFPS(m_screenWidth - 80, m_screenHeight - 25);
                    DrawText("[O] Debug  [G] Bullets  [Space] Shoot  [ESC] Pause", 10, m_screenHeight - 25, 14, {80, 80, 80, 255});
                }

                // Draw pause overlay
                if (m_gameState && *m_gameState == GameState::PAUSED) {
                    // Dim background
                    DrawRectangle(0, 0, m_screenWidth, m_screenHeight, {0, 0, 0, 150});
                    
                    // Pause menu box
                    int boxWidth = 300;
                    int boxHeight = 280;
                    int boxX = (m_screenWidth - boxWidth) / 2;
                    int boxY = (m_screenHeight - boxHeight) / 2;
                    
                    DrawRectangle(boxX, boxY, boxWidth, boxHeight, {20, 20, 40, 240});
                    DrawRectangleLines(boxX, boxY, boxWidth, boxHeight, {100, 100, 255, 255});
                    
                    // Title
                    DrawText("PAUSED", boxX + 90, boxY + 20, 32, WHITE);
                    
                    // Instructions (basic - full menu handled by UI system)
                    DrawText("[ESC] Resume", boxX + 70, boxY + 80, 20, {200, 200, 200, 255});
                    DrawText("[S] Settings", boxX + 70, boxY + 120, 20, {200, 200, 200, 255});
                    DrawText("[M] Main Menu", boxX + 70, boxY + 160, 20, {200, 200, 200, 255});
                    DrawText("[Q] Quit Game", boxX + 70, boxY + 200, 20, {200, 200, 200, 255});
                }
                
                // Show menu UI only when in menu state
                if (m_uiManager && m_gameState && *m_gameState == GameState::MENU) {
                    m_uiManager->render();
                }
            }
        };
} // namespace rtype::ecs
