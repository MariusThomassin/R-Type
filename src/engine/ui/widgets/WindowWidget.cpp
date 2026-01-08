/*
** R-Type Engine - WindowWidget Implementation
** Draggable, resizable window with title bar (engine UI style)
*/

#include "WindowWidget.hpp"
#include <raylib.h>
#include <algorithm>
#include <cmath>

namespace rtype::ui {

    WindowWidget::WindowWidget(const std::string& title, WindowFlags flags)
        : _title(title), _flags(flags) {
        // Default window size
        setSize(400.0f, 300.0f);
        
        // Default style
        _m_style.backgroundColor = UIColor(35, 40, 48, 245);
        _m_style.borderColor = UIColor(70, 80, 95, 255);
        _m_style.borderWidth = 1.0f;
    }

    void WindowWidget::setTitle(const std::string& title) {
        _title = title;
    }

    const std::string& WindowWidget::getTitle() const {
        return _title;
    }

    void WindowWidget::setFlags(WindowFlags flags) {
        _flags = flags;
    }

    WindowFlags WindowWidget::getFlags() const {
        return _flags;
    }

    bool WindowWidget::hasFlag(WindowFlags flag) const {
        return rtype::ui::hasFlag(_flags, flag);
    }

    void WindowWidget::setTitleBarColor(const Color& color) {
        _titleBarColor = color;
    }

    Color WindowWidget::getTitleBarColor() const {
        return _titleBarColor;
    }

    void WindowWidget::setTitleBarActiveColor(const Color& color) {
        _titleBarActiveColor = color;
    }

    Color WindowWidget::getTitleBarActiveColor() const {
        return _titleBarActiveColor;
    }

    bool WindowWidget::isCollapsed() const {
        return _collapsed;
    }

    void WindowWidget::setCollapsed(bool collapsed) {
        _collapsed = collapsed;
        if (_onCollapse) _onCollapse(_collapsed);
    }

    void WindowWidget::toggleCollapsed() {
        setCollapsed(!_collapsed);
    }

    bool WindowWidget::isFocused() const {
        return _windowFocused;
    }

    void WindowWidget::setWindowFocus(bool focused) {
        _windowFocused = focused;
        if (focused && _onFocusCallback) _onFocusCallback();
    }

    UITransform WindowWidget::getContentBounds() const {
        auto transform = getAbsoluteTransform();
        float titleHeight = hasFlag(WindowFlags::NoTitleBar) ? 0.0f : TITLE_BAR_HEIGHT;
        
        return {
            transform.x + _m_style.padding,
            transform.y + titleHeight + _m_style.padding,
            transform.width - _m_style.padding * 2,
            _collapsed ? 0.0f : (transform.height - titleHeight - _m_style.padding * 2)
        };
    }

    float WindowWidget::getScrollOffset() const {
        return _scrollOffset;
    }

    void WindowWidget::setScrollOffset(float offset) {
        float maxScroll = std::max(0.0f, _contentHeight - getContentBounds().height);
        _scrollOffset = std::clamp(offset, 0.0f, maxScroll);
    }

    float WindowWidget::getContentHeight() const {
        return _contentHeight;
    }

    void WindowWidget::setContentHeight(float height) {
        _contentHeight = height;
    }

    void WindowWidget::setOnClose(std::function<void()> callback) {
        _onClose = callback;
    }

    void WindowWidget::setOnCollapse(std::function<void(bool)> callback) {
        _onCollapse = callback;
    }

    void WindowWidget::setOnMove(std::function<void(float, float)> callback) {
        _onMove = callback;
    }

    void WindowWidget::setOnResize(std::function<void(float, float)> callback) {
        _onResize = callback;
    }

    void WindowWidget::setOnFocus(std::function<void()> callback) {
        _onFocusCallback = callback;
    }

    void WindowWidget::setContentWidget(std::shared_ptr<Widget> content) {
        // Remove old content widget if exists
        if (_contentWidget) {
            removeChild(_contentWidget);
        }
        
        _contentWidget = content;
        
        if (_contentWidget) {
            addChild(_contentWidget);
            // Position content within content area
            auto bounds = getContentBounds();
            _contentWidget->setPosition(0, 0); // Relative to content area
        }
    }

