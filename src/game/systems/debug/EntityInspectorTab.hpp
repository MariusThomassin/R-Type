/*
** R-Type ECS - Entity Inspector Tab
** Browse and inspect all entities and their components
*/

#pragma once

#include "DebugTab.hpp"
#include "../../../engine/ecs/components/TransformComponent.hpp"
#include "../../../engine/ecs/components/VelocityComponent.hpp"
#include "../../components/SpritesheetComponent.hpp"
#include "../../components/PlayerShipComponent.hpp"
#include "../../components/BackgroundComponent.hpp"
#include "../../../engine/ecs/components/LifetimeComponent.hpp"
#include "../../components/PlayerComponent.hpp"
#include "../../components/WeaponComponent.hpp"
#include "../../components/ProjectileComponent.hpp"
#include "../../../engine/ecs/components/SpriteComponent.hpp"
#include <vector>
#include <cstdio>

namespace rtype::ecs::debug {

    class EntityInspectorTab : public IDebugTab {
    public:
        const char* getName() const override { return "Entities"; }

        void update(float dt) override {
            m_animTime += dt;
            
            m_refreshTimer += dt;
            if (m_refreshTimer > 0.3f) {
                refreshEntityList();
                m_refreshTimer = 0.0f;
            }
        }

        void draw(int y) override {
            m_startY = y;
            
            DrawText("Entity Inspector", 30, y, 20, WHITE);
            
            char countBuf[64];
            snprintf(countBuf, sizeof(countBuf), "Total: %d entities", static_cast<int>(m_entities.size()));
            DrawText(countBuf, 200, y + 4, 14, {150, 150, 150, 255});
            y += 35;

            m_listX = 20;
            m_listY = y;
            m_listWidth = 260;
            m_listHeight = m_screenHeight - y - 50;
            
            DrawRectangle(m_listX, m_listY, m_listWidth, m_listHeight, {25, 25, 35, 240});
            DrawRectangleLines(m_listX, m_listY, m_listWidth, m_listHeight, {60, 60, 80, 255});
            
            BeginScissorMode(m_listX, m_listY, m_listWidth, m_listHeight);
            int itemY = m_listY + 5 - m_listScroll;
            for (size_t i = 0; i < m_entities.size(); ++i) {
                if (itemY > m_listY - 25 && itemY < m_listY + m_listHeight) {
                    drawEntityItem(static_cast<int>(i), m_listX + 5, itemY);
                }
                itemY += 24;
            }
            EndScissorMode();
            
            if (m_entities.size() * 24 > static_cast<size_t>(m_listHeight)) {
                int scrollbarH = std::max(30, m_listHeight * m_listHeight / static_cast<int>(m_entities.size() * 24));
                int scrollbarY = m_listY + (m_listScroll * (m_listHeight - scrollbarH)) / 
                                std::max(1, static_cast<int>(m_entities.size() * 24) - m_listHeight);
                DrawRectangle(m_listX + m_listWidth - 8, scrollbarY, 6, scrollbarH, {80, 80, 100, 200});
            }

            m_detailX = m_listX + m_listWidth + 15;
            m_detailY = y;
            m_detailWidth = m_screenWidth - m_detailX - 20;
            m_detailHeight = m_listHeight;
            
            DrawRectangle(m_detailX, m_detailY, m_detailWidth, m_detailHeight, {30, 30, 40, 240});
            DrawRectangleLines(m_detailX, m_detailY, m_detailWidth, m_detailHeight, {60, 80, 100, 255});

            if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_entities.size())) {
                drawComponentDetails(m_entities[m_selectedIndex]);
            } else {
                DrawText("Select an entity to inspect", m_detailX + 20, m_detailY + 20, 14, {100, 100, 120, 255});
            }
        }

        void handleMouse(const MouseInput& mouse) override {
            int my = static_cast<int>(mouse.y);
            
            if (isMouseOver(m_listX, m_listY, m_listWidth, m_listHeight)) {
                m_listScroll -= static_cast<int>(mouse.wheelDelta * 40);
                
                int maxScroll = std::max(0, static_cast<int>(m_entities.size() * 24) - m_listHeight + 10);
                m_listScroll = std::clamp(m_listScroll, 0, maxScroll);
            }

            if (isMouseOver(m_detailX, m_detailY, m_detailWidth, m_detailHeight)) {
                m_detailScroll -= static_cast<int>(mouse.wheelDelta * 40);
                m_detailScroll = std::max(0, m_detailScroll);
            }

            if (mouse.leftPressed && isMouseOver(m_listX, m_listY, m_listWidth, m_listHeight)) {
                int relY = my - m_listY + m_listScroll - 5;
                int index = relY / 24;
                if (index >= 0 && index < static_cast<int>(m_entities.size())) {
                    m_selectedIndex = index;
                    m_detailScroll = 0;
                }
            }
        }

    private:
        std::vector<EntityId> m_entities;
        int m_selectedIndex = -1;
        float m_refreshTimer = 0.0f;
        
        int m_listScroll = 0;
        int m_detailScroll = 0;
        
        int m_startY = 0;
        int m_listX = 0, m_listY = 0, m_listWidth = 0, m_listHeight = 0;
        int m_detailX = 0, m_detailY = 0, m_detailWidth = 0, m_detailHeight = 0;

        void refreshEntityList() {
            m_entities.clear();
            m_registry->forEach<TransformComponent>([this](EntityId eid) {
                m_entities.push_back(eid);
            });
            if (m_selectedIndex >= static_cast<int>(m_entities.size())) {
                m_selectedIndex = static_cast<int>(m_entities.size()) - 1;
            }
        }

        const char* getEntityType(EntityId eid) const {
            if (m_registry->hasComponent<PlayerComponent>(eid)) return "Player";
            if (m_registry->hasComponent<ProjectileComponent>(eid)) return "Bullet";
            if (m_registry->hasComponent<BackgroundComponent>(eid)) return "Background";
            if (m_registry->hasComponent<PlayerShipComponent>(eid)) return "Ship";
            if (m_registry->hasComponent<SpritesheetComponent>(eid)) return "Sprite";
            return "Entity";
        }

        Color getEntityColor(EntityId eid) const {
            if (m_registry->hasComponent<PlayerComponent>(eid)) return {100, 200, 255, 255};
            if (m_registry->hasComponent<ProjectileComponent>(eid)) return {255, 150, 100, 255};
            if (m_registry->hasComponent<BackgroundComponent>(eid)) return {100, 100, 200, 255};
            return {180, 180, 180, 255};
        }

        void drawEntityItem(int index, int x, int y) {
            EntityId eid = m_entities[index];
            bool selected = (index == m_selectedIndex);
            
            Color bg = selected ? Color{50, 70, 110, 255} : Color{35, 35, 50, 200};
            DrawRectangle(x, y, m_listWidth - 15, 22, bg);
            
            char buf[64];
            snprintf(buf, sizeof(buf), "%s #%lu", getEntityType(eid), eid);
            
            Color textCol = selected ? WHITE : getEntityColor(eid);
            DrawText(buf, x + 8, y + 4, 14, textCol);
            
            int compCount = countComponents(eid);
            char countBuf[8];
            snprintf(countBuf, sizeof(countBuf), "%d", compCount);
            DrawText(countBuf, x + m_listWidth - 35, y + 4, 12, {100, 100, 120, 255});
        }

        int countComponents(EntityId eid) const {
            int count = 0;
            if (m_registry->hasComponent<TransformComponent>(eid)) count++;
            if (m_registry->hasComponent<VelocityComponent>(eid)) count++;
            if (m_registry->hasComponent<PlayerComponent>(eid)) count++;
            if (m_registry->hasComponent<PlayerShipComponent>(eid)) count++;
            if (m_registry->hasComponent<WeaponComponent>(eid)) count++;
            if (m_registry->hasComponent<SpritesheetComponent>(eid)) count++;
            if (m_registry->hasComponent<SpriteComponent>(eid)) count++;
            if (m_registry->hasComponent<ProjectileComponent>(eid)) count++;
            if (m_registry->hasComponent<BackgroundComponent>(eid)) count++;
            if (m_registry->hasComponent<LifetimeComponent>(eid)) count++;
            return count;
        }

        void drawComponentDetails(EntityId eid) {
            BeginScissorMode(m_detailX, m_detailY, m_detailWidth, m_detailHeight);
            
            int x = m_detailX + 15;
            int y = m_detailY + 15 - m_detailScroll;
            char buf[256];

            snprintf(buf, sizeof(buf), "Entity #%lu (%s)", eid, getEntityType(eid));
            DrawText(buf, x, y, 16, {255, 255, 100, 255});
            y += 30;

            if (auto* t = m_registry->tryGetComponent<TransformComponent>(eid)) {
                y = drawComponentHeader("TransformComponent", x, y, {100, 255, 100, 255});
                snprintf(buf, sizeof(buf), "Position: (%.1f, %.1f)", t->x, t->y);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 18;
                snprintf(buf, sizeof(buf), "Rotation: %.1f deg", t->rotation);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 18;
                snprintf(buf, sizeof(buf), "Scale: (%.2f, %.2f)", t->scaleX, t->scaleY);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 25;
            }

            if (auto* v = m_registry->tryGetComponent<VelocityComponent>(eid)) {
                y = drawComponentHeader("VelocityComponent", x, y, {100, 255, 100, 255});
                snprintf(buf, sizeof(buf), "Velocity: (%.1f, %.1f)", v->vx, v->vy);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 18;
                snprintf(buf, sizeof(buf), "Max Speed: %.1f", v->maxSpeed);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 25;
            }

            if (auto* p = m_registry->tryGetComponent<PlayerComponent>(eid)) {
                y = drawComponentHeader("PlayerComponent", x, y, {100, 200, 255, 255});
                snprintf(buf, sizeof(buf), "Player ID: %d", p->playerId);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 18;
                snprintf(buf, sizeof(buf), "Lives: %d  Score: %d", p->lives, p->score);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 25;
            }

            if (auto* s = m_registry->tryGetComponent<PlayerShipComponent>(eid)) {
                y = drawComponentHeader("PlayerShipComponent (IRenderable)", x, y, {255, 200, 100, 255});
                snprintf(buf, sizeof(buf), "Color: RGB(%d, %d, %d)", s->mainR, s->mainG, s->mainB);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 18;
                snprintf(buf, sizeof(buf), "Scale: %.1f  Layer: %d", s->pixelScale, s->layer);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 18;
                snprintf(buf, sizeof(buf), "Shield: %s  Invincible: %s", 
                         s->shieldActive ? "ON" : "off", s->isInvincible ? "YES" : "no");
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 25;
            }

            if (auto* w = m_registry->tryGetComponent<WeaponComponent>(eid)) {
                y = drawComponentHeader("WeaponComponent", x, y, {255, 100, 100, 255});
                snprintf(buf, sizeof(buf), "Fire Rate: %.2fs  Damage: %d", w->fireRate, w->damage);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 18;
                snprintf(buf, sizeof(buf), "Projectile Speed: %.0f", w->projectileSpeed);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 25;
            }

            if (auto* sp = m_registry->tryGetComponent<SpritesheetComponent>(eid)) {
                y = drawComponentHeader("SpritesheetComponent (IRenderable)", x, y, {255, 200, 100, 255});
                snprintf(buf, sizeof(buf), "Frame: (%d, %d)  Size: %dx%d", 
                         sp->frameX, sp->frameY, sp->frameWidth, sp->frameHeight);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 18;
                snprintf(buf, sizeof(buf), "Glow: %s (%.1f)", sp->hasGlow ? "yes" : "no", sp->glowIntensity);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 25;
            }

            if (auto* pr = m_registry->tryGetComponent<ProjectileComponent>(eid)) {
                y = drawComponentHeader("ProjectileComponent", x, y, {255, 150, 150, 255});
                snprintf(buf, sizeof(buf), "Damage: %d  Owner: %lu", pr->damage, pr->ownerId);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 18;
                snprintf(buf, sizeof(buf), "From Player: %s", pr->isPlayerProjectile ? "yes" : "no");
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 25;
            }

            if (auto* bg = m_registry->tryGetComponent<BackgroundComponent>(eid)) {
                y = drawComponentHeader("BackgroundComponent (IRenderable)", x, y, {150, 150, 255, 255});
                snprintf(buf, sizeof(buf), "Stars: %zu  Screen: %dx%d", 
                         bg->stars.size(), bg->screenWidth, bg->screenHeight);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 25;
            }

            if (auto* lt = m_registry->tryGetComponent<LifetimeComponent>(eid)) {
                y = drawComponentHeader("LifetimeComponent", x, y, {200, 200, 100, 255});
                snprintf(buf, sizeof(buf), "Remaining: %.2fs", lt->timeRemaining);
                DrawText(buf, x + 10, y, 13, {180, 180, 180, 255}); y += 18;
                
                float progress = lt->timeRemaining / (lt->timeRemaining + lt->elapsedTime);
                int barWidth = 150;
                DrawRectangle(x + 10, y, barWidth, 10, {50, 50, 60, 255});
                DrawRectangle(x + 10, y, static_cast<int>(barWidth * progress), 10, {100, 200, 100, 255});
                y += 25;
            }

            EndScissorMode();
        }

        int drawComponentHeader(const char* name, int x, int y, Color color) {
            DrawRectangle(x, y, m_detailWidth - 30, 20, {40, 40, 55, 255});
            DrawText(name, x + 5, y + 3, 14, color);
            return y + 24;
        }
    };

} // namespace rtype::ecs::debug
