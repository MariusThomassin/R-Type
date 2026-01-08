/*
** R-Type ECS - DebugSystem Implementation
** Debug overlay with event-based input
** Click tabs to switch, scroll and click within tabs
*/

#include "DebugSystem.hpp"
#include "WindowedDebugSystem.hpp"

namespace rtype::ecs {

    DebugSystem::DebugSystem(EventBus& eventBus, int screenWidth, int screenHeight)
        : m_eventBus(eventBus), m_screenWidth(screenWidth), m_screenHeight(screenHeight) {
        
        m_tabs.push_back(std::make_unique<debug::StatsTab>());
        m_tabs.push_back(std::make_unique<debug::ArchitectureTab>());
        m_tabs.push_back(std::make_unique<debug::EntityInspectorTab>());
        m_tabs.push_back(std::make_unique<debug::EntitySpawnerTab>());
        m_tabs.push_back(std::make_unique<debug::BulletsTab>());
        m_tabs.push_back(std::make_unique<debug::TexturesTab>());
        m_tabs.push_back(std::make_unique<debug::PerformanceTab>());
        
        // EngineTab for new engine subsystems
        auto engineTab = std::make_unique<debug::EngineTab>(&eventBus);
        m_engineTab = engineTab.get();
        m_tabs.push_back(std::move(engineTab));
        
        m_tabs.push_back(std::make_unique<debug::UILibraryTab>());
        
        // ModesTab needs EventBus reference
        m_modesTab = std::make_unique<debug::ModesTab>(eventBus);
        m_tabs.push_back(std::move(m_modesTab));
        m_modesTabIndex = m_tabs.size() - 1;

        subscribeToEvents();
    }

    DebugSystem::~DebugSystem() {
        unsubscribeFromEvents();
    }

    void DebugSystem::init() {
        for (auto& tab : m_tabs) {
            tab->setRegistry(m_registry);
            tab->setScreenSize(m_screenWidth, m_screenHeight);
        }
    }

    void DebugSystem::setTextures(const std::unordered_map<std::string, Texture2D>* textures) {
        for (auto& tab : m_tabs) {
            tab->setTextures(textures);
        }
    }

    void DebugSystem::update(float dt) {
        if (!m_registry) return;

        // Process toggle
        if (m_pendingToggle) {
            m_enabled = !m_enabled;
            m_pendingToggle = false;
        }
        
        if (!m_enabled) return;

        // Build mouse input state from events
        debug::MouseInput mouse;
        mouse.x = m_mouseX;
        mouse.y = m_mouseY;
        mouse.wheelDelta = m_mouseWheelDelta;
        mouse.leftPressed = m_mouseLeftPressed;
        mouse.leftDown = m_mouseLeftDown;
        mouse.rightPressed = m_mouseRightPressed;
        mouse.rightDown = m_mouseRightDown;

        // Handle tab bar clicks
        handleTabBarClick(mouse);

        // Handle close button
        handleCloseButton(mouse);

        // Update current tab with mouse state
        if (m_currentTab < m_tabs.size()) {
            m_tabs[m_currentTab]->setMouseState(mouse);
            m_tabs[m_currentTab]->update(dt);
        }

        // Clear single-frame events
        m_mouseLeftPressed = false;
        m_mouseRightPressed = false;
        m_mouseWheelDelta = 0;
    }

    void DebugSystem::draw() {
        if (!m_enabled) return;

        // Background overlay
        DrawRectangle(0, 0, m_screenWidth, m_screenHeight, {0, 0, 0, 200});

        // Header
        DrawRectangle(0, 0, m_screenWidth, 50, {30, 30, 50, 255});
        DrawText("DEBUG MODE", 20, 15, 24, {255, 255, 100, 255});

        // "Windowed Mode" button 
        const char* windowedLabel = "Windowed Mode";
        int windowedW = MeasureText(windowedLabel, 12) + 16;
        int windowedX = m_screenWidth - 150;
        bool windowedHover = m_mouseX >= windowedX && m_mouseX < windowedX + windowedW &&
                            m_mouseY >= 12 && m_mouseY < 12 + 26;
        DrawRectangle(windowedX, 12, windowedW, 26, windowedHover ? Color{60, 100, 150, 255} : Color{50, 70, 100, 255});
        DrawRectangleLines(windowedX, 12, windowedW, 26, {80, 120, 170, 255});
        DrawText(windowedLabel, windowedX + 8, 18, 12, windowedHover ? WHITE : Color{200, 200, 200, 255});
        
        if (windowedHover && m_mouseLeftPressed) {
            switchToWindowed();
        }

        // Close button
        int closeX = m_screenWidth - 45;
        bool closeHover = m_mouseX >= closeX && m_mouseX < closeX + 35 &&
                          m_mouseY >= 10 && m_mouseY < 40;
        DrawRectangle(closeX, 10, 35, 30, closeHover ? Color{150, 60, 60, 255} : Color{100, 40, 40, 255});
        DrawText("X", closeX + 12, 16, 18, WHITE);

        // Tab bar
        drawTabBar();

        // Current tab content
        if (m_currentTab < m_tabs.size()) {
            m_tabs[m_currentTab]->draw(95);
        }

        DrawText("[O] Toggle Debug Mode", 10, m_screenHeight - 25, 12, {80, 80, 80, 255});
    }

