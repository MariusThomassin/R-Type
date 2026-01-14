/*
** R-Type ECS - Features Test Tab
** Debug tab for testing gameplay features and level assets
*/

#pragma once

#include "DebugTab.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "engine/ecs/components/VelocityComponent.hpp"
#include "engine/ecs/components/HealthComponent.hpp"
#include "engine/ecs/components/ColliderComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "game/components/PlayerComponent.hpp"
#include "game/components/WeaponComponent.hpp"
#include "game/components/ForceOrbComponent.hpp"
#include "game/components/PowerupComponent.hpp"
#include "game/components/EnemyComponent.hpp"
#include "game/components/SpritesheetComponent.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "engine/ecs/events/definitions/GameEvents.hpp"
#include "engine/ecs/events/definitions/AudioEvents.hpp"
#include "game/systems/LevelLoader.hpp"
#include <vector>
#include <cmath>
#include <cstdlib>
#include <filesystem>

namespace rtype::ecs::debug {

    class FeaturesTestTab : public IDebugTab {
    public:
        const char* getName() const override { return "Features"; }

        void init() {
            discoverAssets();
        }

        void update(float dt) override {
            if (!m_initialized) {
                discoverAssets();
                m_initialized = true;
            }
            m_animTime += dt;
            updatePlayerStats();
            updateForceOrbStats();
        }

        void draw(int y) override {
            m_startY = y;
            
            DrawText("Features Test Panel", 30, y, 20, {100, 255, 200, 255});
            y += 30;

            drawPlayerSection(y);
            y += 160;
            
            drawForceOrbSection(y);
            y += 140;
            
            drawWeaponSection(y);
            y += 120;
            
            drawLevelSection(y);
            y += 110;
            
            drawAssetTestSection(y);
        }

        void handleMouse(const MouseInput& mouse) override {
            if (!mouse.leftPressed) return;
            
            // Force Orb buttons
            if (isMouseOver(m_spawnOrbBtnX, m_spawnOrbBtnY, 120, 28)) {
                spawnOrUpgradeForceOrb();
            }
            if (isMouseOver(m_switchOrbBtnX, m_switchOrbBtnY, 100, 28)) {
                switchOrbSide();
            }
            if (isMouseOver(m_removeOrbBtnX, m_removeOrbBtnY, 100, 28)) {
                removeForceOrb();
            }
            
            // Weapon level buttons
            for (int i = 0; i <= 4; ++i) {
                if (isMouseOver(m_weaponLevelBtnX + i * 50, m_weaponLevelBtnY, 45, 28)) {
                    setWeaponLevel(i);
                }
            }
            
            // Bomb button
            if (isMouseOver(m_bombBtnX, m_bombBtnY, 140, 32)) {
                triggerBomb();
            }
            
            // Spawn enemies button
            if (isMouseOver(m_spawnEnemiesBtnX, m_spawnEnemiesBtnY, 130, 28)) {
                spawnTestEnemies();
            }
            
            // Level buttons
            for (size_t i = 0; i < m_levelFiles.size() && i < 3; ++i) {
                if (isMouseOver(m_levelBtnX + static_cast<int>(i) * 110, m_levelBtnY, 105, 28)) {
                    loadLevel(static_cast<int>(i));
                }
            }
            
            // Background buttons
            for (size_t i = 0; i < m_backgroundFiles.size() && i < 3; ++i) {
                if (isMouseOver(m_bgBtnX + static_cast<int>(i) * 70, m_bgBtnY, 65, 24)) {
                    testBackground(static_cast<int>(i));
                }
            }
            
            // Music buttons
            for (size_t i = 0; i < m_musicFiles.size() && i < 4; ++i) {
                if (isMouseOver(m_musicBtnX + static_cast<int>(i) * 85, m_musicBtnY, 80, 24)) {
                    testMusic(static_cast<int>(i));
                }
            }
            
            // Stop music button
            if (isMouseOver(m_stopMusicBtnX, m_stopMusicBtnY, 60, 24)) {
                stopMusic();
            }
            
            // Procedural background buttons
            for (int i = 0; i < 6; ++i) {
                if (isMouseOver(45 + i * 65, m_procBgBtnY, 60, 24)) {
                    testProceduralBackground(i);
                }
            }
            
            // Fast cycle toggle
            if (isMouseOver(160, m_procBgBtnY + 28, 50, 20)) {
                m_fastCycleMode = !m_fastCycleMode;
            }
        }

