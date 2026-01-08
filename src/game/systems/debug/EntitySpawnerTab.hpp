/*
** R-Type ECS - Entity Spawner Tab
** Interactive entity spawning and management demo
** Supports bullets, enemies, powerups, and custom entities with placeholder visuals
*/

#pragma once

#include "DebugTab.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"
#include "game/components/SpritesheetComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "game/components/BulletSprites.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/PowerupComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>

namespace rtype::ecs::debug {

    /**
     * @brief Placeholder visual component for entities without sprites
     * Renders a simple geometric shape with color and label
     */
    struct PlaceholderVisualComponent : public IComponent {
        enum class Shape { Circle, Square, Diamond, Triangle, Hexagon };
        
        Shape shape = Shape::Circle;
        Color color = WHITE;
        Color borderColor = GRAY;
        float size = 24.0f;
        const char* label = "";
        bool pulsing = false;
        
        PlaceholderVisualComponent() = default;
        PlaceholderVisualComponent(Shape s, Color c, float sz, const char* lbl = "")
            : shape(s), color(c), size(sz), label(lbl) {}
        
        std::string getTypeName() const override { return "PlaceholderVisualComponent"; }
    };

    /**
     * @brief Entity category for spawner
     */
    enum class SpawnCategory {
        Bullet,
        Enemy,
        Powerup,
        Custom
    };

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
            y += 30;

            // Category tabs
            drawCategoryTabs(y);
            y += 35;

            // Main panel
            int panelX = 30, panelY = y, panelW = 380, panelH = 220;
            DrawRectangle(panelX, panelY, panelW, panelH, {30, 35, 45, 240});
            DrawRectangleLines(panelX, panelY, panelW, panelH, {60, 80, 100, 255});

            // Draw category-specific controls
            switch (m_category) {
                case SpawnCategory::Bullet:  drawBulletControls(panelX, panelY); break;
                case SpawnCategory::Enemy:   drawEnemyControls(panelX, panelY); break;
                case SpawnCategory::Powerup: drawPowerupControls(panelX, panelY); break;
                case SpawnCategory::Custom:  drawCustomControls(panelX, panelY); break;
            }

            // Spawn/Clear buttons
            int btnY = panelY + panelH - 45;
            m_spawnBtnX = panelX + 15;
            m_spawnBtnY = btnY;
            DrawRectangle(m_spawnBtnX, m_spawnBtnY, 100, 30, 
                         m_spawnBtnHover ? Color{80, 120, 80, 255} : Color{60, 100, 60, 255});
            DrawRectangleLines(m_spawnBtnX, m_spawnBtnY, 100, 30, {100, 180, 100, 255});
            DrawText("SPAWN", m_spawnBtnX + 25, m_spawnBtnY + 8, 14, WHITE);

            m_clearBtnX = panelX + 130;
            DrawRectangle(m_clearBtnX, m_spawnBtnY, 100, 30, 
                         m_clearBtnHover ? Color{120, 80, 80, 255} : Color{100, 60, 60, 255});
            DrawRectangleLines(m_clearBtnX, m_spawnBtnY, 100, 30, {180, 100, 100, 255});
            DrawText("CLEAR", m_clearBtnX + 28, m_spawnBtnY + 8, 14, WHITE);

            char countBuf[32];
            snprintf(countBuf, sizeof(countBuf), "Spawned: %d", m_spawnedCount);
            DrawText(countBuf, panelX + 250, m_spawnBtnY + 8, 14, {255, 255, 100, 255});

            // Stats panel
            drawStatsPanel(panelX + panelW + 20, panelY, 280, panelH);

