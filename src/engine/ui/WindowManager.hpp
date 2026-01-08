/*
** R-Type Engine - WindowManager
** Manages multiple windows with focus, z-ordering, and docking
*/

#ifndef WINDOWMANAGER_HPP_
#define WINDOWMANAGER_HPP_

#include "widgets/WindowWidget.hpp"
#include "../ecs/core/EventBus.hpp"
#include "../ecs/events/InputEvents.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace rtype::ui {

    /**
     * @brief Layout patterns for auto-arranging windows
     */
    enum class LayoutPattern {
        Cascade,        // Windows offset from each other diagonally
        TileHorizontal, // Side by side
        TileVertical,   // Stacked top to bottom
        Grid,           // N x M grid
        Custom
    };

    /**
     * @brief Manages multiple WindowWidgets with proper z-ordering and focus
     * 
     * Features:
     * - Window creation and destruction
     * - Focus management (click-to-focus)
     * - Z-ordering (focused window on top)
     * - Always-on-top layer support
     * - Modal window support
     * - Drag and resize handling
     * - Cursor management
     * - Keyboard navigation
     * - EventBus integration for input
     * - Window state persistence
     */
    class WindowManager {
    public:
        /**
         * @brief Construct WindowManager with EventBus integration
         * @param eventBus Reference to the ECS EventBus
         */
        explicit WindowManager(rtype::ecs::EventBus& eventBus);
        
        /**
         * @brief Destructor unsubscribes from events
         */
        ~WindowManager();

        // === Window Management ===

        /**
         * @brief Create a new window
         * @param id Unique identifier for the window
         * @param title Window title
         * @param x Initial X position
         * @param y Initial Y position
         * @param width Initial width
         * @param height Initial height
         * @param flags Window behavior flags
         * @return Shared pointer to the created window
         */
        std::shared_ptr<WindowWidget> createWindow(
            const std::string& id,
            const std::string& title,
            float x, float y,
            float width, float height,
            WindowFlags flags = WindowFlags::None
        );

        /**
         * @brief Get a window by its ID
         * @param id Window identifier
         * @return Shared pointer to the window or nullptr
         */
        std::shared_ptr<WindowWidget> getWindow(const std::string& id);

        /**
         * @brief Remove a window by ID
         * @param id Window identifier
         */
        void removeWindow(const std::string& id);

        /**
         * @brief Check if a window exists
         * @param id Window identifier
         * @return true if window exists
         */
        bool hasWindow(const std::string& id) const;

        /**
         * @brief Get all windows in z-order (back to front)
         * @return Vector of window pointers
         */
        const std::vector<std::shared_ptr<WindowWidget>>& getWindows() const;

        /**
         * @brief Get all window IDs
         * @return Vector of window IDs
         */
        std::vector<std::string> getWindowIds() const;

        /**
         * @brief Get window ID from window pointer
         * @param window The window to get ID for
         * @return Window ID or empty string
         */
        std::string getWindowId(const std::shared_ptr<WindowWidget>& window) const;

        // === Focus Management ===

        /**
         * @brief Bring a window to front and give it focus
         * @param id Window identifier
         */
        void focusWindow(const std::string& id);

        /**
         * @brief Get the currently focused window
         * @return Shared pointer to focused window or nullptr
         */
        std::shared_ptr<WindowWidget> getFocusedWindow() const;

        /**
         * @brief Get the ID of the focused window
         * @return Window ID or empty string
         */
        std::string getFocusedWindowId() const;

        /**
         * @brief Focus next window in z-order
         */
        void focusNextWindow();

        /**
         * @brief Focus previous window in z-order
         */
        void focusPrevWindow();

        // === Modal Windows ===

        /**
         * @brief Show a window as modal (blocks input to other windows)
         * @param id Window identifier to show as modal
         * @param blockerId Optional ID of window that triggered the modal
         */
        void showModal(const std::string& id, const std::string& blockerId = "");

        /**
         * @brief Close the current modal window
         */
        void closeModal();

        /**
         * @brief Get the current modal window
         * @return Shared pointer to modal window or nullptr
         */
        std::shared_ptr<WindowWidget> getModalWindow() const;

        /**
         * @brief Check if there is an active modal window
         * @return true if a modal is active
         */
        bool hasActiveModal() const;

        /**
         * @brief Check if input is blocked by modal
         * @return true if modal is blocking input
         */
        bool isInputBlocked() const;

        // === Visibility ===

        /**
         * @brief Show a window
         * @param id Window identifier
         */
        void showWindow(const std::string& id);

        /**
         * @brief Hide a window
         * @param id Window identifier
         */
        void hideWindow(const std::string& id);

        /**
         * @brief Toggle window visibility
         * @param id Window identifier
         */
        void toggleWindow(const std::string& id);

        /**
         * @brief Check if a window is visible
         * @param id Window identifier
         * @return true if visible
         */
        bool isWindowVisible(const std::string& id) const;

        // === Update and Render ===

        /**
         * @brief Update all windows
         * @param dt Delta time
         */
        void update(float dt);

        /**
         * @brief Render all visible windows in z-order
         */
        void render() const;

        // === Cursor Management ===

        /**
         * @brief Get the suggested cursor type based on current window hover states
         * @return Suggested cursor type
         */
        CursorType getSuggestedCursor() const;

        /**
         * @brief Update the system cursor based on window states
         */
        void updateCursor();

        // === Layout ===

        /**
         * @brief Arrange all visible windows using a layout pattern
         * @param pattern The layout pattern to use
         * @param padding Padding between windows
         */
        void layoutWindows(LayoutPattern pattern, float padding = 10.0f);

        /**
         * @brief Arrange specific windows using a layout pattern
         * @param windowIds IDs of windows to arrange
         * @param pattern The layout pattern to use
         * @param padding Padding between windows
         */
        void arrangeWindows(const std::vector<std::string>& windowIds, 
                           LayoutPattern pattern, float padding = 10.0f);

        // === Input Queries ===

        /**
         * @brief Check if mouse is over any window
         * @return true if mouse is captured by a window
         */
        bool isCapturingMouse() const;

        /**
         * @brief Check if a window has keyboard focus
         * @return true if keyboard input is captured
         */
        bool isCapturingKeyboard() const;

        /**
         * @brief Set whether WindowManager should handle all input
         * @param enabled true to enable input handling
         */
        void setEnabled(bool enabled);

        /**
         * @brief Check if WindowManager is enabled
         * @return true if enabled
         */
        bool isEnabled() const;

        /**
         * @brief Set the screen dimensions for layout calculations
         * @param width Screen width
         * @param height Screen height
         */
        void setScreenSize(int width, int height);

    private:
        rtype::ecs::EventBus& _eventBus;
        
        // Windows in z-order (back to front)
        std::vector<std::shared_ptr<WindowWidget>> _normalLayer;      // Normal windows
        std::vector<std::shared_ptr<WindowWidget>> _alwaysOnTopLayer; // Always-on-top windows
        std::vector<std::shared_ptr<WindowWidget>> _windows;          // Combined for compatibility
        std::unordered_map<std::string, std::shared_ptr<WindowWidget>> _windowMap;
        
        // Reverse lookup: window pointer -> ID for O(1) lookups
        std::unordered_map<WindowWidget*, std::string> _windowIdMap;
        
        // State
        std::string _focusedWindowId;
        std::string _activeInteractionWindowId;  // Window currently being dragged/resized
        bool _enabled = true;
        bool _isDragging = false;
        bool _mouseDown = false;
        float _lastMouseX = 0;
        float _lastMouseY = 0;
        
        // Screen dimensions
        int _screenWidth = 1280;
        int _screenHeight = 720;
        
        // Modal window state
        std::string _modalWindowId;
        std::string _modalBlockerId;  // Window that triggered the modal
        
        // Cursor management
        CursorType _currentCursor = CursorType::Default;

        // Event subscription IDs
        rtype::ecs::EventBus::SubscriberId _mouseClickSub;
        rtype::ecs::EventBus::SubscriberId _mouseReleaseSub;
        rtype::ecs::EventBus::SubscriberId _mouseMoveSub;
        rtype::ecs::EventBus::SubscriberId _mouseWheelSub;
        rtype::ecs::EventBus::SubscriberId _keyPressSub;

        // Event handlers
        void handleMouseClick(const rtype::ecs::events::MouseButtonPressedEvent& event);
        void handleMouseRelease(const rtype::ecs::events::MouseButtonReleasedEvent& event);
        void handleMouseMove(const rtype::ecs::events::MouseMoveEvent& event);
        void handleMouseWheel(const rtype::ecs::events::MouseWheelEvent& event);
        void handleKeyPress(const rtype::ecs::events::KeyPressedEvent& event);

        /**
         * @brief Find the topmost window at given coordinates
         * @param x X coordinate
         * @param y Y coordinate
         * @return Window ID or empty string
         */
        std::string findWindowAt(float x, float y);

        /**
         * @brief Bring window to front of z-order
         * @param id Window identifier
         */
        void bringToFront(const std::string& id);
        
        /**
         * @brief Rebuild the combined _windows vector from layers
         */
        void rebuildWindowList();
        
        /**
         * @brief Get visible window IDs in z-order
         * @return Vector of visible window IDs
         */
        std::vector<std::string> getVisibleWindowIds() const;
        
        /**
         * @brief Render modal dimming overlay
         */
        void renderModalOverlay() const;
    };

} // namespace rtype::ui

#endif // WINDOWMANAGER_HPP_