        void setEventBus(EventBus* eventBus) {
            m_eventBus = eventBus;
        }

    private:
        EventBus* m_eventBus = nullptr;
        std::vector<std::string> m_levelFiles;
        std::vector<std::string> m_backgroundFiles;
        std::vector<std::string> m_musicFiles;
        std::string m_basePath;
        bool m_initialized = false;
        bool m_fastCycleMode = false; // Fast cycle for testing day/night
        
        // Player stats cache
        EntityId m_playerEntity = NULL_ENTITY;
        int m_playerScore = 0;
        int m_playerLives = 0;
        int m_weaponLevel = 0;
        
        // Force Orb stats
        bool m_hasForceOrb = false;
        int m_orbLevel = 0;
        std::string m_orbSide = "None";
        
        // UI positions
        int m_startY = 0;
        int m_spawnOrbBtnX = 0, m_spawnOrbBtnY = 0;
        int m_switchOrbBtnX = 0, m_switchOrbBtnY = 0;
        int m_removeOrbBtnX = 0, m_removeOrbBtnY = 0;
        int m_weaponLevelBtnX = 0, m_weaponLevelBtnY = 0;
        int m_bombBtnX = 0, m_bombBtnY = 0;
        int m_spawnEnemiesBtnX = 0, m_spawnEnemiesBtnY = 0;
        int m_levelBtnX = 0, m_levelBtnY = 0;
        int m_bgBtnX = 0, m_bgBtnY = 0;
        int m_musicBtnX = 0, m_musicBtnY = 0;
        int m_stopMusicBtnX = 0, m_stopMusicBtnY = 0;
        int m_procBgBtnY = 0;

        void discoverAssets() {
            namespace fs = std::filesystem;
            
            // Try various base paths relative to where executable runs
            std::vector<std::string> basePaths = {
                "..",                              // Running from build/
                ".",                               // Running from project root
                "../..",                           // Running from build/subdir
                fs::current_path().parent_path().string()  // Parent of cwd
            };
            
            // Find base path with config/levels
            for (const auto& base : basePaths) {
                try {
                    std::string levelPath = base + "/config/levels";
                    if (fs::exists(levelPath) && fs::is_directory(levelPath)) {
                        m_basePath = base;
                        break;
                    }
                } catch (...) {}
            }
            
            // Fallback: if still empty, try parent
            if (m_basePath.empty()) {
                m_basePath = "..";
            }
            
            // Discover levels
            std::string levelDir = m_basePath + "/config/levels";
            try {
                if (fs::exists(levelDir) && fs::is_directory(levelDir)) {
                    for (const auto& entry : fs::directory_iterator(levelDir)) {
                        if (entry.path().extension() == ".json") {
                            m_levelFiles.push_back(entry.path().string());
                        }
                    }
                    std::sort(m_levelFiles.begin(), m_levelFiles.end());
                }
            } catch (...) {}
            
            // Discover backgrounds
            std::string bgDir = m_basePath + "/assets/backgrounds";
            try {
                if (fs::exists(bgDir) && fs::is_directory(bgDir)) {
                    for (const auto& entry : fs::directory_iterator(bgDir)) {
                        auto ext = entry.path().extension().string();
                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg") {
                            m_backgroundFiles.push_back(entry.path().string());
                        }
                    }
                    std::sort(m_backgroundFiles.begin(), m_backgroundFiles.end());
                }
            } catch (...) {}
            
            // Discover music
            std::string musicDir = m_basePath + "/assets/sound/music";
            try {
                if (fs::exists(musicDir) && fs::is_directory(musicDir)) {
                    for (const auto& entry : fs::directory_iterator(musicDir)) {
                        auto ext = entry.path().extension().string();
                        if (ext == ".ogg" || ext == ".mp3" || ext == ".wav") {
                            m_musicFiles.push_back(entry.path().string());
                        }
                    }
                    std::sort(m_musicFiles.begin(), m_musicFiles.end());
                }
            } catch (...) {}
        }