    std::shared_ptr<Widget> WindowWidget::getContentWidget() const {
        return _contentWidget;
    }

    CursorType WindowWidget::getCursorForPosition(float x, float y) const {
        if (!contains(x, y) && !_isResizing && !_isDragging) {
            return CursorType::Default;
        }
        
        // Check resize edges first
        if (!hasFlag(WindowFlags::NoResize) && !_collapsed) {
            int edgeX, edgeY;
            if (isOnResizeEdge(x, y, edgeX, edgeY)) {
                // Determine cursor based on edge combination
                if (edgeX != 0 && edgeY != 0) {
                    // Corner resize
                    if ((edgeX == -1 && edgeY == -1) || (edgeX == 1 && edgeY == 1)) {
                        return CursorType::ResizeNWSE;
                    } else {
                        return CursorType::ResizeNESW;
                    }
                } else if (edgeX != 0) {
                    return CursorType::ResizeH;
                } else if (edgeY != 0) {
                    return CursorType::ResizeV;
                }
            }
        }
        
        // Check scrollbar
        if (isOnScrollbar(x, y)) {
            return CursorType::Pointer;
        }
        
        // Check close/collapse buttons
        if (_hoverClose || _hoverCollapse) {
            return CursorType::Pointer;
        }
        
        // Title bar = move cursor
        if (isInTitleBar(x, y) && !hasFlag(WindowFlags::NoMove)) {
            return CursorType::Arrow;
        }
        
        return CursorType::Default;
    }

    bool WindowWidget::isOnScrollbar(float x, float y) const {
        if (hasFlag(WindowFlags::NoScrollbar) || _collapsed) return false;
        
        auto content = getContentBounds();
        if (_contentHeight <= content.height) return false;
        
        float scrollbarWidth = 8.0f;
        float scrollbarX = content.x + content.width - scrollbarWidth;
        float scrollbarY = content.y;
        float scrollbarHeight = content.height;
        
        // Check if in scrollbar track area
        if (x >= scrollbarX && x < scrollbarX + scrollbarWidth &&
            y >= scrollbarY && y < scrollbarY + scrollbarHeight) {
            return true;
        }
        
        return false;
    }

    bool WindowWidget::getScrollbarThumbGeometry(float& thumbY, float& thumbHeight) const {
        auto content = getContentBounds();
        if (_contentHeight <= content.height) return false;
        
        float scrollbarWidth = 8.0f;
        float scrollbarHeight = content.height;
        float scrollbarY = content.y;
        
        float visibleRatio = content.height / _contentHeight;
        thumbHeight = std::max(20.0f, scrollbarHeight * visibleRatio);
        float maxScroll = _contentHeight - content.height;
        float scrollRatio = maxScroll > 0 ? _scrollOffset / maxScroll : 0;
        thumbY = scrollbarY + scrollRatio * (scrollbarHeight - thumbHeight);
        
        return true;
    }

    bool WindowWidget::onMouseClick() {
        // Check close button
        if (!hasFlag(WindowFlags::NoClose) && _hoverClose) {
            if (_onClose) _onClose();
            return true;
        }

        // Check collapse button
        if (!hasFlag(WindowFlags::NoCollapse) && _hoverCollapse) {
            toggleCollapsed();
            return true;
        }
        
        // Check scrollbar click
        if (!hasFlag(WindowFlags::NoScrollbar) && !_collapsed) {
            float thumbY, thumbHeight;
            if (getScrollbarThumbGeometry(thumbY, thumbHeight)) {
                auto content = getContentBounds();
                float scrollbarWidth = 8.0f;
                float scrollbarX = content.x + content.width - scrollbarWidth;
                
                if (_lastMouseY >= content.y && _lastMouseY < content.y + content.height) {
                    // Check if click is on thumb
                    if (_lastMouseY >= thumbY && _lastMouseY < thumbY + thumbHeight) {
                        _scrollbarDragging = true;
                        _scrollbarDragOffset = _lastMouseY - thumbY;
                        return true;
                    }
                    // Click above or below thumb - page scroll
                    else if (isOnScrollbar(_lastMouseY, _lastMouseY)) {
                        float maxScroll = _contentHeight - content.height;
                        if (_lastMouseY < thumbY) {
                            setScrollOffset(_scrollOffset - content.height * 0.9f);
                        } else {
                            setScrollOffset(_scrollOffset + content.height * 0.9f);
                        }
                        return true;
                    }
                }
            }
        }

        return false;
    }

