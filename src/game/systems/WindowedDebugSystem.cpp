/*
** R-Type ECS - WindowedDebugSystem Implementation
** Engine-style debug UI with draggable windows
*/

#include "WindowedDebugSystem.hpp"
#include "DebugSystem.hpp"
#include <algorithm>
#include <cmath>

namespace rtype::ecs {

    WindowedDebugSystem::WindowedDebugSystem(EventBus& eventBus, int screenWidth, int screenHeight)
        : m_eventBus(eventBus)
        , m_windowManager(eventBus)
        , m_screenWidth(screenWidth)
        , m_screenHeight(screenHeight) {
        
        subscribeToEvents();
    }

    WindowedDebugSystem::~WindowedDebugSystem() {
        unsubscribeFromEvents();
    }

    void WindowedDebugSystem::subscribeToEvents() {
        m_debugToggleSub = m_eventBus.subscribe<events::DebugToggleEvent>(
            [this](const events::DebugToggleEvent&) {
                m_pendingToggle = true;
            }
        );

        m_mouseMoveSub = m_eventBus.subscribe<events::MouseMoveEvent>(
            [this](const events::MouseMoveEvent& e) {
                m_mouseX = e.x;
                m_mouseY = e.y;
            }
        );

        m_mouseClickSub = m_eventBus.subscribe<events::MouseButtonPressedEvent>(
            [this](const events::MouseButtonPressedEvent& e) {
                if (e.button == events::MouseButton::Left) m_mousePressed = true;
            }
        );
    }

    void WindowedDebugSystem::unsubscribeFromEvents() {
        m_eventBus.unsubscribe<events::DebugToggleEvent>(m_debugToggleSub);
        m_eventBus.unsubscribe<events::MouseMoveEvent>(m_mouseMoveSub);
        m_eventBus.unsubscribe<events::MouseButtonPressedEvent>(m_mouseClickSub);
    }

    void WindowedDebugSystem::init() {
        createDefaultWindows();
        
        // Set up menu categories
        m_menuCategories = {
            {"View", {"stats", "entities", "performance"}},
            {"Engine", {"engine", "architecture", "console"}},
            {"Assets", {"textures"}}
        };
    }

    void WindowedDebugSystem::createDefaultWindows() {
        // Stats Window
        auto statsWin = m_windowManager.createWindow("stats", "Stats", 20, 60, 300, 350);
        statsWin->setVisible(false);
        m_windowDrawers["stats"] = [this](float x, float y, float w, float h, float s) {
            drawStatsContent(x, y, w, h, s);
        };

        // Entity Inspector
        auto entitiesWin = m_windowManager.createWindow("entities", "Entity Inspector", 340, 60, 350, 450);
        entitiesWin->setVisible(false);
        m_windowDrawers["entities"] = [this](float x, float y, float w, float h, float s) {
            drawEntitiesContent(x, y, w, h, s);
        };

        // Performance Monitor
        auto perfWin = m_windowManager.createWindow("performance", "Performance", 20, 430, 300, 280);
        perfWin->setVisible(false);
        m_windowDrawers["performance"] = [this](float x, float y, float w, float h, float s) {
            drawPerformanceContent(x, y, w, h, s);
        };

        // Engine Systems
        auto engineWin = m_windowManager.createWindow("engine", "Engine Systems", 710, 60, 380, 400);
        engineWin->setVisible(false);
        m_windowDrawers["engine"] = [this](float x, float y, float w, float h, float s) {
            drawEngineContent(x, y, w, h, s);
        };

        // Textures
        auto texturesWin = m_windowManager.createWindow("textures", "Textures", 710, 480, 380, 230);
        texturesWin->setVisible(false);
        m_windowDrawers["textures"] = [this](float x, float y, float w, float h, float s) {
            drawTexturesContent(x, y, w, h, s);
        };

        // Architecture
        auto archWin = m_windowManager.createWindow("architecture", "Architecture", 400, 200, 500, 400);
        archWin->setVisible(false);
        m_windowDrawers["architecture"] = [this](float x, float y, float w, float h, float s) {
            drawArchitectureContent(x, y, w, h, s);
        };

        // Console
        auto consoleWin = m_windowManager.createWindow("console", "Console", 20, 500, 600, 200);
        consoleWin->setVisible(false);
        m_windowDrawers["console"] = [this](float x, float y, float w, float h, float s) {
            drawConsoleContent(x, y, w, h, s);
        };
    }