        void updatePlayerStats() {
            // Find local player entity
            auto entities = m_registry->getEntitiesWith<PlayerComponent>();
            for (EntityId eid : entities) {
                auto* player = m_registry->tryGetComponent<PlayerComponent>(eid);
                if (player && player->isLocal) {
                    m_playerEntity = eid;
                    m_playerScore = player->score;
                    m_playerLives = player->lives;
                    
                    if (auto* weapon = m_registry->tryGetComponent<WeaponComponent>(eid)) {
                        m_weaponLevel = weapon->powerLevel;
                    }
                    break;
                }
            }
        }

        void updateForceOrbStats() {
            m_hasForceOrb = false;
            auto orbEntities = m_registry->getEntitiesWith<ForceOrbComponent>();
            for (EntityId eid : orbEntities) {
                auto* orb = m_registry->tryGetComponent<ForceOrbComponent>(eid);
                if (orb && orb->ownerId == m_playerEntity) {
                    m_hasForceOrb = true;
                    m_orbLevel = orb->level;
                    m_orbSide = (orb->dockSide == OrbDockSide::Left) ? "Left" : "Right";
                    break;
                }
            }
            if (!m_hasForceOrb) {
                m_orbSide = "None";
                m_orbLevel = 0;
            }
        }

        void drawPlayerSection(int y) {
            DrawRectangle(30, y, 400, 150, {30, 40, 50, 230});
            DrawRectangleLines(30, y, 400, 150, {80, 120, 160, 255});
            DrawText("Player Status", 40, y + 8, 16, {100, 200, 255, 255});
            
            int infoY = y + 35;
            
            // Player ID
            char buf[64];
            snprintf(buf, sizeof(buf), "Entity: %lu", m_playerEntity);
            DrawText(buf, 45, infoY, 14, {180, 180, 180, 255});
            infoY += 22;
            
            // Score
            snprintf(buf, sizeof(buf), "Score: %d", m_playerScore);
            DrawText(buf, 45, infoY, 14, {255, 255, 100, 255});
            infoY += 22;
            
            // Lives
            DrawText("Lives:", 45, infoY, 14, {180, 180, 180, 255});
            for (int i = 0; i < 5; ++i) {
                Color c = (i < m_playerLives) ? Color{255, 80, 80, 255} : Color{60, 60, 60, 255};
                DrawCircle(110 + i * 22, infoY + 7, 8, c);
            }
            infoY += 25;
            
            // Weapon Level
            snprintf(buf, sizeof(buf), "Weapon Level: %d", m_weaponLevel);
            DrawText(buf, 45, infoY, 14, {100, 255, 200, 255});
            
            // Draw weapon pattern preview
            drawWeaponPatternPreview(280, y + 60, m_weaponLevel);
        }

        void drawWeaponPatternPreview(int x, int y, int level) {
            DrawText("Shot Pattern:", x, y - 20, 12, {150, 150, 150, 255});
            
            // Draw projectile pattern based on level
            Color bulletColor;
            switch (level) {
                case 0: bulletColor = {80, 200, 255, 255}; break;   // Cyan
                case 1: bulletColor = {80, 255, 80, 255}; break;    // Green
                case 2: bulletColor = {255, 255, 80, 255}; break;   // Yellow
                case 3: bulletColor = {255, 150, 50, 255}; break;   // Orange
                default: bulletColor = {255, 80, 255, 255}; break;  // Magenta
            }
            
            // Ship position
            int shipX = x + 20;
            int shipY = y + 30;
            DrawTriangle({(float)shipX, (float)shipY - 10}, 
                        {(float)shipX, (float)shipY + 10},
                        {(float)shipX + 20, (float)shipY}, {60, 100, 140, 255});
            
            // Projectiles based on level
            int numShots = (level == 0) ? 1 : (level == 1) ? 2 : (level == 2) ? 3 : (level == 3) ? 4 : 5;
            float offsets[] = {0, -10, 10, -20, 20};
            float angles[] = {0, 0, 0, 10, -10};  // Slight spread
            
            for (int i = 0; i < numShots; ++i) {
                float bx = shipX + 35 + i * 8;
                float by = shipY;
                if (level >= 1) by += offsets[i];
                
                DrawCircle(static_cast<int>(bx), static_cast<int>(by), 4, bulletColor);
            }
        }

