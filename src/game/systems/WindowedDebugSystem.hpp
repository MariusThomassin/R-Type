/*
** R-Type ECS - WindowedDebugSystem
** Engine-style debug UI with draggable windows
** Inspired by professional game engine interfaces (Ubisoft, Unity, Unreal)
*/

#pragma once

#include "../../engine/ecs/core/ISystem.hpp"
#include "../../engine/ecs/core/Registry.hpp"
#include "../../engine/ecs/core/EventBus.hpp"
#include "../../engine/ui/WindowManager.hpp"
#include "../../engine/ui/widgets/WindowWidget.hpp"
#include "engine/ecs/events/InputEvents.hpp"

#include <raylib.h>
#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace rtype::ecs {

    // Forward declaration for classic debug system
    class DebugSystem;

    /**
     * @brief Content drawer function type
     * Draws content inside a window, receives (x, y, width, height) of content area
     */
    using WindowContentDrawer = std::function<void(float, float, float, float, float)>;

    /**
     * @brief Debug window configuration
     */
    struct DebugWindowConfig {
        std::string id;
        std::string title;
        float defaultX = 100;
        float defaultY = 100;
        float defaultWidth = 350;
        float defaultHeight = 400;
        ui::WindowFlags flags = ui::WindowFlags::None;
        WindowContentDrawer drawer;
        bool visibleByDefault = false;
    };

    /**
     * @brief Modern windowed debug system with engine-style UI
     * 
     * Features:
     * - Multiple draggable, resizable debug windows
     * - Organized by category (Stats, Entities, Performance, Engine, etc.)
     * - Top menu bar for quick window access
     * - Persistent window positions
     * - Professional dark theme
     */
    class WindowedDebugSystem : public ISystem {
    public:
        /**
         * @brief Construct the debug system
         * @param eventBus Reference to EventBus
         * @param screenWidth Screen width
         * @param screenHeight Screen height
         */
        WindowedDebugSystem(EventBus& eventBus, int screenWidth = 1280, int screenHeight = 720);

        ~WindowedDebugSystem() override;

        /**
         * @brief Initialize the debug system and create windows
         */
        void init();

        /**
         * @brief Update the debug system
         * @param dt Delta time
         */
        void update(float dt) override;

        /**
         * @brief Draw the debug overlay
         */
        void draw();

        /**
         * @brief Check if debug mode is enabled
         */
        bool isEnabled() const { return m_enabled; }

        /**
         * @brief Set the enabled state
         * @param enabled New enabled state
         */
        void setEnabled(bool enabled) { m_enabled = enabled; }

        /**
         * @brief Get the system phase
         */
        SystemPhase getPhase() const override { return SystemPhase::Input; }

        /**
         * @brief Set textures map for texture viewer
         */
        void setTextures(const std::unordered_map<std::string, Texture2D>* textures);

        /**
         * @brief Register a custom debug window
         */
        void registerWindow(const DebugWindowConfig& config);

        /**
         * @brief Get the window manager
         */
        ui::WindowManager& getWindowManager() { return m_windowManager; }

        /**
         * @brief Toggle a specific window
         */
        void toggleWindow(const std::string& id);

        /**
         * @brief Show a specific window
         */
        void showWindow(const std::string& id);

        // === Stats for windows to display ===
        void setSpatialHashStats(size_t entities, size_t cells, size_t pairs);
        void setDeferredCount(size_t count);

        // === Showoff/StressTest state (for compatibility with old interface) ===
        void updateShowoffState(bool active, const std::string& patternName = "", 
                               int currentPhase = 0, int totalPhases = 0, float progress = 0.0f);
        void updateStressTestState(bool active, bool complete, int intensity, 
                                   float progress, const std::string& reportFilename);

        // === Classic debug system toggle ===
        void setClassicDebugSystem(DebugSystem* classic) { m_classicDebugSystem = classic; }
        void switchToClassic();

    private:
        EventBus& m_eventBus;
        ui::WindowManager m_windowManager;
        
        // Reference to classic debug system for switching
        DebugSystem* m_classicDebugSystem = nullptr;
        
        int m_screenWidth;
        int m_screenHeight;
        bool m_enabled = false;
        bool m_pendingToggle = false;
        bool m_menuBarHovered = false;
        float m_animTime = 0.0f;
        
        // Textures reference
        const std::unordered_map<std::string, Texture2D>* m_textures = nullptr;
        
        // Window drawers
        std::unordered_map<std::string, WindowContentDrawer> m_windowDrawers;
        
        // Stats
        size_t m_spatialEntities = 0;
        size_t m_spatialCells = 0;
        size_t m_collisionPairs = 0;
        size_t m_deferredCount = 0;
        size_t m_pendingCrossThreadEvents = 0;

        // Menu bar state
        struct MenuCategory {
            std::string name;
            std::vector<std::string> windowIds;
            bool open = false;
        };
        std::vector<MenuCategory> m_menuCategories;
        int m_activeMenu = -1;

        // Event subscriptions
        EventBus::SubscriberId m_debugToggleSub;
        EventBus::SubscriberId m_mouseMoveSub;
        EventBus::SubscriberId m_mouseClickSub;
        float m_mouseX = 0, m_mouseY = 0;
        bool m_mousePressed = false;

        void subscribeToEvents();
        void unsubscribeFromEvents();
        void createDefaultWindows();
        void drawMenuBar();
        void drawMenuDropdown(int menuIndex, float x, float y);

        // Window content drawers
        void drawStatsContent(float x, float y, float w, float h, float scroll);
        void drawEntitiesContent(float x, float y, float w, float h, float scroll);
        void drawPerformanceContent(float x, float y, float w, float h, float scroll);
        void drawEngineContent(float x, float y, float w, float h, float scroll);
        void drawTexturesContent(float x, float y, float w, float h, float scroll);
        void drawArchitectureContent(float x, float y, float w, float h, float scroll);
        void drawConsoleContent(float x, float y, float w, float h, float scroll);
        
        // Performance tracking
        std::vector<float> m_frameTimes;
        float m_lastFrameTime = 0;

        // Showoff mode state
        bool m_showoffActive = false;
        std::string m_showoffPatternName;
        int m_showoffCurrentPhase = 0;
        int m_showoffTotalPhases = 0;
        float m_showoffProgress = 0.0f;

        // Stress test state
        bool m_stressTestActive = false;
        bool m_stressTestComplete = false;
        int m_stressTestIntensity = 0;
        float m_stressTestProgress = 0.0f;
        std::string m_stressTestReportFilename;
    };

} // namespace rtype::ecs
