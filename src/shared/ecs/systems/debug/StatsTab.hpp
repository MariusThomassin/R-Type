/*
** R-Type ECS - Stats Debug Tab
** Shows entity counts and system statistics
*/

#pragma once

#include "DebugTab.hpp"
#include "../../components/TransformComponent.hpp"
#include "../../components/SpriteComponent.hpp"
#include "../../components/SpritesheetComponent.hpp"
#include "../../components/ProjectileComponent.hpp"

namespace rtype::ecs::debug {

    class StatsTab : public IDebugTab {
    public:
        const char* getName() const override { return "Stats"; }

        void update(float dt) override {
            m_animTime += dt;
            m_frameTime = dt;
        }

        void draw(int y) override {
            DrawText("Engine Statistics", 30, y, 20, WHITE); y += 35;

            int entities = 0, bullets = 0, sprites = 0, sheets = 0;
            for ([[maybe_unused]] auto _ : m_registry->getEntitiesWith<TransformComponent>()) entities++;
            for ([[maybe_unused]] auto _ : m_registry->getEntitiesWith<ProjectileComponent>()) bullets++;
            for ([[maybe_unused]] auto _ : m_registry->getEntitiesWith<SpriteComponent>()) sprites++;
            for ([[maybe_unused]] auto _ : m_registry->getEntitiesWith<SpritesheetComponent>()) sheets++;

            char buf[128];
            snprintf(buf, sizeof(buf), "Total Entities: %d", entities);
            DrawText(buf, 30, y, 16, WHITE); y += 24;
            snprintf(buf, sizeof(buf), "  - With SpriteComponent: %d", sprites);
            DrawText(buf, 30, y, 14, {180, 180, 180, 255}); y += 20;
            snprintf(buf, sizeof(buf), "  - With SpritesheetComponent: %d", sheets);
            DrawText(buf, 30, y, 14, {180, 180, 180, 255}); y += 20;
            snprintf(buf, sizeof(buf), "  - Projectiles: %d", bullets);
            DrawText(buf, 30, y, 14, {180, 180, 180, 255}); y += 30;

            snprintf(buf, sizeof(buf), "FPS: %d  |  Frame: %.2fms", GetFPS(), m_frameTime * 1000);
            DrawText(buf, 30, y, 16, {100, 255, 100, 255}); y += 40;

            DrawText("Architecture: IRenderable Pattern", 30, y, 16, {100, 200, 255, 255}); y += 24;
            DrawText("Components render themselves via render(transform, ctx)", 30, y, 14, {150, 150, 150, 255}); y += 20;
            DrawText("RenderSystem is a thin coordinator (~220 lines)", 30, y, 14, {150, 150, 150, 255}); y += 30;

            DrawText("Controls:", 30, y, 16, {255, 255, 100, 255}); y += 24;
            DrawText("WASD/Arrows - Move  |  Space - Shoot  |  G - Bullet Pattern", 30, y, 14, {150, 150, 150, 255});
        }

    private:
        float m_frameTime = 0.0f;
    };

} // namespace rtype::ecs::debug