        void drawForceOrbSection(int y) {
            DrawRectangle(30, y, 400, 130, {40, 35, 50, 230});
            DrawRectangleLines(30, y, 400, 130, {150, 100, 200, 255});
            DrawText("Force Orb", 40, y + 8, 16, {200, 150, 255, 255});
            
            int infoY = y + 35;
            
            // Status
            char buf[64];
            snprintf(buf, sizeof(buf), "Active: %s", m_hasForceOrb ? "YES" : "NO");
            DrawText(buf, 45, infoY, 14, m_hasForceOrb ? Color{100, 255, 100, 255} : Color{150, 150, 150, 255});
            infoY += 20;
            
            if (m_hasForceOrb) {
                snprintf(buf, sizeof(buf), "Level: %d   Side: %s", m_orbLevel, m_orbSide.c_str());
                DrawText(buf, 45, infoY, 14, {180, 180, 255, 255});
            }
            infoY += 28;
            
            // Buttons
            m_spawnOrbBtnX = 45;
            m_spawnOrbBtnY = infoY;
            Color spawnColor = m_hasForceOrb ? Color{100, 100, 60, 255} : Color{60, 120, 60, 255};
            DrawRectangle(m_spawnOrbBtnX, m_spawnOrbBtnY, 120, 28, spawnColor);
            DrawRectangleLines(m_spawnOrbBtnX, m_spawnOrbBtnY, 120, 28, {100, 200, 100, 255});
            DrawText(m_hasForceOrb ? "Upgrade Orb" : "Spawn Orb", m_spawnOrbBtnX + 10, m_spawnOrbBtnY + 7, 13, WHITE);
            
            m_switchOrbBtnX = 175;
            m_switchOrbBtnY = infoY;
            DrawRectangle(m_switchOrbBtnX, m_switchOrbBtnY, 100, 28, {60, 60, 100, 255});
            DrawRectangleLines(m_switchOrbBtnX, m_switchOrbBtnY, 100, 28, {100, 100, 200, 255});
            DrawText("Switch Side", m_switchOrbBtnX + 8, m_switchOrbBtnY + 7, 13, WHITE);
            
            m_removeOrbBtnX = 285;
            m_removeOrbBtnY = infoY;
            DrawRectangle(m_removeOrbBtnX, m_removeOrbBtnY, 100, 28, {100, 50, 50, 255});
            DrawRectangleLines(m_removeOrbBtnX, m_removeOrbBtnY, 100, 28, {200, 100, 100, 255});
            DrawText("Remove Orb", m_removeOrbBtnX + 8, m_removeOrbBtnY + 7, 13, WHITE);
        }

        void drawWeaponSection(int y) {
            DrawRectangle(30, y, 400, 110, {35, 45, 40, 230});
            DrawRectangleLines(30, y, 400, 110, {120, 180, 120, 255});
            DrawText("Weapon & Bomb", 40, y + 8, 16, {150, 255, 150, 255});
            
            int ctrlY = y + 35;
            
            // Weapon level buttons
            DrawText("Set Level:", 45, ctrlY, 14, {180, 180, 180, 255});
            m_weaponLevelBtnX = 130;
            m_weaponLevelBtnY = ctrlY - 3;
            
            for (int i = 0; i <= 4; ++i) {
                bool selected = (m_weaponLevel == i);
                Color btnColor = selected ? Color{80, 150, 80, 255} : Color{50, 60, 50, 255};
                Color borderColor;
                switch (i) {
                    case 0: borderColor = {80, 200, 255, 255}; break;
                    case 1: borderColor = {80, 255, 80, 255}; break;
                    case 2: borderColor = {255, 255, 80, 255}; break;
                    case 3: borderColor = {255, 150, 50, 255}; break;
                    default: borderColor = {255, 80, 255, 255}; break;
                }
                
                int btnX = m_weaponLevelBtnX + i * 50;
                DrawRectangle(btnX, m_weaponLevelBtnY, 45, 28, btnColor);
                DrawRectangleLines(btnX, m_weaponLevelBtnY, 45, 28, borderColor);
                
                char buf[8];
                snprintf(buf, sizeof(buf), "L%d", i);
                DrawText(buf, btnX + 13, m_weaponLevelBtnY + 7, 14, selected ? WHITE : borderColor);
            }
            
            ctrlY += 40;
            
            // Bomb button
            m_bombBtnX = 45;
            m_bombBtnY = ctrlY;
            
            float pulse = 0.8f + 0.2f * std::sin(m_animTime * 3.0f);
            Color bombColor = {
                static_cast<unsigned char>(255 * pulse),
                static_cast<unsigned char>(100 * pulse),
                static_cast<unsigned char>(50 * pulse), 255
            };
            DrawRectangle(m_bombBtnX, m_bombBtnY, 140, 32, {80, 40, 30, 255});
            DrawRectangleLines(m_bombBtnX, m_bombBtnY, 140, 32, bombColor);
            DrawText("TRIGGER BOMB", m_bombBtnX + 15, m_bombBtnY + 9, 14, bombColor);
            
            // Spawn enemies button
            m_spawnEnemiesBtnX = 200;
            m_spawnEnemiesBtnY = ctrlY;
            DrawRectangle(m_spawnEnemiesBtnX, m_spawnEnemiesBtnY, 130, 28, {80, 50, 50, 255});
            DrawRectangleLines(m_spawnEnemiesBtnX, m_spawnEnemiesBtnY, 130, 28, {200, 100, 100, 255});
            DrawText("Spawn Enemies", m_spawnEnemiesBtnX + 10, m_spawnEnemiesBtnY + 7, 13, WHITE);
        }