    void WindowedDebugSystem::update(float dt) {
        if (!m_registry) return;

        // Handle toggle
        if (m_pendingToggle) {
            m_enabled = !m_enabled;
            m_pendingToggle = false;
            m_windowManager.setEnabled(m_enabled);
        }

        if (!m_enabled) return;

        m_animTime += dt;
        m_lastFrameTime = dt;
        
        // Track frame times
        m_frameTimes.push_back(dt);
        if (m_frameTimes.size() > 120) {
            m_frameTimes.erase(m_frameTimes.begin());
        }

        // Update pending cross-thread events
        m_pendingCrossThreadEvents = m_eventBus.getPendingEventCount();

        // Update window manager
        m_windowManager.update(dt);
    }

    void WindowedDebugSystem::draw() {
        if (!m_enabled) return;

        // Draw semi-transparent overlay to indicate debug mode
        DrawRectangle(0, 0, m_screenWidth, 30, {25, 28, 35, 240});
        
        // Draw menu bar (will consume m_mousePressed if used)
        drawMenuBar();

        // Draw all windows safely
        const auto& windows = m_windowManager.getWindows();
        for (size_t i = 0; i < windows.size(); ++i) {
            const auto& window = windows[i];
            if (!window || !window->isVisible()) continue;
            window->render();
        }

        // Draw window contents (on top of window backgrounds)
        for (const auto& window : windows) {
            if (!window || !window->isVisible() || window->isCollapsed()) continue;
            
            auto bounds = window->getContentBounds();
            float scroll = window->getScrollOffset();
            
            // Find the drawer for this window
            for (const auto& [id, drawer] : m_windowDrawers) {
                auto mappedWindow = m_windowManager.getWindow(id);
                if (mappedWindow && mappedWindow.get() == window.get()) {
                    // Enable scissor mode to clip content to window
                    BeginScissorMode(
                        static_cast<int>(bounds.x),
                        static_cast<int>(bounds.y),
                        static_cast<int>(bounds.width),
                        static_cast<int>(bounds.height)
                    );
                    drawer(bounds.x, bounds.y - scroll, bounds.width, bounds.height, scroll);
                    EndScissorMode();
                    break;
                }
            }
        }
        
        // Reset mouse pressed state after draw is complete
        m_mousePressed = false;

        // Draw debug mode indicator
        const char* indicator = "DEBUG MODE (O to close)";
        int indicatorW = MeasureText(indicator, 12);
        DrawText(indicator, m_screenWidth - indicatorW - 10, 9, 12, {150, 150, 150, 255});
    }

    void WindowedDebugSystem::drawMenuBar() {
        int menuX = 10;
        const int menuHeight = 22;
        const int menuPadding = 12;

        // Check if we clicked outside any menu
        bool clickedOutside = m_mousePressed && m_mouseY > 30 && m_activeMenu >= 0;
        
        for (size_t i = 0; i < m_menuCategories.size(); ++i) {
            auto& cat = m_menuCategories[i];
            int textW = MeasureText(cat.name.c_str(), 14);
            int menuW = textW + menuPadding * 2;
            
            bool hover = m_mouseX >= menuX && m_mouseX < menuX + menuW &&
                        m_mouseY >= 4 && m_mouseY < 4 + menuHeight;
            bool active = (m_activeMenu == static_cast<int>(i));
            
            // Draw menu button
            if (hover || active) {
                DrawRectangle(menuX, 4, menuW, menuHeight, {50, 55, 70, 255});
            }
            DrawText(cat.name.c_str(), menuX + menuPadding, 8, 14, 
                    (hover || active) ? WHITE : Color{200, 200, 200, 255});

            // Handle click on menu
            if (hover && m_mousePressed) {
                m_activeMenu = (m_activeMenu == static_cast<int>(i)) ? -1 : static_cast<int>(i);
                clickedOutside = false;
            }

            // Draw dropdown if active
            if (active) {
                drawMenuDropdown(static_cast<int>(i), static_cast<float>(menuX), 30.0f);
            }

            menuX += menuW + 5;
        }

        // "Classic Mode" button on the right
        const char* classicLabel = "Classic Mode";
        int classicW = MeasureText(classicLabel, 12) + 16;
        int classicX = m_screenWidth - classicW - 180;  // Leave space for indicator
        
        bool classicHover = m_mouseX >= classicX && m_mouseX < classicX + classicW &&
                           m_mouseY >= 6 && m_mouseY < 6 + 18;
        
        DrawRectangle(classicX, 6, classicW, 18, classicHover ? Color{70, 80, 100, 255} : Color{45, 50, 60, 255});
        DrawRectangleLines(classicX, 6, classicW, 18, {80, 90, 110, 255});
        DrawText(classicLabel, classicX + 8, 9, 12, classicHover ? WHITE : Color{180, 180, 180, 255});
        
        if (classicHover && m_mousePressed) {
            switchToClassic();
        }

        // Close menu if clicked outside
        if (clickedOutside) {
            m_activeMenu = -1;
        }

        // Separator line
        DrawLine(0, 30, m_screenWidth, 30, {50, 55, 65, 255});
    }