            // Draw spawned entities with placeholder visuals
            drawSpawnedEntities();
        }

        void handleMouse(const MouseInput& mouse) override {
            // Category tabs
            for (int i = 0; i < 4; ++i) {
                if (mouse.leftPressed && isMouseOver(30 + i * 95, m_categoryTabY, 90, 25)) {
                    m_category = static_cast<SpawnCategory>(i);
                }
            }

            // Type selection (varies by category)
            handleTypeSelection(mouse);

            // Spawn/Clear buttons
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
        
        SpawnCategory m_category = SpawnCategory::Bullet;
        
        // Bullet options
        int m_bulletType = 0;
        int m_bulletColor = 0;
        
        // Enemy options
        int m_enemyType = 0;
        int m_enemyDifficulty = 1;
        
        // Powerup options
        int m_powerupType = 0;
        
        // Custom options
        int m_customShape = 0;
        int m_customColor = 0;
        float m_customSize = 24.0f;
        
        // UI state
        int m_startY = 0;
        int m_categoryTabY = 0;
        int m_typeButtonsY = 0;
        int m_secondRowY = 0;
        int m_spawnBtnX = 0, m_spawnBtnY = 0;
        int m_clearBtnX = 0;
        bool m_spawnBtnHover = false;
        bool m_clearBtnHover = false;

        void drawCategoryTabs(int& y) {
            m_categoryTabY = y;
            const char* categories[] = {"Bullets", "Enemies", "Powerups", "Custom"};
            Color tabColors[] = {
                {100, 200, 255, 255},  // Blue
                {255, 100, 100, 255},  // Red
                {100, 255, 150, 255},  // Green
                {255, 200, 100, 255}   // Orange
            };
            
            for (int i = 0; i < 4; ++i) {
                bool selected = (static_cast<int>(m_category) == i);
                int tabX = 30 + i * 95;
                Color bg = selected ? Color{50, 60, 80, 255} : Color{35, 40, 50, 255};
                DrawRectangle(tabX, y, 90, 25, bg);
                DrawRectangleLines(tabX, y, 90, 25, tabColors[i]);
                int tw = MeasureText(categories[i], 12);
                DrawText(categories[i], tabX + (90 - tw) / 2, y + 7, 12, 
                        selected ? tabColors[i] : Color{150, 150, 150, 255});
            }
        }

        void drawBulletControls(int panelX, int panelY) {
            DrawText("Bullet Spawner", panelX + 10, panelY + 8, 14, {100, 200, 255, 255});
            int ctrlY = panelY + 35;
            
            // Bullet type
            DrawText("Type:", panelX + 15, ctrlY, 13, {180, 180, 180, 255});
            m_typeButtonsY = ctrlY + 18;
            const char* types[] = {"Small", "Medium", "Large", "Star"};
            for (int i = 0; i < 4; ++i) {
                int btnX = panelX + 15 + i * 85;
                bool sel = (m_bulletType == i);
                DrawRectangle(btnX, m_typeButtonsY, 80, 22, sel ? Color{60, 100, 140, 255} : Color{45, 45, 60, 255});
                DrawRectangleLines(btnX, m_typeButtonsY, 80, 22, {80, 80, 100, 255});
                int tw = MeasureText(types[i], 12);
                DrawText(types[i], btnX + (80 - tw) / 2, m_typeButtonsY + 5, 12, sel ? WHITE : Color{150, 150, 150, 255});
            }
            ctrlY += 50;

            // Color
            DrawText("Color:", panelX + 15, ctrlY, 13, {180, 180, 180, 255});
            m_secondRowY = ctrlY + 18;
            const Color colors[] = {RED, {255, 165, 0, 255}, YELLOW, GREEN, {100, 200, 255, 255}, {200, 100, 255, 255}, WHITE};
            const char* colorNames[] = {"R", "O", "Y", "G", "B", "P", "W"};
            for (int i = 0; i < 7; ++i) {
                int btnX = panelX + 15 + i * 48;
                bool sel = (m_bulletColor == i);
                DrawRectangle(btnX, m_secondRowY, 44, 22, sel ? colors[i] : Color{45, 45, 60, 255});
                DrawRectangleLines(btnX, m_secondRowY, 44, 22, colors[i]);
                DrawText(colorNames[i], btnX + 17, m_secondRowY + 5, 12, sel ? BLACK : colors[i]);
            }
        }

        void drawEnemyControls(int panelX, int panelY) {
            DrawText("Enemy Spawner", panelX + 10, panelY + 8, 14, {255, 100, 100, 255});
            int ctrlY = panelY + 35;
            
            // Enemy type
            DrawText("Type:", panelX + 15, ctrlY, 13, {180, 180, 180, 255});
            m_typeButtonsY = ctrlY + 18;
            const char* types[] = {"Basic", "Chaser", "Shooter", "Turret", "Boss"};
            for (int i = 0; i < 5; ++i) {
                int btnX = panelX + 15 + i * 70;
                bool sel = (m_enemyType == i);
                DrawRectangle(btnX, m_typeButtonsY, 65, 22, sel ? Color{140, 60, 60, 255} : Color{45, 45, 60, 255});
                DrawRectangleLines(btnX, m_typeButtonsY, 65, 22, {150, 80, 80, 255});
                int tw = MeasureText(types[i], 11);
                DrawText(types[i], btnX + (65 - tw) / 2, m_typeButtonsY + 6, 11, sel ? WHITE : Color{150, 150, 150, 255});
            }
            ctrlY += 50;

            // Difficulty
            DrawText("Difficulty:", panelX + 15, ctrlY, 13, {180, 180, 180, 255});
            m_secondRowY = ctrlY + 18;
            for (int i = 1; i <= 5; ++i) {
                int btnX = panelX + 15 + (i - 1) * 50;
                bool sel = (m_enemyDifficulty == i);
                char buf[8];
                snprintf(buf, sizeof(buf), "%d", i);
                Color diffColor = {
                    static_cast<unsigned char>(100 + i * 30),
                    static_cast<unsigned char>(150 - i * 20),
                    static_cast<unsigned char>(150 - i * 20), 255
                };
                DrawRectangle(btnX, m_secondRowY, 45, 22, sel ? diffColor : Color{45, 45, 60, 255});
                DrawRectangleLines(btnX, m_secondRowY, 45, 22, diffColor);
                DrawText(buf, btnX + 18, m_secondRowY + 5, 12, sel ? WHITE : diffColor);
            }
        }

        void drawPowerupControls(int panelX, int panelY) {
            DrawText("Powerup Spawner", panelX + 10, panelY + 8, 14, {100, 255, 150, 255});
            int ctrlY = panelY + 35;
            
            DrawText("Type:", panelX + 15, ctrlY, 13, {180, 180, 180, 255});
            m_typeButtonsY = ctrlY + 18;
            
            struct PowerupInfo {
                const char* name;
                Color color;
            };
            PowerupInfo powerups[] = {
                {"Spread", {0, 255, 255, 255}},
                {"Speed", {255, 255, 0, 255}},
                {"Health", {0, 255, 0, 255}},
                {"Shield", {100, 150, 255, 255}},
                {"Weapon", {255, 150, 0, 255}}
            };
            
            for (int i = 0; i < 5; ++i) {
                int btnX = panelX + 15 + i * 70;
                bool sel = (m_powerupType == i);
                DrawRectangle(btnX, m_typeButtonsY, 65, 22, sel ? powerups[i].color : Color{45, 45, 60, 255});
                DrawRectangleLines(btnX, m_typeButtonsY, 65, 22, powerups[i].color);
                int tw = MeasureText(powerups[i].name, 10);
                DrawText(powerups[i].name, btnX + (65 - tw) / 2, m_typeButtonsY + 6, 10, 
                        sel ? BLACK : powerups[i].color);
            }
            ctrlY += 50;

            // Preview
            DrawText("Preview:", panelX + 15, ctrlY, 13, {180, 180, 180, 255});
            drawPlaceholderShape(panelX + 100, ctrlY + 20, PlaceholderVisualComponent::Shape::Diamond,
                                powerups[m_powerupType].color, 30.0f, true);
        }

        void drawCustomControls(int panelX, int panelY) {
            DrawText("Custom Entity", panelX + 10, panelY + 8, 14, {255, 200, 100, 255});
            int ctrlY = panelY + 35;
            
            // Shape selection
            DrawText("Shape:", panelX + 15, ctrlY, 13, {180, 180, 180, 255});
            m_typeButtonsY = ctrlY + 18;
            const char* shapes[] = {"Circle", "Square", "Diamond", "Triangle", "Hexagon"};
            for (int i = 0; i < 5; ++i) {
                int btnX = panelX + 15 + i * 70;
                bool sel = (m_customShape == i);
                DrawRectangle(btnX, m_typeButtonsY, 65, 22, sel ? Color{100, 80, 60, 255} : Color{45, 45, 60, 255});
                DrawRectangleLines(btnX, m_typeButtonsY, 65, 22, {150, 120, 80, 255});
                int tw = MeasureText(shapes[i], 10);
                DrawText(shapes[i], btnX + (65 - tw) / 2, m_typeButtonsY + 6, 10, sel ? WHITE : Color{150, 150, 150, 255});
            }
            ctrlY += 50;

            // Color
            DrawText("Color:", panelX + 15, ctrlY, 13, {180, 180, 180, 255});
            m_secondRowY = ctrlY + 18;
            const Color colors[] = {RED, {255, 165, 0, 255}, YELLOW, GREEN, {100, 200, 255, 255}, {200, 100, 255, 255}, WHITE};
            for (int i = 0; i < 7; ++i) {
                int btnX = panelX + 15 + i * 48;
                bool sel = (m_customColor == i);
                DrawRectangle(btnX, m_secondRowY, 44, 22, sel ? colors[i] : Color{45, 45, 60, 255});
                DrawRectangleLines(btnX, m_secondRowY, 44, 22, colors[i]);
            }
            
            // Preview
            drawPlaceholderShape(panelX + 15 + 7 * 48 + 30, m_secondRowY,
                                static_cast<PlaceholderVisualComponent::Shape>(m_customShape),
                                colors[m_customColor], 24.0f, false);
        }

        void drawStatsPanel(int x, int y, int w, int h) {
            DrawRectangle(x, y, w, h, {35, 30, 40, 240});
            DrawRectangleLines(x, y, w, h, {80, 60, 100, 255});
            DrawText("Spawned Entities", x + 10, y + 8, 14, {200, 150, 255, 255});

            int listY = y + 35;
            int maxShow = std::min(static_cast<int>(m_spawnedEntities.size()), 7);
            for (int i = 0; i < maxShow; ++i) {
                EntityId eid = m_spawnedEntities[m_spawnedEntities.size() - 1 - i];
                char buf[64];
                
                // Show entity type
                const char* type = "Entity";
                if (m_registry->hasComponent<EnemyComponent>(eid)) type = "Enemy";
                else if (m_registry->hasComponent<PowerupComponent>(eid)) type = "Powerup";
                else if (m_registry->hasComponent<ProjectileComponent>(eid)) type = "Bullet";
                else if (m_registry->hasComponent<PlaceholderVisualComponent>(eid)) type = "Custom";
                
                if (auto* lt = m_registry->tryGetComponent<LifetimeComponent>(eid)) {
                    snprintf(buf, sizeof(buf), "%s #%lu  TTL: %.1fs", type, eid, lt->timeRemaining);
                    float progress = lt->timeRemaining / 8.0f;
                    DrawRectangle(x + 180, listY + 3, 80, 12, {40, 40, 50, 255});
                    DrawRectangle(x + 180, listY + 3, static_cast<int>(80 * progress), 12, 
                                 {static_cast<unsigned char>(255 * (1 - progress)), 
                                  static_cast<unsigned char>(200 * progress), 100, 255});
                } else {
                    snprintf(buf, sizeof(buf), "%s #%lu", type, eid);
                }
                DrawText(buf, x + 15, listY, 12, {180, 180, 180, 255});
                listY += 22;
            }
            
            if (m_spawnedEntities.size() > 7) {
                char moreBuf[32];
                snprintf(moreBuf, sizeof(moreBuf), "... and %d more", 
                        static_cast<int>(m_spawnedEntities.size()) - 7);
                DrawText(moreBuf, x + 15, listY, 11, {120, 120, 140, 255});
            }
        }

        void drawSpawnedEntities() {
            // Draw placeholder visuals for entities that have them
            for (EntityId eid : m_spawnedEntities) {
                if (!m_registry->entityExists(eid)) continue;
                
                auto* transform = m_registry->tryGetComponent<TransformComponent>(eid);
                if (!transform) continue;
                
                // Check if entity has a placeholder visual
                auto* placeholder = m_registry->tryGetComponent<PlaceholderVisualComponent>(eid);
                if (placeholder) {
                    drawPlaceholderShape(
                        static_cast<int>(transform->x),
                        static_cast<int>(transform->y),
                        placeholder->shape,
                        placeholder->color,
                        placeholder->size * transform->scaleX,
                        placeholder->pulsing
                    );
                    
                    // Draw label if present
                    if (placeholder->label && placeholder->label[0] != '\0') {
                        int tw = MeasureText(placeholder->label, 10);
                        DrawText(placeholder->label, 
                                static_cast<int>(transform->x) - tw / 2,
                                static_cast<int>(transform->y) + static_cast<int>(placeholder->size) + 5,
                                10, placeholder->color);
                    }
                }
            }
        }

        void drawPlaceholderShape(int x, int y, PlaceholderVisualComponent::Shape shape, 
                                  Color color, float size, bool pulsing) {
            float pulse = pulsing ? (0.8f + 0.2f * std::sin(m_animTime * 4.0f)) : 1.0f;
            float sz = size * pulse;
            
            Color fillColor = {
                static_cast<unsigned char>(color.r * 0.6f),
                static_cast<unsigned char>(color.g * 0.6f),
                static_cast<unsigned char>(color.b * 0.6f),
                200
            };
            
            switch (shape) {
                case PlaceholderVisualComponent::Shape::Circle:
                    DrawCircle(x, y, sz, fillColor);
                    DrawCircleLines(x, y, sz, color);
                    break;
                    
                case PlaceholderVisualComponent::Shape::Square:
                    DrawRectangle(x - static_cast<int>(sz), y - static_cast<int>(sz), 
                                 static_cast<int>(sz * 2), static_cast<int>(sz * 2), fillColor);
                    DrawRectangleLines(x - static_cast<int>(sz), y - static_cast<int>(sz),
                                      static_cast<int>(sz * 2), static_cast<int>(sz * 2), color);
                    break;
                    
                case PlaceholderVisualComponent::Shape::Diamond: {
                    Vector2 pts[4] = {
                        {static_cast<float>(x), static_cast<float>(y) - sz},
                        {static_cast<float>(x) + sz, static_cast<float>(y)},
                        {static_cast<float>(x), static_cast<float>(y) + sz},
                        {static_cast<float>(x) - sz, static_cast<float>(y)}
                    };
                    DrawTriangle(pts[0], pts[1], pts[2], fillColor);
                    DrawTriangle(pts[0], pts[2], pts[3], fillColor);
                    DrawLineV(pts[0], pts[1], color);
                    DrawLineV(pts[1], pts[2], color);
                    DrawLineV(pts[2], pts[3], color);
                    DrawLineV(pts[3], pts[0], color);
                    break;
                }
                    
                case PlaceholderVisualComponent::Shape::Triangle: {
                    Vector2 pts[3] = {
                        {static_cast<float>(x), static_cast<float>(y) - sz},
                        {static_cast<float>(x) + sz * 0.866f, static_cast<float>(y) + sz * 0.5f},
                        {static_cast<float>(x) - sz * 0.866f, static_cast<float>(y) + sz * 0.5f}
                    };
                    DrawTriangle(pts[0], pts[1], pts[2], fillColor);
                    DrawTriangleLines(pts[0], pts[1], pts[2], color);
                    break;
                }
                    
                case PlaceholderVisualComponent::Shape::Hexagon: {
                    for (int i = 0; i < 6; ++i) {
                        float a1 = (i * 60.0f - 30.0f) * DEG2RAD;
                        float a2 = ((i + 1) * 60.0f - 30.0f) * DEG2RAD;
                        Vector2 p1 = {x + sz * std::cos(a1), y + sz * std::sin(a1)};
                        Vector2 p2 = {x + sz * std::cos(a2), y + sz * std::sin(a2)};
                        Vector2 center = {static_cast<float>(x), static_cast<float>(y)};
                        DrawTriangle(center, p1, p2, fillColor);
                        DrawLineV(p1, p2, color);
                    }
                    break;
                }
            }
        }

        void handleTypeSelection(const MouseInput& mouse) {
            if (!mouse.leftPressed) return;

            switch (m_category) {
                case SpawnCategory::Bullet:
                    for (int i = 0; i < 4; ++i) {
                        if (isMouseOver(30 + 15 + i * 85, m_typeButtonsY, 80, 22)) m_bulletType = i;
                    }
                    for (int i = 0; i < 7; ++i) {
                        if (isMouseOver(30 + 15 + i * 48, m_secondRowY, 44, 22)) m_bulletColor = i;
                    }
                    break;
                    
                case SpawnCategory::Enemy:
                    for (int i = 0; i < 5; ++i) {
                        if (isMouseOver(30 + 15 + i * 70, m_typeButtonsY, 65, 22)) m_enemyType = i;
                    }
                    for (int i = 1; i <= 5; ++i) {
                        if (isMouseOver(30 + 15 + (i - 1) * 50, m_secondRowY, 45, 22)) m_enemyDifficulty = i;
                    }
                    break;
                    
                case SpawnCategory::Powerup:
                    for (int i = 0; i < 5; ++i) {
                        if (isMouseOver(30 + 15 + i * 70, m_typeButtonsY, 65, 22)) m_powerupType = i;
                    }
                    break;
                    
                case SpawnCategory::Custom:
                    for (int i = 0; i < 5; ++i) {
                        if (isMouseOver(30 + 15 + i * 70, m_typeButtonsY, 65, 22)) m_customShape = i;
                    }
                    for (int i = 0; i < 7; ++i) {
                        if (isMouseOver(30 + 15 + i * 48, m_secondRowY, 44, 22)) m_customColor = i;
                    }
                    break;
            }
        }

        void spawnEntity() {
            switch (m_category) {
                case SpawnCategory::Bullet:  spawnBullet(); break;
                case SpawnCategory::Enemy:   spawnEnemy(); break;
                case SpawnCategory::Powerup: spawnPowerup(); break;
                case SpawnCategory::Custom:  spawnCustom(); break;
            }
            m_spawnedCount++;
        }

        void spawnBullet() {
            Entity e = m_registry->createEntity();
            
            float x = 200.0f + static_cast<float>(std::rand() % (m_screenWidth - 400));
            float y = 150.0f + static_cast<float>(std::rand() % (m_screenHeight - 350));
            
            m_registry->addComponent(e, TransformComponent(x, y, 0, 1.5f, 1.5f));
            m_registry->addComponent(e, VelocityComponent(
                static_cast<float>((std::rand() % 200) - 100),
                static_cast<float>((std::rand() % 200) - 100), 200.0f));

            SpritesheetComponent sprite;
            BulletType types[] = {BulletType::Pellet, BulletType::Ball, BulletType::BallLarge, BulletType::Star};
            sprite.setBullet(types[m_bulletType], static_cast<BulletColor>(m_bulletColor));
            sprite.hasGlow = true;
            sprite.glowIntensity = 0.5f;
            m_registry->addComponent(e, sprite);
            
            m_registry->addComponent(e, ProjectileComponent(NULL_ENTITY, 10, true));
            m_registry->addComponent(e, LifetimeComponent(8.0f));

            m_spawnedEntities.push_back(e.id);
        }

        void spawnEnemy() {
            Entity e = m_registry->createEntity();
            
            float x = 200.0f + static_cast<float>(std::rand() % (m_screenWidth - 400));
            float y = 150.0f + static_cast<float>(std::rand() % (m_screenHeight - 350));
            
            EnemyType types[] = {EnemyType::Basic, EnemyType::Chaser, EnemyType::Shooter, 
                                 EnemyType::Turret, EnemyType::Boss};
            EnemyType type = types[m_enemyType];
            
            float baseSpeed = (type == EnemyType::Turret) ? 0.0f : 50.0f + m_enemyDifficulty * 20.0f;
            float size = (type == EnemyType::Boss) ? 48.0f : 24.0f + m_enemyDifficulty * 4.0f;
            int health = (type == EnemyType::Boss) ? 500 : 50 * m_enemyDifficulty;
            
            m_registry->addComponent(e, TransformComponent(x, y, 0, 1.0f, 1.0f));
            m_registry->addComponent(e, VelocityComponent(
                static_cast<float>((std::rand() % 100) - 50) * (baseSpeed / 50.0f),
                static_cast<float>((std::rand() % 100) - 50) * (baseSpeed / 50.0f), 
                baseSpeed));
            m_registry->addComponent(e, EnemyComponent(type, m_enemyDifficulty, 100 * m_enemyDifficulty));
            m_registry->addComponent(e, HealthComponent(health));
            m_registry->addComponent(e, ColliderComponent(size, size));
            
            // Placeholder visual (no sprite yet)
            Color enemyColors[] = {{255, 100, 100, 255}, {255, 150, 50, 255}, {200, 50, 200, 255},
                                   {100, 100, 200, 255}, {255, 50, 50, 255}};
            PlaceholderVisualComponent::Shape shapes[] = {
                PlaceholderVisualComponent::Shape::Triangle,
                PlaceholderVisualComponent::Shape::Diamond,
                PlaceholderVisualComponent::Shape::Hexagon,
                PlaceholderVisualComponent::Shape::Square,
                PlaceholderVisualComponent::Shape::Hexagon
            };
            
            PlaceholderVisualComponent placeholder(shapes[m_enemyType], enemyColors[m_enemyType], size);
            const char* labels[] = {"Basic", "Chaser", "Shooter", "Turret", "BOSS"};
            placeholder.label = labels[m_enemyType];
            placeholder.pulsing = (type == EnemyType::Boss);
            m_registry->addComponent(e, placeholder);
            
            m_registry->addComponent(e, LifetimeComponent(8.0f));

            m_spawnedEntities.push_back(e.id);
        }

        void spawnPowerup() {
            Entity e = m_registry->createEntity();
            
            float x = 200.0f + static_cast<float>(std::rand() % (m_screenWidth - 400));
            float y = 150.0f + static_cast<float>(std::rand() % (m_screenHeight - 350));
            
            PowerupType types[] = {PowerupType::SPREAD_SHOT, PowerupType::SPEED_BOOST,
                                   PowerupType::HEALTH_UP, PowerupType::SHIELD, PowerupType::WEAPON_UPGRADE};
            
            m_registry->addComponent(e, TransformComponent(x, y, 0, 1.0f, 1.0f));
            m_registry->addComponent(e, VelocityComponent(0, 30.0f, 50.0f));  // Slow drift down
            m_registry->addComponent(e, PowerupComponent(types[m_powerupType], 10.0f, 1.0f));
            m_registry->addComponent(e, ColliderComponent(24.0f, 24.0f));
            
            // Placeholder visual
            Color powerupColors[] = {{0, 255, 255, 255}, {255, 255, 0, 255}, {0, 255, 0, 255},
                                     {100, 150, 255, 255}, {255, 150, 0, 255}};
            PlaceholderVisualComponent placeholder(PlaceholderVisualComponent::Shape::Diamond, 
                                                   powerupColors[m_powerupType], 20.0f);
            placeholder.pulsing = true;
            placeholder.label = PowerupComponent::getName(types[m_powerupType]);
            m_registry->addComponent(e, placeholder);
            
            m_registry->addComponent(e, LifetimeComponent(8.0f));

            m_spawnedEntities.push_back(e.id);
        }

        void spawnCustom() {
            Entity e = m_registry->createEntity();
            
            float x = 200.0f + static_cast<float>(std::rand() % (m_screenWidth - 400));
            float y = 150.0f + static_cast<float>(std::rand() % (m_screenHeight - 350));
            
            const Color colors[] = {RED, {255, 165, 0, 255}, YELLOW, GREEN, 
                                   {100, 200, 255, 255}, {200, 100, 255, 255}, WHITE};
            
            m_registry->addComponent(e, TransformComponent(x, y, 0, 1.0f, 1.0f));
            m_registry->addComponent(e, VelocityComponent(
                static_cast<float>((std::rand() % 150) - 75),
                static_cast<float>((std::rand() % 150) - 75), 150.0f));
            
            PlaceholderVisualComponent placeholder(
                static_cast<PlaceholderVisualComponent::Shape>(m_customShape),
                colors[m_customColor], m_customSize);
            placeholder.pulsing = (m_customShape == 2);  // Diamond pulses
            m_registry->addComponent(e, placeholder);
            
            m_registry->addComponent(e, ColliderComponent(m_customSize, m_customSize));
            m_registry->addComponent(e, LifetimeComponent(8.0f));

            m_spawnedEntities.push_back(e.id);
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