        void drawLevelSection(int y) {
            DrawRectangle(30, y, 400, 100, {40, 40, 55, 230});
            DrawRectangleLines(30, y, 400, 100, {100, 120, 180, 255});
            DrawText("Level Loader", 40, y + 8, 16, {120, 150, 255, 255});
            
            int ctrlY = y + 35;
            
            if (m_levelFiles.empty()) {
                DrawText("No level files found in config/levels/", 45, ctrlY, 13, {180, 100, 100, 255});
            } else {
                DrawText("Load Level:", 45, ctrlY, 14, {180, 180, 180, 255});
                m_levelBtnX = 45;
                m_levelBtnY = ctrlY + 25;
                
                const char* levelNames[] = {"Level 1", "Level 2", "Level 3"};
                Color levelColors[] = {{100, 200, 100, 255}, {200, 200, 100, 255}, {200, 100, 100, 255}};
                
                for (size_t i = 0; i < m_levelFiles.size() && i < 3; ++i) {
                    int btnX = m_levelBtnX + static_cast<int>(i) * 110;
                    DrawRectangle(btnX, m_levelBtnY, 105, 28, {40, 50, 70, 255});
                    DrawRectangleLines(btnX, m_levelBtnY, 105, 28, levelColors[i]);
                    DrawText(levelNames[i], btnX + 20, m_levelBtnY + 7, 14, levelColors[i]);
                }
            }
        }

