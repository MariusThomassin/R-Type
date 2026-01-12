/*
** R-Type ECS - DebugSystem
** Debug overlay with event-based input
** Click tabs to switch, scroll and click within tabs
*/

#pragma once

#include "../../engine/ecs/core/ISystem.hpp"
#include "../../engine/ecs/core/Registry.hpp"
#include "../../engine/ecs/core/EventBus.hpp"
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
#include "debug/UILibraryTab.hpp"
#include "debug/EngineTab.hpp"
#include "debug/FeaturesTestTab.hpp"

#include <raylib.h>
#include <vector>
#include <memory>

namespace rtype::ecs {

    // Forward declaration for windowed debug system
    class WindowedDebugSystem;

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
            /**
             * @brief Construct a new Debug System object
             * @param eventBus Reference to EventBus for input handling
             * @param screenWidth Width of the game screen
             * @param screenHeight Height of the game screen
             */
            DebugSystem(EventBus& eventBus, int screenWidth = 1280, int screenHeight = 720);

            /**
             * @brief Destroy the Debug System object
             */
            ~DebugSystem() override;

            /**
             * @brief Initialize tabs with registry and screen size
             */
            void init();

            /**
             * @brief Set textures map for tabs that need it
             * @param textures Pointer to textures map
             */
            void setTextures(const std::unordered_map<std::string, Texture2D>* textures);

            /**
             * @brief Update the debug system and current tab
             * @param dt Delta time since last update
             */
            void update(float dt) override;

            /**
             * @brief Draw the debug overlay and current tab
             */
            void draw();

            /**
             * @brief Check if the debug system is enabled
             * @return true if enabled
             */
            bool isEnabled() const { return m_enabled; }

            /**
             * @brief Set the enabled state
             * @param enabled New enabled state
             */
            void setEnabled(bool enabled) { m_enabled = enabled; }

            /**
             * @brief Get the execution phase of this system
             * @return The phase determining update order
             */
            SystemPhase getPhase() const override { return SystemPhase::Input; }

            /**
             * @brief Update the showoff mode state in ModesTab
             */
            void updateShowoffState(bool active, const std::string& patternName = "", int currentPhase = 0, int totalPhases = 0, float progress = 0.0f);

            /**
             * @brief Update the stress test state in ModesTab
             */
            void updateStressTestState(bool active, bool complete, int intensity, float progress, const std::string& reportFilename);

            /**
             * @brief Update engine subsystem stats in EngineTab
             * @param spatialEntities Number of entities in spatial hash
             * @param spatialCells Number of active cells
             * @param collisionPairs Number of collision pairs checked
             * @param deferredCount Number of deferred entity deletions
             */
            void updateEngineStats(size_t spatialEntities, size_t spatialCells, 
                                   size_t collisionPairs, size_t deferredCount);

            // === Windowed debug system toggle ===
            void setWindowedDebugSystem(WindowedDebugSystem* windowed) { m_windowedDebugSystem = windowed; }
            void switchToWindowed();

        private:
            /**
             * @brief Subscribe to input events
             */
            void subscribeToEvents();

            /**
             * @brief Unsubscribe from input events
             */
            void unsubscribeFromEvents();

            /**
             * @brief Handle tab bar mouse clicks
             * @param mouse Current mouse input state
             */
            void handleTabBarClick(const debug::MouseInput& mouse);

            /**
             * @brief Handle close button clicks
             * @param mouse Current mouse input state
             */
            void handleCloseButton(const debug::MouseInput& mouse);

            /**
             * @brief Draw the tab bar
             */
            void drawTabBar();

            /**
             * @brief Reference to EventBus for communication
             */
            EventBus& m_eventBus;

            /**
             * @brief Collection of debug tabs
             */
            std::vector<std::unique_ptr<debug::IDebugTab>> m_tabs;

            /**
             * @brief Special reference to ModesTab for state updates
             */
            std::unique_ptr<debug::ModesTab> m_modesTab;

            /**
             * @brief Pointer to EngineTab for updating engine stats
             */
            debug::EngineTab* m_engineTab = nullptr;

            /**
             * @brief Index of ModesTab in m_tabs vector
             */
            size_t m_modesTabIndex = 0;

            /**
             * @brief Currently active tab index
             */
            size_t m_currentTab = 0;

            /**
             * @brief Screen dimensions
             */
            int m_screenWidth, m_screenHeight;

            /**
             * @brief Debug overlay visibility state
             */
            bool m_enabled = false;

            /**
             * @brief Pending toggle state (from event)
             */
            bool m_pendingToggle = false;

            /**
             * @brief Current mouse position and state
             */
            int m_mouseX = 0, m_mouseY = 0;
            /**
             * @brief Mouse button and wheel states
             */
            int m_mouseWheelDelta = 0;
            /**
             * @brief Mouse button pressed/down states
             */
            bool m_mouseLeftPressed = false, m_mouseLeftDown = false;
            /**
             * @brief Mouse button pressed/down states
             */
            bool m_mouseRightPressed = false, m_mouseRightDown = false;

            /**
             * @brief Event subscription IDs
             */
            EventBus::SubscriberId m_debugToggleSubId;
            /**
             * @brief Mouse move event subscription ID
             */
            EventBus::SubscriberId m_mouseMoveSub;
            /**
             * @brief Mouse button pressed event subscription ID
             */
            EventBus::SubscriberId m_mouseButtonPressedSub;
            /**
             * @brief Mouse button released event subscription ID
             */
            EventBus::SubscriberId m_mouseButtonReleasedSub;
            /**
             * @brief Mouse wheel event subscription ID
             */
            EventBus::SubscriberId m_mouseWheelSub;
            
            /**
             * @brief Reference to windowed debug system for switching
             */
            WindowedDebugSystem* m_windowedDebugSystem = nullptr;
        };
} // namespace rtype::ecs