    void WindowedDebugSystem::drawMenuDropdown(int menuIndex, float x, float y) {
        if (menuIndex < 0 || menuIndex >= static_cast<int>(m_menuCategories.size())) return;
        
        auto& cat = m_menuCategories[menuIndex];
        const float itemHeight = 24;
        const float dropdownWidth = 180;
        float dropdownHeight = cat.windowIds.size() * itemHeight + 8;

        // Background
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), 
                     static_cast<int>(dropdownWidth), static_cast<int>(dropdownHeight),
                     {40, 45, 55, 250});
        DrawRectangleLines(static_cast<int>(x), static_cast<int>(y),
                          static_cast<int>(dropdownWidth), static_cast<int>(dropdownHeight),
                          {70, 80, 95, 255});

        float itemY = y + 4;
        for (const auto& windowId : cat.windowIds) {
            auto window = m_windowManager.getWindow(windowId);
            if (!window) continue;

            bool visible = window->isVisible();
            bool hover = m_mouseX >= x && m_mouseX < x + dropdownWidth &&
                        m_mouseY >= itemY && m_mouseY < itemY + itemHeight;

            if (hover) {
                DrawRectangle(static_cast<int>(x + 2), static_cast<int>(itemY),
                             static_cast<int>(dropdownWidth - 4), static_cast<int>(itemHeight),
                             {60, 70, 90, 255});
            }

            // Checkmark for visible windows
            if (visible) {
                DrawText("✓", static_cast<int>(x + 8), static_cast<int>(itemY + 5), 12, 
                        {100, 200, 100, 255});
            }

            DrawText(window->getTitle().c_str(), static_cast<int>(x + 28), static_cast<int>(itemY + 5), 
                    13, hover ? WHITE : Color{200, 200, 200, 255});

            // Handle click
            if (hover && m_mousePressed) {
                m_windowManager.toggleWindow(windowId);
                m_activeMenu = -1;  // Close menu after selection
            }

            itemY += itemHeight;
        }
    }

    void WindowedDebugSystem::setTextures(const std::unordered_map<std::string, Texture2D>* textures) {
        m_textures = textures;
    }

    void WindowedDebugSystem::registerWindow(const DebugWindowConfig& config) {
        auto window = m_windowManager.createWindow(
            config.id, config.title,
            config.defaultX, config.defaultY,
            config.defaultWidth, config.defaultHeight,
            config.flags
        );
        window->setVisible(config.visibleByDefault);
        m_windowDrawers[config.id] = config.drawer;
    }

    void WindowedDebugSystem::toggleWindow(const std::string& id) {
        m_windowManager.toggleWindow(id);
    }

    void WindowedDebugSystem::showWindow(const std::string& id) {
        m_windowManager.showWindow(id);
    }

    void WindowedDebugSystem::setSpatialHashStats(size_t entities, size_t cells, size_t pairs) {
        m_spatialEntities = entities;
        m_spatialCells = cells;
        m_collisionPairs = pairs;
    }

    void WindowedDebugSystem::setDeferredCount(size_t count) {
        m_deferredCount = count;
    }

    void WindowedDebugSystem::updateShowoffState(bool active, const std::string& patternName,
                                                  int currentPhase, int totalPhases, float progress) {
        m_showoffActive = active;
        m_showoffPatternName = patternName;
        m_showoffCurrentPhase = currentPhase;
        m_showoffTotalPhases = totalPhases;
        m_showoffProgress = progress;
    }

    void WindowedDebugSystem::updateStressTestState(bool active, bool complete, int intensity,
                                                     float progress, const std::string& reportFilename) {
        m_stressTestActive = active;
        m_stressTestComplete = complete;
        m_stressTestIntensity = intensity;
        m_stressTestProgress = progress;
        m_stressTestReportFilename = reportFilename;
    }

    // === Window Content Drawers ===

    void WindowedDebugSystem::drawStatsContent(float x, float y, float w, float h, float scroll) {
        char buf[128];
        float lineY = y + 5;
        const float lineH = 20;

        DrawText("Game Statistics", static_cast<int>(x + 5), static_cast<int>(lineY), 16, 
                {100, 200, 255, 255});
        lineY += lineH + 10;

        snprintf(buf, sizeof(buf), "FPS: %d", GetFPS());
        DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 14, {100, 255, 100, 255});
        lineY += lineH;

        snprintf(buf, sizeof(buf), "Frame Time: %.2f ms", m_lastFrameTime * 1000);
        DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 14, WHITE);
        lineY += lineH;

        if (m_registry) {
            snprintf(buf, sizeof(buf), "Entities: %zu", m_registry->getEntityCount());
            DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 14, WHITE);
            lineY += lineH;
        }

        lineY += 10;
        DrawLine(static_cast<int>(x + 10), static_cast<int>(lineY), 
                static_cast<int>(x + w - 10), static_cast<int>(lineY), {60, 65, 80, 255});
        lineY += 15;

        DrawText("Memory", static_cast<int>(x + 5), static_cast<int>(lineY), 14, {255, 200, 100, 255});
        lineY += lineH;

        // Simulated memory stats
        DrawText("ECS Registry: ~2.4 MB", static_cast<int>(x + 10), static_cast<int>(lineY), 12, 
                {180, 180, 180, 255});
        lineY += 16;
        DrawText("Textures: ~48.2 MB", static_cast<int>(x + 10), static_cast<int>(lineY), 12,
                {180, 180, 180, 255});
        lineY += 16;
        DrawText("Audio: ~12.1 MB", static_cast<int>(x + 10), static_cast<int>(lineY), 12,
                {180, 180, 180, 255});
    }

    void WindowedDebugSystem::drawEntitiesContent(float x, float y, float w, float h, float scroll) {
        if (!m_registry) {
            DrawText("No registry", static_cast<int>(x + 10), static_cast<int>(y + 10), 14, RED);
            return;
        }

        float lineY = y + 5;
        char buf[128];

        DrawText("Entity List", static_cast<int>(x + 5), static_cast<int>(lineY), 16, 
                {100, 200, 255, 255});
        lineY += 30;

        // Get all entities (simplified - would need proper entity iteration)
        size_t entityCount = m_registry->getEntityCount();
        snprintf(buf, sizeof(buf), "Total: %zu entities", entityCount);
        DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 12, {180, 180, 180, 255});
        lineY += 25;

        // Component type counts (placeholder)
        DrawText("Components by Type:", static_cast<int>(x + 5), static_cast<int>(lineY), 14, 
                {255, 200, 100, 255});
        lineY += 22;

        const char* components[] = {"Transform", "Velocity", "Sprite", "Health", "Collider"};
        for (const char* comp : components) {
            snprintf(buf, sizeof(buf), "  %s: ~%d", comp, rand() % 100 + 10);
            DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 12, {160, 160, 160, 255});
            lineY += 18;
        }
    }

    void WindowedDebugSystem::drawPerformanceContent(float x, float y, float w, float h, float scroll) {
        float lineY = y + 5;
        char buf[128];

        DrawText("Performance Metrics", static_cast<int>(x + 5), static_cast<int>(lineY), 16, 
                {100, 200, 255, 255});
        lineY += 30;

        // Current stats
        snprintf(buf, sizeof(buf), "FPS: %d", GetFPS());
        DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 18, {100, 255, 100, 255});
        lineY += 25;

        // Frame time graph
        DrawText("Frame Time Graph:", static_cast<int>(x + 5), static_cast<int>(lineY), 12, 
                {180, 180, 180, 255});
        lineY += 18;

        float graphX = x + 10;
        float graphY = lineY;
        float graphW = w - 20;
        float graphH = 100;

        DrawRectangle(static_cast<int>(graphX), static_cast<int>(graphY),
                     static_cast<int>(graphW), static_cast<int>(graphH), {30, 35, 45, 255});

        // 16.67ms line (60fps target)
        float targetY = graphY + graphH - (16.67f / 50.0f * graphH);
        DrawLine(static_cast<int>(graphX), static_cast<int>(targetY),
                static_cast<int>(graphX + graphW), static_cast<int>(targetY), {100, 255, 100, 80});

        // Draw frame times
        if (!m_frameTimes.empty()) {
            float barW = graphW / m_frameTimes.size();
            for (size_t i = 0; i < m_frameTimes.size(); ++i) {
                float ft = m_frameTimes[i] * 1000;
                float barH = std::min(ft / 50.0f * graphH, graphH);
                
                Color c;
                if (ft > 33.33f) c = {255, 80, 80, 255};
                else if (ft > 16.67f) c = {255, 200, 100, 255};
                else c = {100, 200, 255, 255};
                
                DrawRectangle(
                    static_cast<int>(graphX + i * barW),
                    static_cast<int>(graphY + graphH - barH),
                    std::max(1, static_cast<int>(barW - 1)),
                    static_cast<int>(barH),
                    c
                );
            }
        }

        lineY += graphH + 10;

        // Stats
        float avgTime = 0;
        for (float t : m_frameTimes) avgTime += t;
        if (!m_frameTimes.empty()) avgTime /= m_frameTimes.size();
        
        snprintf(buf, sizeof(buf), "Avg: %.2f ms", avgTime * 1000);
        DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 12, {180, 180, 180, 255});
    }

    void WindowedDebugSystem::drawEngineContent(float x, float y, float w, float h, float scroll) {
        float lineY = y + 5;
        char buf[256];

        // Tabs simulation (just sections for now)
        const char* sections[] = {"EventBus", "Collision", "Registry", "Config"};
        float tabX = x + 5;
        for (const char* section : sections) {
            int tw = MeasureText(section, 11);
            DrawRectangle(static_cast<int>(tabX), static_cast<int>(lineY), tw + 16, 20, {50, 55, 70, 255});
            DrawText(section, static_cast<int>(tabX + 8), static_cast<int>(lineY + 4), 11, 
                    {180, 180, 180, 255});
            tabX += tw + 20;
        }
        lineY += 30;

        // EventBus section
        DrawText("EventBus (Thread-Safe)", static_cast<int>(x + 5), static_cast<int>(lineY), 14, 
                {200, 150, 255, 255});
        lineY += 22;
        
        snprintf(buf, sizeof(buf), "Pending Cross-Thread: %zu", m_pendingCrossThreadEvents);
        DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 12, {180, 180, 180, 255});
        lineY += 18;

        DrawText("• shared_mutex for reads", static_cast<int>(x + 10), static_cast<int>(lineY), 11, 
                {150, 150, 150, 255});
        lineY += 16;
        DrawText("• Cross-thread queue", static_cast<int>(x + 10), static_cast<int>(lineY), 11, 
                {150, 150, 150, 255});
        lineY += 25;

        // Collision section
        DrawText("SpatialHash Collision", static_cast<int>(x + 5), static_cast<int>(lineY), 14, 
                {255, 200, 100, 255});
        lineY += 22;

        snprintf(buf, sizeof(buf), "Entities: %zu  Cells: %zu", m_spatialEntities, m_spatialCells);
        DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 12, {180, 180, 180, 255});
        lineY += 18;

        snprintf(buf, sizeof(buf), "Pairs Checked: %zu", m_collisionPairs);
        DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 12, {180, 180, 180, 255});
        lineY += 25;

        // Registry section
        DrawText("Registry Transactions", static_cast<int>(x + 5), static_cast<int>(lineY), 14, 
                {255, 150, 150, 255});
        lineY += 22;

        snprintf(buf, sizeof(buf), "Deferred Deletions: %zu", m_deferredCount);
        DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 12, 
                m_deferredCount > 0 ? Color{255, 200, 100, 255} : Color{180, 180, 180, 255});
    }

    void WindowedDebugSystem::drawTexturesContent(float x, float y, float w, float h, float scroll) {
        float lineY = y + 5;

        DrawText("Loaded Textures", static_cast<int>(x + 5), static_cast<int>(lineY), 14, 
                {100, 200, 255, 255});
        lineY += 25;

        if (!m_textures || m_textures->empty()) {
            DrawText("No textures loaded", static_cast<int>(x + 10), static_cast<int>(lineY), 12,
                    {150, 150, 150, 255});
            return;
        }

        for (const auto& [name, tex] : *m_textures) {
            char buf[128];
            snprintf(buf, sizeof(buf), "%s (%dx%d)", name.c_str(), tex.width, tex.height);
            DrawText(buf, static_cast<int>(x + 10), static_cast<int>(lineY), 11, {180, 180, 180, 255});
            lineY += 16;

            if (lineY > y + h) break;
        }
    }

    void WindowedDebugSystem::drawArchitectureContent(float x, float y, float w, float h, float scroll) {
        float lineY = y + 5;

        DrawText("ECS Architecture", static_cast<int>(x + 5), static_cast<int>(lineY), 16, 
                {100, 200, 255, 255});
        lineY += 35;

        // Draw simple architecture diagram
        float centerX = x + w / 2;
        
        // Registry box
        DrawRectangle(static_cast<int>(centerX - 60), static_cast<int>(lineY), 120, 40, {60, 60, 100, 255});
        DrawRectangleLines(static_cast<int>(centerX - 60), static_cast<int>(lineY), 120, 40, {100, 150, 255, 255});
        DrawText("Registry", static_cast<int>(centerX - 30), static_cast<int>(lineY + 12), 14, WHITE);
        
        float regBottom = lineY + 40;
        lineY += 60;

        // Entities and Components
        DrawRectangle(static_cast<int>(centerX - 140), static_cast<int>(lineY), 80, 30, {80, 60, 60, 255});
        DrawRectangleLines(static_cast<int>(centerX - 140), static_cast<int>(lineY), 80, 30, {255, 150, 100, 255});
        DrawText("Entities", static_cast<int>(centerX - 125), static_cast<int>(lineY + 8), 12, WHITE);

        DrawRectangle(static_cast<int>(centerX + 60), static_cast<int>(lineY), 90, 30, {60, 80, 60, 255});
        DrawRectangleLines(static_cast<int>(centerX + 60), static_cast<int>(lineY), 90, 30, {100, 255, 100, 255});
        DrawText("Components", static_cast<int>(centerX + 68), static_cast<int>(lineY + 8), 11, WHITE);

        // Lines
        DrawLine(static_cast<int>(centerX - 100), static_cast<int>(regBottom), 
                static_cast<int>(centerX - 100), static_cast<int>(lineY), {150, 150, 150, 255});
        DrawLine(static_cast<int>(centerX + 105), static_cast<int>(regBottom),
                static_cast<int>(centerX + 105), static_cast<int>(lineY), {150, 150, 150, 255});

        lineY += 50;

        // Systems
        DrawRectangle(static_cast<int>(centerX - 50), static_cast<int>(lineY), 100, 30, {60, 60, 80, 255});
        DrawRectangleLines(static_cast<int>(centerX - 50), static_cast<int>(lineY), 100, 30, {150, 150, 255, 255});
        DrawText("Systems", static_cast<int>(centerX - 28), static_cast<int>(lineY + 8), 12, WHITE);

        // EventBus
        DrawRectangle(static_cast<int>(centerX + 80), static_cast<int>(lineY), 80, 30, {80, 60, 80, 255});
        DrawRectangleLines(static_cast<int>(centerX + 80), static_cast<int>(lineY), 80, 30, {200, 150, 200, 255});
        DrawText("EventBus", static_cast<int>(centerX + 90), static_cast<int>(lineY + 8), 11, WHITE);
    }

    void WindowedDebugSystem::drawConsoleContent(float x, float y, float w, float h, float scroll) {
        float lineY = y + 5;

        DrawText("Console Output", static_cast<int>(x + 5), static_cast<int>(lineY), 14, 
                {100, 200, 255, 255});
        lineY += 25;

        // Simulated console messages
        const char* messages[] = {
            "[INFO] Game initialized successfully",
            "[INFO] Loaded 12 textures",
            "[INFO] Network: Listening on port 4242",
            "[DEBUG] ECS Registry ready",
            "[INFO] Player spawned at (100, 360)"
        };

        for (const char* msg : messages) {
            Color msgColor = {180, 180, 180, 255};
            if (msg[1] == 'D') msgColor = {150, 150, 200, 255};  // DEBUG
            if (msg[1] == 'W') msgColor = {255, 200, 100, 255};  // WARN
            if (msg[1] == 'E') msgColor = {255, 100, 100, 255};  // ERROR
            
            DrawText(msg, static_cast<int>(x + 10), static_cast<int>(lineY), 11, msgColor);
            lineY += 16;
        }
    }

    void WindowedDebugSystem::switchToClassic() {
        if (m_classicDebugSystem) {
            m_enabled = false;
            m_classicDebugSystem->setEnabled(true);
        }
    }

} // namespace rtype::ecs