        void drawAssetTestSection(int y) {
            DrawRectangle(30, y, 400, 180, {45, 40, 55, 230});
            DrawRectangleLines(30, y, 400, 180, {150, 120, 200, 255});
            DrawText("Asset Testing", 40, y + 8, 16, {180, 150, 255, 255});
            
            int ctrlY = y + 32;
            
            // Backgrounds
            char bgLabel[32];
            snprintf(bgLabel, sizeof(bgLabel), "Backgrounds (%zu):", m_backgroundFiles.size());
            DrawText(bgLabel, 45, ctrlY, 12, {180, 180, 180, 255});
            
            m_bgBtnX = 45;
            m_bgBtnY = ctrlY + 18;
            
            for (size_t i = 0; i < m_backgroundFiles.size() && i < 3; ++i) {
                namespace fs = std::filesystem;
                int btnX = m_bgBtnX + static_cast<int>(i) * 70;
                std::string name = fs::path(m_backgroundFiles[i]).stem().string();
                if (name.length() > 6) name = name.substr(0, 6) + "..";
                
                DrawRectangle(btnX, m_bgBtnY, 65, 24, {50, 60, 80, 255});
                DrawRectangleLines(btnX, m_bgBtnY, 65, 24, {100, 150, 200, 255});
                DrawText(name.c_str(), btnX + 4, m_bgBtnY + 6, 11, WHITE);
            }
            
            ctrlY += 50;
            
            // Music
            char musicLabel[32];
            snprintf(musicLabel, sizeof(musicLabel), "Music (%zu):", m_musicFiles.size());
            DrawText(musicLabel, 45, ctrlY, 12, {180, 180, 180, 255});
            
            m_musicBtnX = 45;
            m_musicBtnY = ctrlY + 18;
            
            for (size_t i = 0; i < m_musicFiles.size() && i < 4; ++i) {
                namespace fs = std::filesystem;
                int btnX = m_musicBtnX + static_cast<int>(i) * 85;
                std::string name = fs::path(m_musicFiles[i]).stem().string();
                if (name.length() > 8) name = name.substr(0, 8) + "..";
                
                DrawRectangle(btnX, m_musicBtnY, 80, 24, {60, 50, 70, 255});
                DrawRectangleLines(btnX, m_musicBtnY, 80, 24, {200, 150, 200, 255});
                DrawText(name.c_str(), btnX + 4, m_musicBtnY + 6, 11, WHITE);
            }
            
            // Stop music button
            m_stopMusicBtnX = 385;
            m_stopMusicBtnY = m_musicBtnY;
            DrawRectangle(m_stopMusicBtnX, m_stopMusicBtnY, 40, 24, {80, 40, 40, 255});
            DrawRectangleLines(m_stopMusicBtnX, m_stopMusicBtnY, 40, 24, {200, 80, 80, 255});
            DrawText("Stop", m_stopMusicBtnX + 6, m_stopMusicBtnY + 6, 11, WHITE);
            
            ctrlY += 50;
            
            // Procedural backgrounds
            DrawText("SFX Backgrounds:", 45, ctrlY, 12, {180, 180, 180, 255});
            m_procBgBtnY = ctrlY + 18;
            
            const char* bgNames[] = {"Sunrise", "City", "Space", "Future", "Ocean", "Mount"};
            Color bgColors[] = {
                {255, 150, 80, 255}, {100, 120, 180, 255}, {150, 100, 200, 255},
                {80, 255, 200, 255}, {80, 150, 255, 255}, {100, 180, 100, 255}
            };
            
            for (int i = 0; i < 6; ++i) {
                int btnX = 45 + i * 65;
                DrawRectangle(btnX, m_procBgBtnY, 60, 24, {40, 40, 50, 255});
                DrawRectangleLines(btnX, m_procBgBtnY, 60, 24, bgColors[i]);
                DrawText(bgNames[i], btnX + 4, m_procBgBtnY + 6, 10, bgColors[i]);
            }
            
            // Fast cycle toggle
            ctrlY += 50;
            DrawText("Fast Cycle (10s):", 45, ctrlY, 12, {180, 180, 180, 255});
            int toggleX = 160;
            DrawRectangle(toggleX, ctrlY - 2, 50, 20, m_fastCycleMode ? Color{50, 150, 50, 255} : Color{50, 50, 60, 255});
            DrawRectangleLines(toggleX, ctrlY - 2, 50, 20, {100, 100, 110, 255});
            DrawText(m_fastCycleMode ? "ON" : "OFF", toggleX + 15, ctrlY + 2, 12, WHITE);
        }

        // === Actions ===

        void spawnOrUpgradeForceOrb() {
            if (!m_eventBus || m_playerEntity == NULL_ENTITY) return;
            
            if (m_hasForceOrb) {
                // Upgrade existing orb
                m_eventBus->emit(events::ForceOrbUpgraded{0, m_playerEntity, m_orbLevel, m_orbLevel + 1});
            } else {
                // Spawn new orb
                m_eventBus->emit(events::SpawnForceOrb{m_playerEntity});
            }
        }

        void switchOrbSide() {
            if (!m_hasForceOrb) return;
            
            // Find orb and switch side
            auto orbEntities = m_registry->getEntitiesWith<ForceOrbComponent>();
            for (EntityId eid : orbEntities) {
                auto* orb = m_registry->tryGetComponent<ForceOrbComponent>(eid);
                if (orb && orb->ownerId == m_playerEntity) {
                    orb->dockSide = (orb->dockSide == OrbDockSide::Left) ? OrbDockSide::Right : OrbDockSide::Left;
                    break;
                }
            }
        }

        void removeForceOrb() {
            auto orbEntities = m_registry->getEntitiesWith<ForceOrbComponent>();
            for (EntityId eid : orbEntities) {
                auto* orb = m_registry->tryGetComponent<ForceOrbComponent>(eid);
                if (orb && orb->ownerId == m_playerEntity) {
                    m_registry->destroyEntity(eid);
                    break;
                }
            }
        }

        void setWeaponLevel(int level) {
            if (m_playerEntity == NULL_ENTITY) return;
            
            auto* weapon = m_registry->tryGetComponent<WeaponComponent>(m_playerEntity);
            if (weapon) {
                weapon->powerLevel = level;
            }
        }

