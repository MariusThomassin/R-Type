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
#include "../components/SpritesheetComponent.hpp"
#include "../components/PlayerShipComponent.hpp"
#include "../components/BackgroundComponent.hpp"
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
    PLAYING
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
        RenderSystem(int screenWidth = 1280, int screenHeight = 720)
            : m_screenWidth(screenWidth)
            , m_screenHeight(screenHeight)
            , m_animTime(0.0f) {
            loadTextures();
        }

        ~RenderSystem() override {
            if (IsWindowReady()) {
                for (auto& [id, tex] : m_textures) {
                    UnloadTexture(tex);
                }
            }
        }

        void setUIManager(rtype::ui::UIManager* uiManager) {
            m_uiManager = uiManager;
        }
        
        void setGameStatePtr(const GameState* gameState) {
            m_gameState = gameState;
        }

        void update(float dt) override {
            if (!m_registry) return;

            m_animTime += dt;

            RenderContext ctx{&m_textures, m_screenWidth, m_screenHeight, m_animTime, dt};

            BeginDrawing();
            ClearBackground({8, 8, 20, 255});
            
            m_registry->forEach<TransformComponent, BackgroundComponent>(
                [this, &ctx, dt](EntityId e) {
                    auto& background = m_registry->getComponent<BackgroundComponent>(e);
                    const auto& transform = m_registry->getComponent<TransformComponent>(e);
                    background.updateAnimation(dt);
                    background.render(transform, ctx);
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

            // Sort using cached layer values (O(n log n) but with fast comparisons)
            std::sort(entities.begin(), entities.end(), [](const RenderItem& a, const RenderItem& b) {
                return a.layer < b.layer;
            });

            for (const RenderItem& item : entities) {
                EntityId e = item.entity;
                const auto& transform = m_registry->getComponent<TransformComponent>(e);
                
                if (m_registry->hasComponent<PlayerShipComponent>(e)) {
                    auto& ship = m_registry->getComponent<PlayerShipComponent>(e);
                    if (ship.isRenderable()) {
                        ship.updateAnimation(dt);
                        ship.render(transform, ctx);
                    }
                } else if (m_registry->hasComponent<SpritesheetComponent>(e)) {
                    auto& sheet = m_registry->getComponent<SpritesheetComponent>(e);
                    if (sheet.isRenderable()) {
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

            drawUI(ctx);
            
            if (m_overlayCallback) m_overlayCallback();

            EndDrawing();
        }

        SystemPhase getPhase() const override { return SystemPhase::Render; }
        int getScreenWidth() const { return m_screenWidth; }
        int getScreenHeight() const { return m_screenHeight; }

        const std::unordered_map<std::string, Texture2D>* getTextures() const {
            return &m_textures;
        }

        void setOverlayCallback(std::function<void()> callback) {
            m_overlayCallback = callback;
        }

        bool loadTexture(const std::string& id, const std::string& path) {
            if (m_textures.count(id)) return true;
            Texture2D tex = LoadTexture(path.c_str());
            if (tex.id == 0) return false;
            m_textures[id] = tex;
            return true;
        }

        Texture2D* getTexture(const std::string& id) {
            auto it = m_textures.find(id);
            return (it != m_textures.end()) ? &it->second : nullptr;
        }

    private:
        int m_screenWidth, m_screenHeight;
        float m_animTime;
        std::size_t m_lastEntityCount = 0;  // Cache for vector reserve optimization
        std::unordered_map<std::string, Texture2D> m_textures;
        std::function<void()> m_overlayCallback;
        rtype::ui::UIManager* m_uiManager = nullptr;
        const GameState* m_gameState = nullptr;

        // ==================== Helpers ====================

        int getLayer(EntityId e) const {
            if (m_registry->hasComponent<PlayerShipComponent>(e))
                return m_registry->getComponent<PlayerShipComponent>(e).getRenderLayer();
            if (m_registry->hasComponent<SpriteComponent>(e))
                return m_registry->getComponent<SpriteComponent>(e).getRenderLayer();
            if (m_registry->hasComponent<SpritesheetComponent>(e))
                return m_registry->getComponent<SpritesheetComponent>(e).getRenderLayer();
            return 0;
        }

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

        void drawUI(const RenderContext& ctx) {
            // Show game UI only when playing
            if (m_gameState && *m_gameState == GameState::PLAYING) {
                DrawText("SCORE", 20, 15, 20, {150, 150, 150, 255});
                DrawText("00000", 20, 40, 28, WHITE);
                DrawText("R-TYPE", m_screenWidth - 90, 15, 20, {100, 100, 255, 255});
                DrawFPS(m_screenWidth - 80, m_screenHeight - 25);
                DrawText("[O] Debug  [G] Bullets  [Space] Shoot", 10, m_screenHeight - 25, 14, {80, 80, 80, 255});
            }
            
            // Show menu UI only when in menu state
            if (m_uiManager && m_gameState && *m_gameState == GameState::MENU) {
                m_uiManager->render();
            }
        }
    };

} // namespace rtype::ecs
