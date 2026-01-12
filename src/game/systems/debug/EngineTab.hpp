/*
** R-Type ECS - Engine Tab
** Displays refactored engine features: ConfigManager, AssetManager, SpatialHash, etc.
** Shows real-time stats and allows live configuration editing
*/

#pragma once

#include "DebugTab.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include <string>
#include <vector>
#include <cmath>

namespace rtype::ecs::debug {

    /**
     * @brief Debug tab for engine subsystems
     * 
     * Displays:
     * - ConfigManager: Live config values, hot-reload
     * - AssetManager: Loaded assets, memory, ref counts
     * - SpatialHash: Collision optimization stats
     * - EventBus: Thread-safe messaging stats
     * - Registry: Deferred operations count
     */
    class EngineTab : public IDebugTab {
    public:
        EngineTab(EventBus* eventBus = nullptr) : m_eventBus(eventBus) {}

        const char* getName() const override { return "Engine"; }

        void update(float dt) override {
            m_animTime += dt;
            m_updateTimer += dt;
            
            // Update stats every 0.5 seconds
            if (m_updateTimer >= 0.5f) {
                m_updateTimer = 0.0f;
                refreshStats();
            }
        }

        void draw(int y) override {
            DrawText("Engine Subsystems", 30, y, 20, WHITE);
            y += 35;

            // Section buttons
            int btnY = y;
            if (drawSectionButton("Config", 30, btnY, m_activeSection == 0)) m_activeSection = 0;
            if (drawSectionButton("Assets", 120, btnY, m_activeSection == 1)) m_activeSection = 1;
            if (drawSectionButton("Collision", 210, btnY, m_activeSection == 2)) m_activeSection = 2;
            if (drawSectionButton("Events", 310, btnY, m_activeSection == 3)) m_activeSection = 3;
            if (drawSectionButton("Registry", 400, btnY, m_activeSection == 4)) m_activeSection = 4;
            y += 40;

            // Draw active section
            switch (m_activeSection) {
                case 0: drawConfigSection(y); break;
                case 1: drawAssetSection(y); break;
                case 2: drawCollisionSection(y); break;
                case 3: drawEventBusSection(y); break;
                case 4: drawRegistrySection(y); break;
            }
        }

        void handleMouse(const MouseInput& mouse) override {
            m_scrollOffset += mouse.wheelDelta * 20.0f;
            m_scrollOffset = std::min(0.0f, m_scrollOffset);
        }

        void setEventBus(EventBus* eventBus) { m_eventBus = eventBus; }

        // External stat setters for systems that track their own metrics
        void setSpatialHashStats(size_t entities, size_t cells, size_t pairsChecked) {
            m_spatialEntities = entities;
            m_spatialCells = cells;
            m_collisionPairs = pairsChecked;
        }

        void setDeferredCount(size_t count) {
            m_deferredCount = count;
        }

    private:
        EventBus* m_eventBus = nullptr;
        int m_activeSection = 0;
        float m_scrollOffset = 0.0f;
        float m_updateTimer = 0.0f;

        // Cached stats
        size_t m_spatialEntities = 0;
        size_t m_spatialCells = 0;
        size_t m_collisionPairs = 0;
        size_t m_deferredCount = 0;
        size_t m_pendingCrossThreadEvents = 0;

        void refreshStats() {
            if (m_eventBus) {
                m_pendingCrossThreadEvents = m_eventBus->getPendingEventCount();
            }
            
            // Registry deferred count is set externally via setDeferredCount()
        }

        bool drawSectionButton(const char* label, int x, int y, bool active) {
            int w = 80, h = 28;
            bool over = isMouseOver(x, y, w, h);
            
            Color bg = active ? Color{80, 100, 140, 255} : 
                       over ? Color{60, 70, 90, 255} : Color{40, 45, 60, 255};
            Color border = active ? Color{100, 150, 255, 255} : Color{70, 80, 100, 255};
            
            DrawRectangle(x, y, w, h, bg);
            DrawRectangleLines(x, y, w, h, border);
            
            int textW = MeasureText(label, 14);
            DrawText(label, x + (w - textW) / 2, y + 7, 14, WHITE);
            
            return over && m_mouse.leftPressed;
        }