    bool WindowWidget::onMouseMove(float x, float y) {
        auto transform = getAbsoluteTransform();
        _lastMouseY = y;  // Track for scrollbar dragging
        
        // Update button hover states
        _hoverClose = isInCloseButton(x, y);
        _hoverCollapse = isInCollapseButton(x, y);
        
        // Update scrollbar hover state
        _scrollbarHovered = isOnScrollbar(x, y);
        
        // Update suggested cursor
        _suggestedCursor = getCursorForPosition(x, y);
        
        // Check resize cursor
        int edgeX, edgeY;
        if (!hasFlag(WindowFlags::NoResize) && isOnResizeEdge(x, y, edgeX, edgeY)) {
            return true;
        }
        
        return contains(x, y);
    }

    bool WindowWidget::onMouseWheel(float delta) {
        if (hasFlag(WindowFlags::NoScrollbar)) return false;
        
        setScrollOffset(_scrollOffset - delta * 30.0f);
        return true;
    }

    void WindowWidget::onBlur() {
        setWindowFocus(false);
    }

    void WindowWidget::handleMouseDrag(float x, float y, bool leftDown) {
        auto transform = getAbsoluteTransform();
        _lastMouseY = y;
        
        if (leftDown) {
            // Handle scrollbar dragging first
            if (_scrollbarDragging) {
                auto content = getContentBounds();
                float scrollbarHeight = content.height;
                
                float thumbY, thumbHeight;
                if (getScrollbarThumbGeometry(thumbY, thumbHeight)) {
                    float newThumbY = y - _scrollbarDragOffset;
                    float minThumbY = content.y;
                    float maxThumbY = content.y + scrollbarHeight - thumbHeight;
                    newThumbY = std::clamp(newThumbY, minThumbY, maxThumbY);
                    
                    float scrollRatio = (newThumbY - content.y) / (scrollbarHeight - thumbHeight);
                    float maxScroll = _contentHeight - content.height;
                    setScrollOffset(scrollRatio * maxScroll);
                }
                return;
            }
            
            if (!_isDragging && !_isResizing) {
                // Check if we should start dragging
                if (!hasFlag(WindowFlags::NoMove) && isInTitleBar(x, y) && 
                    !isInCloseButton(x, y) && !isInCollapseButton(x, y)) {
                    _isDragging = true;
                    _dragOffsetX = x - transform.x;
                    _dragOffsetY = y - transform.y;
                }
                // Check if we should start resizing
                else if (!hasFlag(WindowFlags::NoResize) && !_collapsed) {
                    int edgeX, edgeY;
                    if (isOnResizeEdge(x, y, edgeX, edgeY)) {
                        _isResizing = true;
                        _resizeEdgeX = edgeX;
                        _resizeEdgeY = edgeY;
                        _resizeStartX = x;
                        _resizeStartY = y;
                        _resizeStartW = transform.width;
                        _resizeStartH = transform.height;
                        _dragOffsetX = transform.x;
                        _dragOffsetY = transform.y;
                    }
                }
            }
            
            // Continue dragging
            if (_isDragging) {
                float newX = x - _dragOffsetX;
                float newY = y - _dragOffsetY;
                setPosition(newX, newY);
                if (_onMove) _onMove(newX, newY);
            }
            
            // Continue resizing
            if (_isResizing) {
                float deltaX = x - _resizeStartX;
                float deltaY = y - _resizeStartY;
                
                float newW = _resizeStartW;
                float newH = _resizeStartH;
                float newX = _dragOffsetX;
                float newY = _dragOffsetY;
                
                if (_resizeEdgeX == 1) {  // Right edge
                    newW = std::max(MIN_WIDTH, _resizeStartW + deltaX);
                } else if (_resizeEdgeX == -1) {  // Left edge
                    float widthChange = std::min(deltaX, _resizeStartW - MIN_WIDTH);
                    newX = _dragOffsetX + widthChange;
                    newW = _resizeStartW - widthChange;
                }
                
                if (_resizeEdgeY == 1) {  // Bottom edge
                    newH = std::max(MIN_HEIGHT, _resizeStartH + deltaY);
                } else if (_resizeEdgeY == -1) {  // Top edge
                    float heightChange = std::min(deltaY, _resizeStartH - MIN_HEIGHT);
                    newY = _dragOffsetY + heightChange;
                    newH = _resizeStartH - heightChange;
                }
                
                setPosition(newX, newY);
                setSize(newW, newH);
                if (_onResize) _onResize(newW, newH);
            }
        }
    }

