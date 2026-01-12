/*
** R-Type Engine - WindowWidget
** Draggable, resizable window with title bar (engine UI style)
*/

#ifndef WINDOWWIDGET_HPP_
#define WINDOWWIDGET_HPP_

#include "../Widget.hpp"
#include <functional>
#include <string>

namespace rtype::ui {

    /**
     * @brief Cursor types for different window interactions
     */
    enum class CursorType {
        Default,
        Arrow,
        IBeam,
        ResizeH,       // Horizontal resize (left/right edge)
        ResizeV,       // Vertical resize (top/bottom edge)
        ResizeNWSE,    // Diagonal resize (top-left/bottom-right)
        ResizeNESW,    // Diagonal resize (top-right/bottom-left)
        Hand,
        Pointer,
        NotAllowed
    };

    /**
     * @brief Window flags for customization
     */
    enum class WindowFlags : uint32_t {
        None           = 0,
        NoTitleBar     = 1 << 0,
        NoResize       = 1 << 1,
        NoMove         = 1 << 2,
        NoClose        = 1 << 3,
        NoCollapse     = 1 << 4,
        NoScrollbar    = 1 << 5,
        AlwaysOnTop    = 1 << 6,
        NoFocusOnClick = 1 << 7,
        Modal          = 1 << 8,
        Transparent    = 1 << 9
    };

