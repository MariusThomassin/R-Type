/*
** R-Type ECS - DebugSystem
** Debug overlay with event-based input
** Click tabs to switch, scroll and click within tabs
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/InputEvents.hpp"
#include "debug/DebugTab.hpp"
#include "debug/StatsTab.hpp"
#include "debug/TexturesTab.hpp"
#include "debug/ArchitectureTab.hpp"
#include "debug/EntityInspectorTab.hpp"
#include "debug/EntitySpawnerTab.hpp"
#include "debug/BulletsTab.hpp"
#include "debug/PerformanceTab.hpp"
#include "debug/ModesTab.hpp"

#include <raylib.h>
#include <vector>
#include <memory>

namespace rtype::ecs {

    /**
     * @brief Debug overlay system with event-based input
     * 
     * Features:
     * - Click tabs to switch views
     * - Mouse scroll and click within tabs
     * - All input comes through EventBus
     * - Press 'O' to toggle debug mode
     */
    class DebugSystem : public ISystem {
    public:
        DebugSystem(EventBus& eventBus, int screenWidth = 1280, int screenHeight = 720)
            : m_eventBus(eventBus), m_screenWidth(screenWidth), m_screenHeight(screenHeight) {
            
            m_tabs.push_back(std::make_unique<debug::StatsTab>());
            m_tabs.push_back(std::make_unique<debug::ArchitectureTab>());
            m_tabs.push_back(std::make_unique<debug::EntityInspectorTab>());
            m_tabs.push_back(std::make_unique<debug::EntitySpawnerTab>());
            m_tabs.push_back(std::make_unique<debug::BulletsTab>());
            m_tabs.push_back(std::make_unique<debug::TexturesTab>());
            m_tabs.push_back(std::make_unique<debug::PerformanceTab>());
            
            // ModesTab needs EventBus reference
            m_modesTab = std::make_unique<debug::ModesTab>(eventBus);
            m_tabs.push_back(std::move(m_modesTab));
            m_modesTabIndex = m_tabs.size() - 1;

            subscribeToEvents();
        }

        ~DebugSystem() override {
            unsubscribeFromEvents();
        }

        void init() {
            for (auto& tab : m_tabs) {
                tab->setRegistry(m_registry);
                tab->setScreenSize(m_screenWidth, m_screenHeight);
            }
        }

        void setTextures(const std::unordered_map<std::string, Texture2D>* textures) {
            for (auto& tab : m_tabs) {
                tab->setTextures(textures);
            }
        }

        void update(float dt) override {
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

        void draw() {
            if (!m_enabled) return;

            // Background overlay
            DrawRectangle(0, 0, m_screenWidth, m_screenHeight, {0, 0, 0, 200});

            // Header
            DrawRectangle(0, 0, m_screenWidth, 50, {30, 30, 50, 255});
            DrawText("DEBUG MODE", 20, 15, 24, {255, 255, 100, 255});

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

        bool isEnabled() const { return m_enabled; }
        SystemPhase getPhase() const override { return SystemPhase::Input; }

        // Mode state synchronization
        void updateShowoffState(bool active, const std::string& patternName = "", 
                               int currentPhase = 0, int totalPhases = 0, float progress = 0.0f) {
            if (m_modesTabIndex < m_tabs.size()) {
                auto* modesTab = dynamic_cast<debug::ModesTab*>(m_tabs[m_modesTabIndex].get());
                if (modesTab) {
                    modesTab->setShowoffState(active, patternName, currentPhase, totalPhases, progress);
                }
            }
        }

        void updateStressTestState(bool active, bool complete, int intensity = 0, 
                                   float progress = 0.0f, const std::string& reportFile = "") {
            if (m_modesTabIndex < m_tabs.size()) {
                auto* modesTab = dynamic_cast<debug::ModesTab*>(m_tabs[m_modesTabIndex].get());
                if (modesTab) {
                    modesTab->setStressTestState(active, complete, intensity, progress, reportFile);
                }
            }
        }

    private:
        EventBus& m_eventBus;
        int m_screenWidth, m_screenHeight;
        bool m_enabled = false;
        size_t m_currentTab = 0;
        std::vector<std::unique_ptr<debug::IDebugTab>> m_tabs;
        std::unique_ptr<debug::ModesTab> m_modesTab;  // Keep reference for state updates
        size_t m_modesTabIndex = 0;

        // Mouse state from events
        float m_mouseX = 0, m_mouseY = 0;
        float m_mouseWheelDelta = 0;
        bool m_mouseLeftPressed = false;
        bool m_mouseLeftDown = false;
        bool m_mouseRightPressed = false;
        bool m_mouseRightDown = false;
        bool m_pendingToggle = false;

        // Event subscriptions
        EventBus::SubscriberId m_keyPressedSub = 0;
        EventBus::SubscriberId m_mouseMoveSub = 0;
        EventBus::SubscriberId m_mouseButtonSub = 0;
        EventBus::SubscriberId m_mouseButtonReleaseSub = 0;
        EventBus::SubscriberId m_mouseWheelSub = 0;

        void subscribeToEvents() {
            m_keyPressedSub = m_eventBus.subscribe<events::KeyPressedEvent>(
                [this](const events::KeyPressedEvent& e) {
                    if (e.key == events::KeyCode::O) {
                        m_pendingToggle = true;
                    }
                }
            );

            m_mouseMoveSub = m_eventBus.subscribe<events::MouseMoveEvent>(
                [this](const events::MouseMoveEvent& e) {
                    m_mouseX = e.x;
                    m_mouseY = e.y;
                }
            );

            m_mouseButtonSub = m_eventBus.subscribe<events::MouseButtonPressedEvent>(
                [this](const events::MouseButtonPressedEvent& e) {
                    m_mouseX = e.x;
                    m_mouseY = e.y;
                    if (e.button == events::MouseButton::Left) {
                        m_mouseLeftPressed = true;
                        m_mouseLeftDown = true;
                    } else if (e.button == events::MouseButton::Right) {
                        m_mouseRightPressed = true;
                        m_mouseRightDown = true;
                    }
                }
            );

            m_mouseButtonReleaseSub = m_eventBus.subscribe<events::MouseButtonReleasedEvent>(
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

        void unsubscribeFromEvents() {
            m_eventBus.unsubscribe<events::KeyPressedEvent>(m_keyPressedSub);
            m_eventBus.unsubscribe<events::MouseMoveEvent>(m_mouseMoveSub);
            m_eventBus.unsubscribe<events::MouseButtonPressedEvent>(m_mouseButtonSub);
            m_eventBus.unsubscribe<events::MouseButtonReleasedEvent>(m_mouseButtonReleaseSub);
            m_eventBus.unsubscribe<events::MouseWheelEvent>(m_mouseWheelSub);
        }

        void handleTabBarClick(const debug::MouseInput& mouse) {
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

        void handleCloseButton(const debug::MouseInput& mouse) {
            int closeX = m_screenWidth - 45;
            if (mouse.leftPressed && 
                mouse.x >= closeX && mouse.x < closeX + 35 &&
                mouse.y >= 10 && mouse.y < 40) {
                m_enabled = false;
            }
        }

        void drawTabBar() {
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
    };

} // namespace rtype::ecs