    void WindowWidget::handleMouseRelease() {
        _isDragging = false;
        _isResizing = false;
        _scrollbarDragging = false;
    }

    bool WindowWidget::isInTitleBar(float x, float y) const {
        if (hasFlag(WindowFlags::NoTitleBar)) return false;
        
        auto transform = getAbsoluteTransform();
        return x >= transform.x && x < transform.x + transform.width &&
               y >= transform.y && y < transform.y + TITLE_BAR_HEIGHT;
    }

    bool WindowWidget::isInCloseButton(float x, float y) const {
        if (hasFlag(WindowFlags::NoClose) || hasFlag(WindowFlags::NoTitleBar)) return false;
        
        auto transform = getAbsoluteTransform();
        float btnX = transform.x + transform.width - BUTTON_SIZE - 4;
        float btnY = transform.y + (TITLE_BAR_HEIGHT - BUTTON_SIZE) / 2;
        
        return x >= btnX && x < btnX + BUTTON_SIZE &&
               y >= btnY && y < btnY + BUTTON_SIZE;
    }

    bool WindowWidget::isInCollapseButton(float x, float y) const {
        if (hasFlag(WindowFlags::NoCollapse) || hasFlag(WindowFlags::NoTitleBar)) return false;
        
        auto transform = getAbsoluteTransform();
        float offset = hasFlag(WindowFlags::NoClose) ? BUTTON_SIZE + 4 : (BUTTON_SIZE + 4) * 2;
        float btnX = transform.x + transform.width - offset;
        float btnY = transform.y + (TITLE_BAR_HEIGHT - BUTTON_SIZE) / 2;
        
        return x >= btnX && x < btnX + BUTTON_SIZE &&
               y >= btnY && y < btnY + BUTTON_SIZE;
    }

    bool WindowWidget::isOnResizeEdge(float x, float y, int& edgeX, int& edgeY) const {
        if (_collapsed) return false;
        
        auto transform = getAbsoluteTransform();
        edgeX = 0;
        edgeY = 0;
        
        bool nearLeft = x >= transform.x - RESIZE_BORDER && x < transform.x + RESIZE_BORDER;
        bool nearRight = x >= transform.x + transform.width - RESIZE_BORDER && 
                        x < transform.x + transform.width + RESIZE_BORDER;
        bool nearTop = y >= transform.y - RESIZE_BORDER && y < transform.y + RESIZE_BORDER;
        bool nearBottom = y >= transform.y + transform.height - RESIZE_BORDER &&
                         y < transform.y + transform.height + RESIZE_BORDER;
        
        if (nearLeft) edgeX = -1;
        else if (nearRight) edgeX = 1;
        
        if (nearTop) edgeY = -1;
        else if (nearBottom) edgeY = 1;
        
        return edgeX != 0 || edgeY != 0;
    }