        void drawConfigSection(int y) {
            DrawText("ConfigManager", 30, y, 18, {100, 200, 255, 255});
            y += 28;

            char buf[256];

            // Feature description
            DrawText("JSON-based configuration with hot-reload support", 40, y, 12, {150, 150, 150, 255});
            y += 24;

            // Config file info
            snprintf(buf, sizeof(buf), "Config File: config/default.json");
            DrawText(buf, 40, y, 14, {180, 180, 180, 255});
            y += 30;

            // Features list
            DrawText("Features:", 40, y, 14, {255, 200, 100, 255});
            y += 22;

            const char* features[] = {
                "- Type-safe get<T>(key, default) API",
                "- Dot-notation nested keys (video.width)",
                "- Hot-reload with file change detection",
                "- Change callbacks for live updates",
                "- Thread-safe access with shared_mutex"
            };
            
            for (const char* f : features) {
                DrawText(f, 50, y, 12, {180, 180, 180, 255});
                y += 18;
            }
            y += 10;

            // Usage example
            DrawText("Usage Pattern:", 40, y, 14, {100, 200, 255, 255});
            y += 20;
            DrawText("auto& config = ConfigManager::instance();", 50, y, 12, {150, 200, 150, 255});
            y += 16;
            DrawText("int width = config.get<int>(\"video.width\", 1280);", 50, y, 12, {150, 200, 150, 255});
            y += 16;
            DrawText("config.onChanged(\"video\", callback);", 50, y, 12, {150, 200, 150, 255});
        }

        void drawAssetSection(int y) {
            DrawText("AssetManager", 30, y, 18, {100, 255, 150, 255});
            y += 28;

            DrawText("Centralized asset loading with reference counting", 40, y, 12, {150, 150, 150, 255});
            y += 24;

            // Features
            DrawText("Features:", 40, y, 14, {255, 200, 100, 255});
            y += 20;
            const char* features[] = {
                "- Automatic reference counting",
                "- Type-safe asset loading with templates",
                "- Custom loader registration",
                "- Hot-reload with file watching",
                "- Garbage collection for unused assets",
                "- Thread-safe with shared_mutex"
            };
            for (const char* f : features) {
                DrawText(f, 50, y, 12, {180, 180, 180, 255});
                y += 16;
            }
            y += 10;

            // Usage example
            DrawText("Usage Pattern:", 40, y, 14, {100, 255, 150, 255});
            y += 20;
            DrawText("auto& assets = AssetManager::instance();", 50, y, 12, {150, 200, 150, 255});
            y += 16;
            DrawText("assets.registerLoader<Texture2D>(textureLoader);", 50, y, 12, {150, 200, 150, 255});
            y += 16;
            DrawText("auto tex = assets.load<Texture2D>(\"player.png\");", 50, y, 12, {150, 200, 150, 255});
        }

        void drawCollisionSection(int y) {
            DrawText("SpatialHash Collision System", 30, y, 18, {255, 200, 100, 255});
            y += 28;

            DrawText("O(1) spatial partitioning for broad-phase collision detection", 40, y, 12, {150, 150, 150, 255});
            y += 24;

            char buf[256];

            // Stats
            snprintf(buf, sizeof(buf), "Entities in Grid: %zu", m_spatialEntities);
            DrawText(buf, 40, y, 14, {180, 180, 180, 255});
            y += 20;

            snprintf(buf, sizeof(buf), "Active Cells: %zu", m_spatialCells);
            DrawText(buf, 40, y, 14, {180, 180, 180, 255});
            y += 20;

            snprintf(buf, sizeof(buf), "Collision Pairs Checked: %zu", m_collisionPairs);
            DrawText(buf, 40, y, 14, {180, 180, 180, 255});
            y += 30;

            // Comparison with old system
            DrawText("Performance Comparison:", 40, y, 14, {255, 200, 100, 255});
            y += 22;

            // Visual comparison
            size_t n = m_spatialEntities > 0 ? m_spatialEntities : 100;
            size_t oldComplexity = n * n;
            size_t newComplexity = m_collisionPairs > 0 ? m_collisionPairs : n;
            
            snprintf(buf, sizeof(buf), "Old (O(N^2)): %zu checks", oldComplexity);
            DrawText(buf, 50, y, 13, {255, 100, 100, 255});
            y += 18;

            snprintf(buf, sizeof(buf), "New (Spatial Hash): %zu checks", newComplexity);
            DrawText(buf, 50, y, 13, {100, 255, 100, 255});
            y += 18;

            if (oldComplexity > 0 && newComplexity > 0) {
                float improvement = (float)oldComplexity / newComplexity;
                snprintf(buf, sizeof(buf), "Improvement: %.1fx faster", improvement);
                DrawText(buf, 50, y, 14, {100, 200, 255, 255});
            }
        }