        void triggerBomb() {
            if (!m_eventBus || m_playerEntity == NULL_ENTITY) return;
            
            // Get player position for bomb effect center
            float bombX = 640.0f, bombY = 360.0f;
            if (auto* transform = m_registry->tryGetComponent<TransformComponent>(m_playerEntity)) {
                bombX = transform->x;
                bombY = transform->y;
            }
            m_eventBus->emit(events::BombActivated{m_playerEntity, bombX, bombY});
        }

        void spawnTestEnemies() {
            // Spawn a wave of test enemies
            for (int i = 0; i < 5; ++i) {
                Entity e = m_registry->createEntity();
                
                float x = m_screenWidth - 50.0f;
                float y = 100.0f + i * 120.0f;
                
                m_registry->addComponent(e, TransformComponent(x, y, 0, 1.0f, 1.0f));
                m_registry->addComponent(e, VelocityComponent(-80.0f - i * 10.0f, 0.0f, 150.0f));
                m_registry->addComponent(e, HealthComponent(2));  // Weak enemy for bomb testing
                m_registry->addComponent(e, EnemyComponent(EnemyType::Basic, 1, 100));
                m_registry->addComponent(e, ColliderComponent(32.0f, 32.0f));
                m_registry->addComponent(e, LifetimeComponent(15.0f));
                
                // Visual indicator
                SpritesheetComponent sprite;
                sprite.textureId = "enemy_basic";
                sprite.frameWidth = 32;
                sprite.frameHeight = 32;
                m_registry->addComponent(e, sprite);
            }
        }

        void loadLevel(int index) {
            if (index < 0 || index >= static_cast<int>(m_levelFiles.size())) return;
            if (!m_eventBus) return;
            
            auto levelConfig = LevelLoader::loadFromFile(m_levelFiles[index]);
            if (levelConfig) {
                // Prepend base path to asset paths
                std::string bgPath = levelConfig->background;
                std::string stagePath = levelConfig->stageMusic;
                std::string bossPath = levelConfig->bossMusic;
                
                if (!bgPath.empty() && bgPath[0] != '/' && bgPath[0] != '.') {
                    bgPath = m_basePath + "/" + bgPath;
                }
                if (!stagePath.empty() && stagePath[0] != '/' && stagePath[0] != '.') {
                    stagePath = m_basePath + "/" + stagePath;
                }
                if (!bossPath.empty() && bossPath[0] != '/' && bossPath[0] != '.') {
                    bossPath = m_basePath + "/" + bossPath;
                }
                
                // Emit level assets loaded first (for background/music)
                m_eventBus->emit(events::LevelAssetsLoaded{
                    bgPath,
                    stagePath,
                    bossPath,
                    !bgPath.empty(),
                    !stagePath.empty(),
                    !bossPath.empty()
                });
                
                // Emit level loaded info
                m_eventBus->emit(events::LevelLoaded{
                    static_cast<int>(levelConfig->waves.size()),
                    levelConfig->difficulty
                });
                
                // Emit load level request to trigger enemy spawning
                // This will be picked up by EnemySpawnerSystem
                m_eventBus->emit(events::LoadLevelRequest{
                    m_levelFiles[index]
                });
            }
        }
        
        void testBackground(int index) {
            if (index < 0 || index >= static_cast<int>(m_backgroundFiles.size())) return;
            if (!m_eventBus) return;
            
            m_eventBus->emit(events::BackgroundChangeRequest{
                m_backgroundFiles[index],
                -100
            });
        }
        
        void testMusic(int index) {
            if (index < 0 || index >= static_cast<int>(m_musicFiles.size())) return;
            if (!m_eventBus) return;
            
            m_eventBus->emit(events::PlayMusic{
                m_musicFiles[index],
                1.0f,
                true
            });
        }
        
        void stopMusic() {
            if (!m_eventBus) return;
            m_eventBus->emit(events::StopMusic{});
        }
        
        void testProceduralBackground(int type) {
            if (!m_eventBus) return;
            float duration = m_fastCycleMode ? 10.0f : 120.0f;
            m_eventBus->emit(events::ProceduralBgChangeRequest{type, -101, duration});
        }
    };

} // namespace rtype::ecs::debug