    void WindowWidget::render() const {
        if (!isVisible()) return;
        
        // Render window background, title bar, etc.
        renderSelf();
        
        // If collapsed, don't render children
        if (_collapsed) return;
        
        // Get content bounds for clipping
        auto content = getContentBounds();
        
        // Apply scissor mode to clip children to content area
        BeginScissorMode(
            static_cast<int>(content.x),
            static_cast<int>(content.y),
            static_cast<int>(content.width),
            static_cast<int>(content.height)
        );
        
        // Render all child widgets (clipped to content area)
        for (auto& child : getChildren()) {
            child->render();
        }
        
        EndScissorMode();
    }

    void WindowWidget::renderSelf() const {
        auto transform = getAbsoluteTransform();
        float windowHeight = _collapsed ? TITLE_BAR_HEIGHT : transform.height;
        
        // Drop shadow
        if (!hasFlag(WindowFlags::Transparent)) {
            DrawRectangle(
                static_cast<int>(transform.x + 4),
                static_cast<int>(transform.y + 4),
                static_cast<int>(transform.width),
                static_cast<int>(windowHeight),
                {0, 0, 0, 60}
            );
        }
        
        // Window background
        if (!hasFlag(WindowFlags::Transparent)) {
            DrawRectangle(
                static_cast<int>(transform.x),
                static_cast<int>(transform.y),
                static_cast<int>(transform.width),
                static_cast<int>(windowHeight),
                _windowBgColor
            );
        }
        
        // Border
        Color borderCol = _windowFocused ? _borderActiveColor : _borderColor;
        DrawRectangleLines(
            static_cast<int>(transform.x),
            static_cast<int>(transform.y),
            static_cast<int>(transform.width),
            static_cast<int>(windowHeight),
            borderCol
        );
        
        // Title bar
        if (!hasFlag(WindowFlags::NoTitleBar)) {
            renderTitleBar();
        }
        
        // Content and scrollbar
        if (!_collapsed) {
            renderContent();
            if (!hasFlag(WindowFlags::NoScrollbar)) {
                renderScrollbar();
            }
            if (!hasFlag(WindowFlags::NoResize)) {
                renderResizeHandle();
            }
        }
    }

    void WindowWidget::renderTitleBar() const {
        auto transform = getAbsoluteTransform();
        
        // Title bar background
        Color barColor = _windowFocused ? _titleBarActiveColor : _titleBarColor;
        DrawRectangle(
            static_cast<int>(transform.x + 1),
            static_cast<int>(transform.y + 1),
            static_cast<int>(transform.width - 2),
            static_cast<int>(TITLE_BAR_HEIGHT - 1),
            barColor
        );
        
        // Title text
        int textWidth = MeasureText(_title.c_str(), 14);
        DrawText(
            _title.c_str(),
            static_cast<int>(transform.x + 10),
            static_cast<int>(transform.y + (TITLE_BAR_HEIGHT - 14) / 2),
            14,
            _windowFocused ? WHITE : Color{200, 200, 200, 255}
        );
        
        // Close button
        if (!hasFlag(WindowFlags::NoClose)) {
            float btnX = transform.x + transform.width - BUTTON_SIZE - 4;
            float btnY = transform.y + (TITLE_BAR_HEIGHT - BUTTON_SIZE) / 2;
            
            Color closeBg = _hoverClose ? Color{180, 60, 60, 255} : Color{80, 40, 40, 255};
            DrawRectangle(static_cast<int>(btnX), static_cast<int>(btnY),
                         static_cast<int>(BUTTON_SIZE), static_cast<int>(BUTTON_SIZE), closeBg);
            
            // X icon
            float cx = btnX + BUTTON_SIZE / 2;
            float cy = btnY + BUTTON_SIZE / 2;
            float s = 5;
            DrawLine(static_cast<int>(cx - s), static_cast<int>(cy - s),
                    static_cast<int>(cx + s), static_cast<int>(cy + s),
                    _hoverClose ? WHITE : Color{200, 200, 200, 255});
            DrawLine(static_cast<int>(cx + s), static_cast<int>(cy - s),
                    static_cast<int>(cx - s), static_cast<int>(cy + s),
                    _hoverClose ? WHITE : Color{200, 200, 200, 255});
        }
        
        // Collapse button
        if (!hasFlag(WindowFlags::NoCollapse)) {
            float offset = hasFlag(WindowFlags::NoClose) ? BUTTON_SIZE + 4 : (BUTTON_SIZE + 4) * 2;
            float btnX = transform.x + transform.width - offset;
            float btnY = transform.y + (TITLE_BAR_HEIGHT - BUTTON_SIZE) / 2;
            
            Color collapseBg = _hoverCollapse ? Color{70, 80, 100, 255} : Color{50, 55, 70, 255};
            DrawRectangle(static_cast<int>(btnX), static_cast<int>(btnY),
                         static_cast<int>(BUTTON_SIZE), static_cast<int>(BUTTON_SIZE), collapseBg);
            
            // Arrow icon
            float cx = btnX + BUTTON_SIZE / 2;
            float cy = btnY + BUTTON_SIZE / 2;
            if (_collapsed) {
                // Down arrow (expand)
                DrawTriangle(
                    {cx - 5, cy - 3}, {cx + 5, cy - 3}, {cx, cy + 4},
                    _hoverCollapse ? WHITE : Color{180, 180, 180, 255}
                );
            } else {
                // Up arrow (collapse)
                DrawTriangle(
                    {cx, cy - 4}, {cx + 5, cy + 3}, {cx - 5, cy + 3},
                    _hoverCollapse ? WHITE : Color{180, 180, 180, 255}
                );
            }
        }
        
        // Separator line
        DrawLine(
            static_cast<int>(transform.x),
            static_cast<int>(transform.y + TITLE_BAR_HEIGHT),
            static_cast<int>(transform.x + transform.width),
            static_cast<int>(transform.y + TITLE_BAR_HEIGHT),
            _borderColor
        );
    }