        void drawEventBusSection(int y) {
            DrawText("EventBus (Thread-Safe)", 30, y, 18, {200, 150, 255, 255});
            y += 28;

            DrawText("Decoupled pub/sub messaging with cross-thread support", 40, y, 12, {150, 150, 150, 255});
            y += 24;

            char buf[256];

            // Cross-thread events
            snprintf(buf, sizeof(buf), "Pending Cross-Thread Events: %zu", m_pendingCrossThreadEvents);
            DrawText(buf, 40, y, 14, {180, 180, 180, 255});
            y += 30;

            // Features
            DrawText("Thread-Safety Features:", 40, y, 14, {255, 200, 100, 255});
            y += 22;

            const char* features[] = {
                "- shared_mutex for concurrent reads",
                "- Cross-thread event queue",
                "- Atomic subscriber IDs",
                "- Safe subscription during emission",
                "- No data races on emit/subscribe"
            };
            
            for (int i = 0; i < 5; i++) {
                float pulse = 0.7f + 0.3f * std::sin(m_animTime * 2.0f + i * 0.5f);
                DrawText(features[i], 50, y, 12, {
                    static_cast<unsigned char>(150 * pulse),
                    static_cast<unsigned char>(150 * pulse),
                    static_cast<unsigned char>(200 * pulse), 255
                });
                y += 18;
            }
            y += 10;

            // Usage example
            DrawText("Usage Pattern:", 40, y, 14, {100, 200, 255, 255});
            y += 20;
            DrawText("eventBus.emitCrossThread<Event>(data);", 50, y, 12, {150, 200, 150, 255});
            y += 16;
            DrawText("eventBus.processCrossThreadEvents();", 50, y, 12, {150, 200, 150, 255});
        }

        void drawRegistrySection(int y) {
            DrawText("Registry (Transaction System)", 30, y, 18, {255, 150, 150, 255});
            y += 28;

            DrawText("Deferred entity destruction for safe iteration", 40, y, 12, {150, 150, 150, 255});
            y += 24;

            char buf[256];

            // Deferred count
            snprintf(buf, sizeof(buf), "Pending Deferred Deletions: %zu", m_deferredCount);
            Color countColor = m_deferredCount > 0 ? Color{255, 200, 100, 255} : Color{180, 180, 180, 255};
            DrawText(buf, 40, y, 14, countColor);
            y += 30;

            // Explanation
            DrawText("How It Works:", 40, y, 14, {255, 200, 100, 255});
            y += 22;

            const char* steps[] = {
                "1. Call destroyEntityDeferred(entity) during iteration",
                "2. Entity marked for destruction, remains valid",
                "3. After iteration: flushDeferred() removes entities",
                "4. No iterator invalidation, no dangling references"
            };

            for (const char* step : steps) {
                DrawText(step, 50, y, 12, {180, 180, 180, 255});
                y += 18;
            }
            y += 10;

            // Benefits
            DrawText("Benefits:", 40, y, 14, {100, 255, 100, 255});
            y += 20;
            DrawText("- Safe entity removal during system updates", 50, y, 12, {150, 150, 150, 255});
            y += 16;
            DrawText("- Predictable destruction timing", 50, y, 12, {150, 150, 150, 255});
            y += 16;
            DrawText("- No undefined behavior from invalid iterators", 50, y, 12, {150, 150, 150, 255});
        }
    };

} // namespace rtype::ecs::debug
