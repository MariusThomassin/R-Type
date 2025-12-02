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

#include <raylib.h>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <unordered_map>
#include <string>
#include <functional>

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

        void update(float dt) override {
            if (!m_registry) return;

            m_animTime += dt;

            RenderContext ctx{&m_textures, m_screenWidth, m_screenHeight, m_animTime, dt};

            BeginDrawing();
            ClearBackground({8, 8, 20, 255});
            
            // Render backgrounds first (layer 0)
            for (EntityId e : m_registry->getEntitiesWith<TransformComponent, BackgroundComponent>()) {
                auto& background = m_registry->getComponent<BackgroundComponent>(e);
                const auto& transform = m_registry->getComponent<TransformComponent>(e);
                background.updateAnimation(dt);
                background.render(transform, ctx);
            }

            // Collect and sort all renderable entities
            std::vector<EntityId> entities;
            for (EntityId e : m_registry->getEntitiesWith<TransformComponent, SpriteComponent>()) {
                entities.push_back(e);
            }
            for (EntityId e : m_registry->getEntitiesWith<TransformComponent, SpritesheetComponent>()) {
                if (!m_registry->hasComponent<SpriteComponent>(e)) {
                    entities.push_back(e);
                }
            }
            for (EntityId e : m_registry->getEntitiesWith<TransformComponent, PlayerShipComponent>()) {
                if (!m_registry->hasComponent<SpriteComponent>(e) && !m_registry->hasComponent<SpritesheetComponent>(e)) {
                    entities.push_back(e);
                }
            }

            std::sort(entities.begin(), entities.end(), [this](EntityId a, EntityId b) {
                return getLayer(a) < getLayer(b);
            });

            for (EntityId e : entities) {
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

            drawUI();
            
            // Call overlay callback if set (for DebugSystem)
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
        std::unordered_map<std::string, Texture2D> m_textures;
        std::function<void()> m_overlayCallback;

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

        void drawUI() {
            DrawText("SCORE", 20, 15, 20, {150, 150, 150, 255});
            DrawText("00000", 20, 40, 28, WHITE);
            DrawText("R-TYPE", m_screenWidth - 90, 15, 20, {100, 100, 255, 255});
            DrawFPS(m_screenWidth - 80, m_screenHeight - 25);
            DrawText("[O] Debug  [G] Bullets  [Space] Shoot", 10, m_screenHeight - 25, 14, {80, 80, 80, 255});
        }
    };

} // namespace rtype::ecs
