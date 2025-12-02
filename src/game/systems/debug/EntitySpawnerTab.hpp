/*
** R-Type ECS - Entity Spawner Tab
** Interactive entity spawning and management demo
*/

#pragma once

#include "DebugTab.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "game/components/SpritesheetComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "game/components/BulletSprites.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>

namespace rtype::ecs::debug {

    class EntitySpawnerTab : public IDebugTab {
    public:
        const char* getName() const override { return "Spawner"; }

        void update(float dt) override {
            m_animTime += dt;
            
            cleanupExpiredEntities();
        }

        void draw(int y) override {
            m_startY = y;
            
            DrawText("Entity Spawner", 30, y, 20, WHITE);
            y += 35;

            int panelX = 30, panelY = y, panelW = 350, panelH = 200;
            DrawRectangle(panelX, panelY, panelW, panelH, {30, 35, 45, 240});
            DrawRectangleLines(panelX, panelY, panelW, panelH, {60, 80, 100, 255});
            DrawText("Spawn Controls", panelX + 10, panelY + 8, 14, {100, 200, 255, 255});

            int ctrlY = panelY + 35;
            
            DrawText("Bullet Type:", panelX + 15, ctrlY, 13, {180, 180, 180, 255});
            m_typeButtonsY = ctrlY + 18;
            for (int i = 0; i < 4; ++i) {
                int btnX = panelX + 15 + i * 80;
                bool selected = (m_selectedType == i);
                Color bg = selected ? Color{60, 100, 140, 255} : Color{45, 45, 60, 255};
                DrawRectangle(btnX, m_typeButtonsY, 75, 22, bg);
                DrawRectangleLines(btnX, m_typeButtonsY, 75, 22, {80, 80, 100, 255});
                
                const char* names[] = {"Small", "Medium", "Large", "Star"};
                int tw = MeasureText(names[i], 12);
                DrawText(names[i], btnX + (75 - tw) / 2, m_typeButtonsY + 5, 12, 
                        selected ? WHITE : Color{150, 150, 150, 255});
            }
            ctrlY += 50;

            DrawText("Color:", panelX + 15, ctrlY, 13, {180, 180, 180, 255});
            m_colorButtonsY = ctrlY + 18;
            const Color colors[] = {RED, {255, 165, 0, 255}, YELLOW, GREEN, {100, 200, 255, 255}, 
                                   {200, 100, 255, 255}, WHITE};
            const char* colorNames[] = {"R", "O", "Y", "G", "B", "P", "W"};
            for (int i = 0; i < 7; ++i) {
                int btnX = panelX + 15 + i * 45;
                bool selected = (m_selectedColor == i);
                DrawRectangle(btnX, m_colorButtonsY, 40, 22, selected ? colors[i] : Color{45, 45, 60, 255});
                DrawRectangleLines(btnX, m_colorButtonsY, 40, 22, colors[i]);
                DrawText(colorNames[i], btnX + 15, m_colorButtonsY + 5, 12, 
                        selected ? BLACK : colors[i]);
            }
            ctrlY += 50;

            m_spawnBtnY = ctrlY;
            m_spawnBtnX = panelX + 15;
            DrawRectangle(m_spawnBtnX, m_spawnBtnY, 100, 30, m_spawnBtnHover ? Color{80, 120, 80, 255} : Color{60, 100, 60, 255});
            DrawRectangleLines(m_spawnBtnX, m_spawnBtnY, 100, 30, {100, 180, 100, 255});
            DrawText("SPAWN", m_spawnBtnX + 25, m_spawnBtnY + 8, 14, WHITE);

            m_clearBtnX = panelX + 130;
            DrawRectangle(m_clearBtnX, m_spawnBtnY, 100, 30, m_clearBtnHover ? Color{120, 80, 80, 255} : Color{100, 60, 60, 255});
            DrawRectangleLines(m_clearBtnX, m_spawnBtnY, 100, 30, {180, 100, 100, 255});
            DrawText("CLEAR", m_clearBtnX + 28, m_spawnBtnY + 8, 14, WHITE);

            char countBuf[32];
            snprintf(countBuf, sizeof(countBuf), "Spawned: %d", m_spawnedCount);
            DrawText(countBuf, panelX + 250, m_spawnBtnY + 8, 14, {255, 255, 100, 255});

            int statsX = panelX + panelW + 20, statsY = panelY, statsW = 300, statsH = panelH;
            DrawRectangle(statsX, statsY, statsW, statsH, {35, 30, 40, 240});
            DrawRectangleLines(statsX, statsY, statsW, statsH, {80, 60, 100, 255});
            DrawText("Spawned Entities", statsX + 10, statsY + 8, 14, {200, 150, 255, 255});

            int listY = statsY + 35;
            int maxShow = std::min(static_cast<int>(m_spawnedEntities.size()), 6);
            for (int i = 0; i < maxShow; ++i) {
                EntityId eid = m_spawnedEntities[m_spawnedEntities.size() - 1 - i];
                char buf[64];
                
                if (auto* lt = m_registry->tryGetComponent<LifetimeComponent>(eid)) {
                    snprintf(buf, sizeof(buf), "Entity #%lu  TTL: %.1fs", eid, lt->timeRemaining);
                    
                    float progress = lt->timeRemaining / 5.0f;
                    DrawRectangle(statsX + 200, listY + 3, 80, 12, {40, 40, 50, 255});
                    DrawRectangle(statsX + 200, listY + 3, static_cast<int>(80 * progress), 12, 
                                 {static_cast<unsigned char>(255 * (1 - progress)), 
                                  static_cast<unsigned char>(200 * progress), 100, 255});
                } else {
                    snprintf(buf, sizeof(buf), "Entity #%lu", eid);
                }
                DrawText(buf, statsX + 15, listY, 13, {180, 180, 180, 255});
                listY += 22;
            }
            
            if (m_spawnedEntities.size() > 6) {
                char moreBuf[32];
                snprintf(moreBuf, sizeof(moreBuf), "... and %d more", 
                        static_cast<int>(m_spawnedEntities.size()) - 6);
                DrawText(moreBuf, statsX + 15, listY, 12, {120, 120, 140, 255});
            }

            y = panelY + panelH + 20;
            DrawText("Code Example:", 30, y, 14, {100, 200, 255, 255});
            y += 22;
            
            Color codeColor = {160, 200, 160, 255};
            DrawText("Entity e = registry.createEntity();", 40, y, 12, codeColor); y += 16;
            DrawText("registry.addComponent(e, TransformComponent(x, y));", 40, y, 12, codeColor); y += 16;
            DrawText("registry.addComponent(e, VelocityComponent(vx, vy, maxSpeed));", 40, y, 12, codeColor); y += 16;
            DrawText("registry.addComponent(e, SpritesheetComponent(...));", 40, y, 12, codeColor); y += 16;
            DrawText("registry.addComponent(e, LifetimeComponent(5.0f));", 40, y, 12, codeColor);
        }