    inline WindowFlags operator|(WindowFlags a, WindowFlags b) {
        return static_cast<WindowFlags>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
    }
    inline WindowFlags operator&(WindowFlags a, WindowFlags b) {
        return static_cast<WindowFlags>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
    }
    inline bool hasFlag(WindowFlags flags, WindowFlags flag) {
        return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(flag)) != 0;
    }

    /**
     * @brief Engine-style draggable, resizable window widget
     * 
     * Features:
     * - Title bar with close/collapse buttons
     * - Dragging by title bar
     * - Resizing from edges and corners
     * - Focus management and z-ordering
     * - Scrollable content area
     * - Collapsible to title bar only
     * - Customizable via WindowFlags
     */
    class WindowWidget : public Widget {
    public:
        static constexpr float TITLE_BAR_HEIGHT = 28.0f;
        static constexpr float RESIZE_BORDER = 8.0f;  // Increased for easier diagonal resize
        static constexpr float MIN_WIDTH = 150.0f;
        static constexpr float MIN_HEIGHT = 80.0f;
        static constexpr float BUTTON_SIZE = 20.0f;

        /**
         * @brief Construct a new WindowWidget
         * @param title Window title text
         * @param flags Window behavior flags
         */
        explicit WindowWidget(const std::string& title = "Window", 
                             WindowFlags flags = WindowFlags::None);

        ~WindowWidget() override = default;

        // === Title and Appearance ===
        
        void setTitle(const std::string& title);
        const std::string& getTitle() const;

        void setFlags(WindowFlags flags);
        WindowFlags getFlags() const;
        bool hasFlag(WindowFlags flag) const;

        void setTitleBarColor(const Color& color);
        Color getTitleBarColor() const;

        void setTitleBarActiveColor(const Color& color);
        Color getTitleBarActiveColor() const;

        // === Window State ===
        
        bool isCollapsed() const;
        void setCollapsed(bool collapsed);
        void toggleCollapsed();

        bool isFocused() const;
        void setWindowFocus(bool focused);

        // === Content Area ===
        
        UITransform getContentBounds() const;
        float getScrollOffset() const;
        void setScrollOffset(float offset);
        float getContentHeight() const;
        void setContentHeight(float height);

        /**
         * @brief Set a widget to render as window content
         * @param content The widget to use as content
         */
        void setContentWidget(std::shared_ptr<Widget> content);
        
        /**
         * @brief Get the content widget
         * @return Shared pointer to the content widget or nullptr
         */
        std::shared_ptr<Widget> getContentWidget() const;

        // === Cursor Management ===
        
        /**
         * @brief Get the suggested cursor type based on mouse position
         * @param x Mouse X coordinate
         * @param y Mouse Y coordinate
         * @return Suggested cursor type for current position
         */
        CursorType getCursorForPosition(float x, float y) const;

        /**
         * @brief Get the current suggested cursor (from last mouse move)
         * @return Current suggested cursor type
         */
        CursorType getSuggestedCursor() const { return _suggestedCursor; }

        // === Callbacks ===
        
        void setOnClose(std::function<void()> callback);
        void setOnCollapse(std::function<void(bool)> callback);
        void setOnMove(std::function<void(float, float)> callback);
        void setOnResize(std::function<void(float, float)> callback);
        void setOnFocus(std::function<void()> callback);

        // === Event Handling ===
        
        bool onMouseClick() override;
        bool onMouseMove(float x, float y) override;
        bool onMouseWheel(float delta) override;
        void onBlur() override;

        // For continuous drag/resize handling
        void handleMouseDrag(float x, float y, bool leftDown);
        void handleMouseRelease();

        // === Rendering ===
        
        /**
         * @brief Override render to apply content clipping
         * Ensures children are properly clipped to content bounds
         */
        void render() const override;
        
        void renderSelf() const override;

        // === Interaction Queries ===
        
        bool isInTitleBar(float x, float y) const;
        bool isInCloseButton(float x, float y) const;
        bool isInCollapseButton(float x, float y) const;
        bool isOnResizeEdge(float x, float y, int& edgeX, int& edgeY) const;
        
        /**
         * @brief Check if position is on scrollbar thumb
         * @param x Mouse X coordinate
         * @param y Mouse Y coordinate
         * @return true if on scrollbar thumb
         */
        bool isOnScrollbar(float x, float y) const;
        
        /**
         * @brief Check if scrollbar is being dragged
         * @return true if scrollbar is being dragged
         */
        bool isScrollbarDragging() const { return _scrollbarDragging; }

    protected:
        std::string _title;
        WindowFlags _flags = WindowFlags::None;
        
        // Colors
        Color _titleBarColor = {45, 50, 60, 255};
        Color _titleBarActiveColor = {55, 65, 85, 255};
        Color _windowBgColor = {35, 40, 48, 255};       // Fully opaque background
        Color _contentBgColor = {30, 35, 42, 255};      // Content area background
        Color _borderColor = {70, 80, 95, 255};
        Color _borderActiveColor = {90, 120, 180, 255};
        
        // State
        bool _collapsed = false;
        bool _windowFocused = false;
        bool _isDragging = false;
        bool _isResizing = false;
        int _resizeEdgeX = 0;  // -1=left, 0=none, 1=right
        int _resizeEdgeY = 0;  // -1=top, 0=none, 1=bottom
        float _dragOffsetX = 0;
        float _dragOffsetY = 0;
        float _resizeStartX = 0;
        float _resizeStartY = 0;
        float _resizeStartW = 0;
        float _resizeStartH = 0;
        
        // Scrolling
        float _scrollOffset = 0.0f;
        float _contentHeight = 0.0f;
        
        // Hover state for buttons
        mutable bool _hoverClose = false;
        mutable bool _hoverCollapse = false;
        
        // Callbacks
        std::function<void()> _onClose;
        std::function<void(bool)> _onCollapse;
        std::function<void(float, float)> _onMove;
        std::function<void(float, float)> _onResize;
        std::function<void()> _onFocusCallback;
        
        // Content widget (alternative to content drawer)
        std::shared_ptr<Widget> _contentWidget;
        
        // Cursor management
        mutable CursorType _suggestedCursor = CursorType::Default;
        
        // Interactive scrollbar
        bool _scrollbarHovered = false;
        bool _scrollbarDragging = false;
        float _scrollbarDragOffset = 0.0f;
        float _lastMouseY = 0.0f;

        void renderTitleBar() const;
        void renderContent() const;
        void renderScrollbar() const;
        void renderResizeHandle() const;
        
        /**
         * @brief Calculate scrollbar thumb position and dimensions
         * @param thumbY Output: Y position of thumb
         * @param thumbHeight Output: Height of thumb
         * @return true if scrollbar is visible (content overflows)
         */
        bool getScrollbarThumbGeometry(float& thumbY, float& thumbHeight) const;
    };

} // namespace rtype::ui

#endif // WINDOWWIDGET_HPP_