    void DebugSystem::updateShowoffState(bool active, const std::string& patternName, 
                            int currentPhase, int totalPhases, float progress) {
        if (m_modesTabIndex < m_tabs.size()) {
            auto* modesTab = dynamic_cast<debug::ModesTab*>(m_tabs[m_modesTabIndex].get());
            if (modesTab) {
                modesTab->setShowoffState(active, patternName, currentPhase, totalPhases, progress);
            }
        }
    }

    void DebugSystem::updateStressTestState(bool active, bool complete, int intensity, 
                                 float progress, const std::string& reportFilename) {
        if (m_modesTabIndex < m_tabs.size()) {
            auto* modesTab = dynamic_cast<debug::ModesTab*>(m_tabs[m_modesTabIndex].get());
            if (modesTab) {
                modesTab->setStressTestState(active, complete, intensity, progress, reportFilename);
            }
        }
    }

    void DebugSystem::updateEngineStats(size_t spatialEntities, size_t spatialCells,
                                        size_t collisionPairs, size_t deferredCount) {
        if (m_engineTab) {
            m_engineTab->setSpatialHashStats(spatialEntities, spatialCells, collisionPairs);
            m_engineTab->setDeferredCount(deferredCount);
        }
    }

    void DebugSystem::subscribeToEvents() {
        m_debugToggleSubId = m_eventBus.subscribe<events::DebugToggleEvent>(
            [this](const events::DebugToggleEvent&) {
                m_pendingToggle = true;
            }
        );

        m_mouseMoveSub = m_eventBus.subscribe<events::MouseMoveEvent>(
            [this](const events::MouseMoveEvent& e) {
                m_mouseX = static_cast<int>(e.x);
                m_mouseY = static_cast<int>(e.y);
            }
        );

        m_mouseButtonPressedSub = m_eventBus.subscribe<events::MouseButtonPressedEvent>(
            [this](const events::MouseButtonPressedEvent& e) {
                if (e.button == events::MouseButton::Left) {
                    m_mouseLeftPressed = true;
                    m_mouseLeftDown = true;
                } else if (e.button == events::MouseButton::Right) {
                    m_mouseRightPressed = true;
                    m_mouseRightDown = true;
                }
            }
        );

        m_mouseButtonReleasedSub = m_eventBus.subscribe<events::MouseButtonReleasedEvent>(
            [this](const events::MouseButtonReleasedEvent& e) {
                if (e.button == events::MouseButton::Left) {
                    m_mouseLeftDown = false;
                } else if (e.button == events::MouseButton::Right) {
                    m_mouseRightDown = false;
                }
            }
        );

        m_mouseWheelSub = m_eventBus.subscribe<events::MouseWheelEvent>(
            [this](const events::MouseWheelEvent& e) {
                m_mouseWheelDelta = e.delta;
            }
        );
    }

    void DebugSystem::unsubscribeFromEvents() {
        m_eventBus.unsubscribe<events::DebugToggleEvent>(m_debugToggleSubId);
        m_eventBus.unsubscribe<events::MouseMoveEvent>(m_mouseMoveSub);
        m_eventBus.unsubscribe<events::MouseButtonPressedEvent>(m_mouseButtonPressedSub);
        m_eventBus.unsubscribe<events::MouseButtonReleasedEvent>(m_mouseButtonReleasedSub);
        m_eventBus.unsubscribe<events::MouseWheelEvent>(m_mouseWheelSub);
    }

    void DebugSystem::handleTabBarClick(const debug::MouseInput& mouse) {
        if (!mouse.leftPressed || mouse.y < 52 || mouse.y > 85) return;

        int x = 20;
        for (size_t i = 0; i < m_tabs.size(); i++) {
            int width = MeasureText(m_tabs[i]->getName(), 14) + 24;
            if (mouse.x >= x && mouse.x < x + width) {
                m_currentTab = i;
                return;
            }
            x += width + 4;
        }
    }

    void DebugSystem::handleCloseButton(const debug::MouseInput& mouse) {
        int closeX = m_screenWidth - 45;
        if (mouse.leftPressed && 
            mouse.x >= closeX && mouse.x < closeX + 35 &&
            mouse.y >= 10 && mouse.y < 40) {
            m_enabled = false;
        }
    }

    void DebugSystem::drawTabBar() {
        int x = 20;

        for (size_t i = 0; i < m_tabs.size(); i++) {
            bool active = (i == m_currentTab);
            const char* name = m_tabs[i]->getName();
            int width = MeasureText(name, 14) + 24;
            
            bool hover = m_mouseX >= x && m_mouseX < x + width &&
                        m_mouseY >= 55 && m_mouseY < 85;
            
            Color bg = active ? Color{60, 70, 110, 255} : 
                      (hover ? Color{50, 55, 80, 255} : Color{40, 40, 60, 255});
            Color border = active ? Color{100, 150, 255, 255} : Color{60, 60, 80, 255};
            Color text = active ? Color{255, 255, 100, 255} : 
                        (hover ? WHITE : Color{150, 150, 150, 255});

            DrawRectangle(x, 55, width, 30, bg);
            if (active) {
                DrawRectangle(x, 82, width, 3, {100, 150, 255, 255});
            }
            DrawRectangleLines(x, 55, width, 30, border);
            DrawText(name, x + 12, 63, 14, text);
            
            x += width + 4;
        }
    }

    void DebugSystem::switchToWindowed() {
        if (m_windowedDebugSystem) {
            m_enabled = false;
            m_windowedDebugSystem->setEnabled(true);
        }
    }

} // namespace rtype::ecs