    void WindowWidget::renderContent() const {
        // Draw content area background for solid appearance
        auto bounds = getContentBounds();
        
        // Draw solid content background to prevent any transparency/overlap issues
        DrawRectangle(
            static_cast<int>(bounds.x),
            static_cast<int>(bounds.y),
            static_cast<int>(bounds.width),
            static_cast<int>(bounds.height),
            _contentBgColor
        );
        
        // Note: Child widgets are rendered via render() override with scissor clipping
    }

    void WindowWidget::renderScrollbar() const {
        auto content = getContentBounds();
        if (_contentHeight <= content.height) return;
        
        float scrollbarWidth = 8.0f;
        float scrollbarHeight = content.height;
        float scrollbarX = content.x + content.width - scrollbarWidth;
        float scrollbarY = content.y;
        
        // Track
        Color trackColor = _scrollbarHovered || _scrollbarDragging 
            ? Color{40, 40, 50, 255} 
            : Color{30, 30, 35, 255};
        DrawRectangle(
            static_cast<int>(scrollbarX),
            static_cast<int>(scrollbarY),
            static_cast<int>(scrollbarWidth),
            static_cast<int>(scrollbarHeight),
            trackColor
        );
        
        // Thumb
        float thumbY, thumbHeight;
        if (getScrollbarThumbGeometry(thumbY, thumbHeight)) {
            Color thumbColor = _scrollbarDragging 
                ? Color{120, 140, 180, 255}
                : (_scrollbarHovered ? Color{100, 115, 145, 255} : Color{80, 90, 110, 255});
            
            DrawRectangle(
                static_cast<int>(scrollbarX + 1),
                static_cast<int>(thumbY),
                static_cast<int>(scrollbarWidth - 2),
                static_cast<int>(thumbHeight),
                thumbColor
            );
        }
    }

    void WindowWidget::renderResizeHandle() const {
        auto transform = getAbsoluteTransform();
        
        // Bottom-right corner resize grip
        float x = transform.x + transform.width - 12;
        float y = transform.y + transform.height - 12;
        
        Color gripColor = {100, 110, 130, 200};
        for (int i = 0; i < 3; ++i) {
            DrawLine(
                static_cast<int>(x + i * 4),
                static_cast<int>(y + 10),
                static_cast<int>(x + 10),
                static_cast<int>(y + i * 4),
                gripColor
            );
        }
    }

} // namespace rtype::ui