        void handleMouse(const MouseInput& mouse) override {
            for (int i = 0; i < 4; ++i) {
                int btnX = 30 + 15 + i * 80;
                if (mouse.leftPressed && isMouseOver(btnX, m_typeButtonsY, 75, 22)) {
                    m_selectedType = i;
                }
            }

            for (int i = 0; i < 7; ++i) {
                int btnX = 30 + 15 + i * 45;
                if (mouse.leftPressed && isMouseOver(btnX, m_colorButtonsY, 40, 22)) {
                    m_selectedColor = i;
                }
            }

            m_spawnBtnHover = isMouseOver(m_spawnBtnX, m_spawnBtnY, 100, 30);
            if (mouse.leftPressed && m_spawnBtnHover) {
                spawnEntity();
            }

            m_clearBtnHover = isMouseOver(m_clearBtnX, m_spawnBtnY, 100, 30);
            if (mouse.leftPressed && m_clearBtnHover) {
                clearEntities();
            }
        }

    private:
        std::vector<EntityId> m_spawnedEntities;
        int m_spawnedCount = 0;
        
        int m_selectedType = 0;
        int m_selectedColor = 0;
        
        int m_startY = 0;
        int m_typeButtonsY = 0;
        int m_colorButtonsY = 0;
        int m_spawnBtnX = 0, m_spawnBtnY = 0;
        int m_clearBtnX = 0;
        bool m_spawnBtnHover = false;
        bool m_clearBtnHover = false;

        void spawnEntity() {
            Entity e = m_registry->createEntity();
            
            float x = 200.0f + static_cast<float>(std::rand() % (m_screenWidth - 400));
            float y = 150.0f + static_cast<float>(std::rand() % (m_screenHeight - 350));
            
            m_registry->addComponent(e, TransformComponent(x, y, 0, 1.5f, 1.5f));
            m_registry->addComponent(e, VelocityComponent(
                static_cast<float>((std::rand() % 200) - 100),
                static_cast<float>((std::rand() % 200) - 100),
                200.0f
            ));

            SpritesheetComponent sprite;
            
            BulletType types[] = {BulletType::Pellet, BulletType::Ball,
                                  BulletType::BallLarge, BulletType::Star};
            sprite.setBullet(types[m_selectedType], static_cast<BulletColor>(m_selectedColor));
            sprite.hasGlow = true;
            sprite.glowIntensity = 0.5f;
            m_registry->addComponent(e, sprite);
            
            m_registry->addComponent(e, LifetimeComponent(5.0f));

            m_spawnedEntities.push_back(e.id);
            m_spawnedCount++;
        }

        void clearEntities() {
            for (EntityId id : m_spawnedEntities) {
                if (m_registry->entityExists(id)) {
                    m_registry->destroyEntity(id);
                }
            }
            m_spawnedEntities.clear();
        }

        void cleanupExpiredEntities() {
            m_spawnedEntities.erase(
                std::remove_if(m_spawnedEntities.begin(), m_spawnedEntities.end(),
                    [this](EntityId id) { return !m_registry->entityExists(id); }),
                m_spawnedEntities.end()
            );
        }
    };

} // namespace rtype::ecs::debug
