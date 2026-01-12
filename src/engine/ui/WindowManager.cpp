/*
** R-Type Engine - WindowManager Implementation
** Manages multiple windows with focus, z-ordering, and docking
*/

#include "WindowManager.hpp"
#include <raylib.h>
#include <algorithm>
#include <cmath>

namespace rtype::ui {

    WindowManager::WindowManager(rtype::ecs::EventBus& eventBus)
        : _eventBus(eventBus) {
        
        // Subscribe to mouse events
        _mouseClickSub = _eventBus.subscribe<rtype::ecs::events::MouseButtonPressedEvent>(
            [this](const rtype::ecs::events::MouseButtonPressedEvent& e) {
                handleMouseClick(e);
            }
        );
        
        _mouseReleaseSub = _eventBus.subscribe<rtype::ecs::events::MouseButtonReleasedEvent>(
            [this](const rtype::ecs::events::MouseButtonReleasedEvent& e) {
                handleMouseRelease(e);
            }
        );
        
        _mouseMoveSub = _eventBus.subscribe<rtype::ecs::events::MouseMoveEvent>(
            [this](const rtype::ecs::events::MouseMoveEvent& e) {
                handleMouseMove(e);
            }
        );
        
        _mouseWheelSub = _eventBus.subscribe<rtype::ecs::events::MouseWheelEvent>(
            [this](const rtype::ecs::events::MouseWheelEvent& e) {
                handleMouseWheel(e);
            }
        );
        
        // Subscribe to keyboard events for navigation
        _keyPressSub = _eventBus.subscribe<rtype::ecs::events::KeyPressedEvent>(
            [this](const rtype::ecs::events::KeyPressedEvent& e) {
                handleKeyPress(e);
            }
        );
    }

    WindowManager::~WindowManager() {
        _eventBus.unsubscribe<rtype::ecs::events::MouseButtonPressedEvent>(_mouseClickSub);
        _eventBus.unsubscribe<rtype::ecs::events::MouseButtonReleasedEvent>(_mouseReleaseSub);
        _eventBus.unsubscribe<rtype::ecs::events::MouseMoveEvent>(_mouseMoveSub);
        _eventBus.unsubscribe<rtype::ecs::events::MouseWheelEvent>(_mouseWheelSub);
        _eventBus.unsubscribe<rtype::ecs::events::KeyPressedEvent>(_keyPressSub);
    }

    std::shared_ptr<WindowWidget> WindowManager::createWindow(
        const std::string& id,
        const std::string& title,
        float x, float y,
        float width, float height,
        WindowFlags flags
    ) {
        // Remove existing window with same ID
        if (hasWindow(id)) {
            removeWindow(id);
        }
        
        auto window = std::make_shared<WindowWidget>(title, flags);
        window->setPosition(x, y);
        window->setSize(width, height);
        
        // Set up close callback to remove from manager
        window->setOnClose([this, id]() {
            hideWindow(id);
        });
        
        // Note: We don't set onFocus callback here because focusWindow() 
        // is called directly by WindowManager when handling clicks.
        // Setting it would cause infinite recursion.
        
        // Add to appropriate layer based on flags
        if (window->hasFlag(WindowFlags::AlwaysOnTop)) {
            _alwaysOnTopLayer.push_back(window);
        } else {
            _normalLayer.push_back(window);
        }
        
        _windowMap[id] = window;
        _windowIdMap[window.get()] = id;
        
        // Rebuild combined list
        rebuildWindowList();
        
        // Auto-focus new window
        focusWindow(id);
        
        return window;
    }

    std::shared_ptr<WindowWidget> WindowManager::getWindow(const std::string& id) {
        auto it = _windowMap.find(id);
        return (it != _windowMap.end()) ? it->second : nullptr;
    }

    void WindowManager::removeWindow(const std::string& id) {
        auto it = _windowMap.find(id);
        if (it == _windowMap.end()) return;
        
        auto window = it->second;
        
        // Remove from appropriate layer
        if (window->hasFlag(WindowFlags::AlwaysOnTop)) {
            _alwaysOnTopLayer.erase(
                std::remove(_alwaysOnTopLayer.begin(), _alwaysOnTopLayer.end(), window),
                _alwaysOnTopLayer.end()
            );
        } else {
            _normalLayer.erase(
                std::remove(_normalLayer.begin(), _normalLayer.end(), window),
                _normalLayer.end()
            );
        }
        
        // Remove from lookup maps
        _windowIdMap.erase(window.get());
        _windowMap.erase(it);
        
        // Clear modal if this was the modal window
        if (_modalWindowId == id) {
            _modalWindowId.clear();
            _modalBlockerId.clear();
        }
        
        // Rebuild combined list
        rebuildWindowList();
        
        if (_focusedWindowId == id) {
            _focusedWindowId.clear();
            // Focus next visible window using optimized lookup
            if (!_windows.empty()) {
                for (auto rit = _windows.rbegin(); rit != _windows.rend(); ++rit) {
                    if ((*rit)->isVisible()) {
                        auto idIt = _windowIdMap.find((*rit).get());
                        if (idIt != _windowIdMap.end()) {
                            focusWindow(idIt->second);
                            break;
                        }
                    }
                }
            }
        }
    }

    bool WindowManager::hasWindow(const std::string& id) const {
        return _windowMap.find(id) != _windowMap.end();
    }

    const std::vector<std::shared_ptr<WindowWidget>>& WindowManager::getWindows() const {
        return _windows;
    }

    std::vector<std::string> WindowManager::getWindowIds() const {
        std::vector<std::string> ids;
        ids.reserve(_windowMap.size());
        
        for (const auto& [id, window] : _windowMap) {
            ids.push_back(id);
        }
        
        return ids;
    }

    std::string WindowManager::getWindowId(const std::shared_ptr<WindowWidget>& window) const {
        if (!window) return "";
        
        auto it = _windowIdMap.find(window.get());
        return (it != _windowIdMap.end()) ? it->second : "";
    }

    void WindowManager::focusWindow(const std::string& id) {
        auto window = getWindow(id);
        if (!window) return;
        
        // Unfocus previous
        if (!_focusedWindowId.empty() && _focusedWindowId != id) {
            if (auto prev = getWindow(_focusedWindowId)) {
                prev->setWindowFocus(false);
            }
        }
        
        _focusedWindowId = id;
        window->setWindowFocus(true);
        bringToFront(id);
    }

    std::shared_ptr<WindowWidget> WindowManager::getFocusedWindow() const {
        if (_focusedWindowId.empty()) return nullptr;
        auto it = _windowMap.find(_focusedWindowId);
        return (it != _windowMap.end()) ? it->second : nullptr;
    }

    std::string WindowManager::getFocusedWindowId() const {
        return _focusedWindowId;
    }

    void WindowManager::focusNextWindow() {
        auto visibleIds = getVisibleWindowIds();
        if (visibleIds.empty()) return;
        
        // Find current focused index
        size_t currentIdx = 0;
        for (size_t i = 0; i < visibleIds.size(); ++i) {
            if (visibleIds[i] == _focusedWindowId) {
                currentIdx = i;
                break;
            }
        }
        
        // Focus next (wrap around)
        size_t nextIdx = (currentIdx + 1) % visibleIds.size();
        focusWindow(visibleIds[nextIdx]);
    }

    void WindowManager::focusPrevWindow() {
        auto visibleIds = getVisibleWindowIds();
        if (visibleIds.empty()) return;
        
        // Find current focused index
        size_t currentIdx = 0;
        for (size_t i = 0; i < visibleIds.size(); ++i) {
            if (visibleIds[i] == _focusedWindowId) {
                currentIdx = i;
                break;
            }
        }
        
        // Focus previous (wrap around)
        size_t prevIdx = (currentIdx == 0) ? visibleIds.size() - 1 : currentIdx - 1;
        focusWindow(visibleIds[prevIdx]);
    }

    // === Modal Windows ===

    void WindowManager::showModal(const std::string& id, const std::string& blockerId) {
        auto window = getWindow(id);
        if (!window) return;
        
        _modalWindowId = id;
        _modalBlockerId = blockerId;
        
        window->setVisible(true);
        focusWindow(id);
    }

    void WindowManager::closeModal() {
        if (_modalWindowId.empty()) return;
        
        hideWindow(_modalWindowId);
        
        std::string blockerToFocus = _modalBlockerId;
        _modalWindowId.clear();
        _modalBlockerId.clear();
        
        // Return focus to blocker window if it exists
        if (!blockerToFocus.empty() && hasWindow(blockerToFocus)) {
            focusWindow(blockerToFocus);
        }
    }

    std::shared_ptr<WindowWidget> WindowManager::getModalWindow() const {
        if (_modalWindowId.empty()) return nullptr;
        auto it = _windowMap.find(_modalWindowId);
        return (it != _windowMap.end()) ? it->second : nullptr;
    }

    bool WindowManager::hasActiveModal() const {
        return !_modalWindowId.empty();
    }

    bool WindowManager::isInputBlocked() const {
        return hasActiveModal();
    }

    void WindowManager::showWindow(const std::string& id) {
        if (auto window = getWindow(id)) {
            window->setVisible(true);
            focusWindow(id);
        }
    }

    void WindowManager::hideWindow(const std::string& id) {
        if (auto window = getWindow(id)) {
            window->setVisible(false);
            
            if (_focusedWindowId == id) {
                _focusedWindowId.clear();
                // Focus next visible window using optimized lookup
                for (auto rit = _windows.rbegin(); rit != _windows.rend(); ++rit) {
                    if ((*rit)->isVisible()) {
                        auto idIt = _windowIdMap.find((*rit).get());
                        if (idIt != _windowIdMap.end()) {
                            focusWindow(idIt->second);
                            return;
                        }
                    }
                }
            }
        }
    }

    void WindowManager::toggleWindow(const std::string& id) {
        if (isWindowVisible(id)) {
            hideWindow(id);
        } else {
            showWindow(id);
        }
    }

    bool WindowManager::isWindowVisible(const std::string& id) const {
        auto it = _windowMap.find(id);
        return (it != _windowMap.end()) ? it->second->isVisible() : false;
    }

    void WindowManager::update(float dt) {
        if (!_enabled) return;
        
        for (auto& window : _windows) {
            if (window->isVisible()) {
                window->update(dt);
            }
        }
        
        // Update cursor
        updateCursor();
    }

    void WindowManager::render() const {
        if (!_enabled) return;
        
        // Render in z-order (back to front)
        for (const auto& window : _windows) {
            if (window->isVisible()) {
                window->render();
            }
        }
        
        // Render modal overlay if active
        if (hasActiveModal()) {
            renderModalOverlay();
        }
    }

    // === Cursor Management ===

    CursorType WindowManager::getSuggestedCursor() const {
        // Check windows in reverse z-order (front to back)
        for (auto rit = _windows.rbegin(); rit != _windows.rend(); ++rit) {
            if ((*rit)->isVisible() && (*rit)->contains(_lastMouseX, _lastMouseY)) {
                CursorType cursor = (*rit)->getSuggestedCursor();
                if (cursor != CursorType::Default) {
                    return cursor;
                }
            }
        }
        return CursorType::Default;
    }

    void WindowManager::updateCursor() {
        CursorType newCursor = getSuggestedCursor();
        
        if (newCursor != _currentCursor) {
            _currentCursor = newCursor;
            
            // Map CursorType to Raylib cursor
            switch (_currentCursor) {
                case CursorType::Arrow:
                    SetMouseCursor(MOUSE_CURSOR_ARROW);
                    break;
                case CursorType::IBeam:
                    SetMouseCursor(MOUSE_CURSOR_IBEAM);
                    break;
                case CursorType::ResizeH:
                    SetMouseCursor(MOUSE_CURSOR_RESIZE_EW);
                    break;
                case CursorType::ResizeV:
                    SetMouseCursor(MOUSE_CURSOR_RESIZE_NS);
                    break;
                case CursorType::ResizeNWSE:
                    SetMouseCursor(MOUSE_CURSOR_RESIZE_NWSE);
                    break;
                case CursorType::ResizeNESW:
                    SetMouseCursor(MOUSE_CURSOR_RESIZE_NESW);
                    break;
                case CursorType::Hand:
                case CursorType::Pointer:
                    SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
                    break;
                case CursorType::NotAllowed:
                    SetMouseCursor(MOUSE_CURSOR_NOT_ALLOWED);
                    break;
                case CursorType::Default:
                default:
                    SetMouseCursor(MOUSE_CURSOR_DEFAULT);
                    break;
            }
        }
    }

    // === Layout ===

    void WindowManager::setScreenSize(int width, int height) {
        _screenWidth = width;
        _screenHeight = height;
    }

    void WindowManager::layoutWindows(LayoutPattern pattern, float padding) {
        std::vector<std::string> visibleIds = getVisibleWindowIds();
        arrangeWindows(visibleIds, pattern, padding);
    }

    void WindowManager::arrangeWindows(const std::vector<std::string>& windowIds, 
                                       LayoutPattern pattern, float padding) {
        if (windowIds.empty()) return;
        
        std::vector<std::shared_ptr<WindowWidget>> windows;
        for (const auto& id : windowIds) {
            if (auto win = getWindow(id)) {
                windows.push_back(win);
            }
        }
        
        if (windows.empty()) return;
        
        float menuBarHeight = 30.0f; // Account for debug menu bar
        float availWidth = static_cast<float>(_screenWidth) - padding * 2;
        float availHeight = static_cast<float>(_screenHeight) - menuBarHeight - padding * 2;
        
        switch (pattern) {
            case LayoutPattern::Cascade: {
                float offsetX = padding;
                float offsetY = menuBarHeight + padding;
                float cascadeStep = 30.0f;
                
                for (auto& win : windows) {
                    win->setPosition(offsetX, offsetY);
                    offsetX += cascadeStep;
                    offsetY += cascadeStep;
                    
                    // Wrap if going off screen
                    if (offsetX + 300 > _screenWidth) offsetX = padding;
                    if (offsetY + 200 > _screenHeight) offsetY = menuBarHeight + padding;
                }
                break;
            }
            case LayoutPattern::TileHorizontal: {
                float winWidth = (availWidth - padding * (windows.size() - 1)) / windows.size();
                float x = padding;
                
                for (auto& win : windows) {
                    win->setPosition(x, menuBarHeight + padding);
                    win->setSize(winWidth, availHeight);
                    x += winWidth + padding;
                }
                break;
            }
            case LayoutPattern::TileVertical: {
                float winHeight = (availHeight - padding * (windows.size() - 1)) / windows.size();
                float y = menuBarHeight + padding;
                
                for (auto& win : windows) {
                    win->setPosition(padding, y);
                    win->setSize(availWidth, winHeight);
                    y += winHeight + padding;
                }
                break;
            }
            case LayoutPattern::Grid: {
                size_t cols = static_cast<size_t>(std::ceil(std::sqrt(static_cast<double>(windows.size()))));
                size_t rows = (windows.size() + cols - 1) / cols;
                
                float winWidth = (availWidth - padding * (cols - 1)) / cols;
                float winHeight = (availHeight - padding * (rows - 1)) / rows;
                
                for (size_t i = 0; i < windows.size(); ++i) {
                    size_t row = i / cols;
                    size_t col = i % cols;
                    
                    float x = padding + col * (winWidth + padding);
                    float y = menuBarHeight + padding + row * (winHeight + padding);
                    
                    windows[i]->setPosition(x, y);
                    windows[i]->setSize(winWidth, winHeight);
                }
                break;
            }
            case LayoutPattern::Custom:
            default:
                // Do nothing for custom layout
                break;
        }
    }

    bool WindowManager::isCapturingMouse() const {
        if (!_enabled) return false;
        return !const_cast<WindowManager*>(this)->findWindowAt(_lastMouseX, _lastMouseY).empty();
    }

    bool WindowManager::isCapturingKeyboard() const {
        if (!_enabled) return false;
        return !_focusedWindowId.empty();
    }

    void WindowManager::setEnabled(bool enabled) {
        _enabled = enabled;
    }

    bool WindowManager::isEnabled() const {
        return _enabled;
    }

    void WindowManager::handleMouseClick(const rtype::ecs::events::MouseButtonPressedEvent& event) {
        if (!_enabled) return;
        if (event.button != rtype::ecs::events::MouseButton::Left) return;  // Left click only
        
        _mouseDown = true;
        float x = event.x;
        float y = event.y;
        
        // If modal is active, only allow clicks on modal window
        if (hasActiveModal()) {
            std::string clickedId = findWindowAt(x, y);
            if (clickedId != _modalWindowId) {
                // Block click - maybe flash modal window to indicate it needs attention
                return;
            }
        }
        
        // Find window at click position (search in reverse z-order)
        std::string clickedId = findWindowAt(x, y);
        
        if (!clickedId.empty()) {
            auto window = getWindow(clickedId);
            if (window) {
                // Focus this window
                if (!window->hasFlag(WindowFlags::NoFocusOnClick)) {
                    focusWindow(clickedId);
                }
                
                // Track this window for drag/resize interaction
                _activeInteractionWindowId = clickedId;
                
                // Pass click to window
                window->onMouseClick();
            }
        } else {
            _activeInteractionWindowId.clear();
        }
    }

    void WindowManager::handleMouseRelease(const rtype::ecs::events::MouseButtonReleasedEvent& event) {
        if (!_enabled) return;
        if (event.button != rtype::ecs::events::MouseButton::Left) return;
        
        _mouseDown = false;
        _isDragging = false;
        
        // Only notify the window that was being interacted with
        if (!_activeInteractionWindowId.empty()) {
            if (auto window = getWindow(_activeInteractionWindowId)) {
                window->handleMouseRelease();
            }
        }
        
        _activeInteractionWindowId.clear();
    }

    void WindowManager::handleMouseMove(const rtype::ecs::events::MouseMoveEvent& event) {
        if (!_enabled) return;
        
        _lastMouseX = event.x;
        _lastMouseY = event.y;
        
        // Update hover states for all windows
        for (auto& window : _windows) {
            if (window->isVisible()) {
                window->onMouseMove(event.x, event.y);
            }
        }
        
        // Handle dragging/resizing ONLY for the active interaction window
        if (_mouseDown && !_activeInteractionWindowId.empty()) {
            if (auto window = getWindow(_activeInteractionWindowId)) {
                window->handleMouseDrag(event.x, event.y, true);
            }
        }
    }

    void WindowManager::handleMouseWheel(const rtype::ecs::events::MouseWheelEvent& event) {
        if (!_enabled) return;
        
        // Send wheel to topmost window at mouse position
        std::string windowId = findWindowAt(_lastMouseX, _lastMouseY);
        if (!windowId.empty()) {
            // If modal is active, only allow scroll on modal window
            if (hasActiveModal() && windowId != _modalWindowId) {
                return;
            }
            if (auto window = getWindow(windowId)) {
                window->onMouseWheel(event.delta);
            }
        }
    }

    void WindowManager::handleKeyPress(const rtype::ecs::events::KeyPressedEvent& event) {
        if (!_enabled) return;
        
        // Check for window navigation shortcuts
        // Ctrl+Tab = next window, Ctrl+Shift+Tab = previous window
        bool ctrlDown = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL);
        bool shiftDown = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);
        
        if (ctrlDown && event.key == rtype::ecs::events::KeyCode::Tab) {
            if (shiftDown) {
                focusPrevWindow();
            } else {
                focusNextWindow();
            }
        }
        
        // Ctrl+W = close current window
        if (ctrlDown && event.key == rtype::ecs::events::KeyCode::W) {
            if (!_focusedWindowId.empty()) {
                if (hasActiveModal() && _focusedWindowId == _modalWindowId) {
                    closeModal();
                } else {
                    hideWindow(_focusedWindowId);
                }
            }
        }
    }

    std::string WindowManager::findWindowAt(float x, float y) {
        // Search in reverse order (front to back)
        // Check always-on-top layer first
        for (auto rit = _alwaysOnTopLayer.rbegin(); rit != _alwaysOnTopLayer.rend(); ++rit) {
            if (!(*rit)->isVisible()) continue;
            
            auto transform = (*rit)->getAbsoluteTransform();
            float height = (*rit)->isCollapsed() ? WindowWidget::TITLE_BAR_HEIGHT : transform.height;
            
            if (x >= transform.x && x < transform.x + transform.width &&
                y >= transform.y && y < transform.y + height) {
                // Use optimized reverse lookup
                auto idIt = _windowIdMap.find((*rit).get());
                if (idIt != _windowIdMap.end()) {
                    return idIt->second;
                }
            }
        }
        
        // Then check normal layer
        for (auto rit = _normalLayer.rbegin(); rit != _normalLayer.rend(); ++rit) {
            if (!(*rit)->isVisible()) continue;
            
            auto transform = (*rit)->getAbsoluteTransform();
            float height = (*rit)->isCollapsed() ? WindowWidget::TITLE_BAR_HEIGHT : transform.height;
            
            if (x >= transform.x && x < transform.x + transform.width &&
                y >= transform.y && y < transform.y + height) {
                // Use optimized reverse lookup
                auto idIt = _windowIdMap.find((*rit).get());
                if (idIt != _windowIdMap.end()) {
                    return idIt->second;
                }
            }
        }
        
        return "";
    }

    void WindowManager::bringToFront(const std::string& id) {
        auto it = _windowMap.find(id);
        if (it == _windowMap.end()) return;
        
        auto window = it->second;
        
        // Determine which layer the window is in
        std::vector<std::shared_ptr<WindowWidget>>* layer;
        if (window->hasFlag(WindowFlags::AlwaysOnTop)) {
            layer = &_alwaysOnTopLayer;
        } else {
            layer = &_normalLayer;
        }
        
        // Don't reorder if already at front of its layer
        if (!layer->empty() && layer->back() == window) return;
        
        // Remove from current position in layer
        layer->erase(
            std::remove(layer->begin(), layer->end(), window),
            layer->end()
        );
        
        // Add to end (front) of layer
        layer->push_back(window);
        
        // Rebuild combined list
        rebuildWindowList();
    }

    void WindowManager::rebuildWindowList() {
        _windows.clear();
        _windows.reserve(_normalLayer.size() + _alwaysOnTopLayer.size());
        
        // Add normal layer first (back)
        for (const auto& win : _normalLayer) {
            _windows.push_back(win);
        }
        
        // Add always-on-top layer last (front)
        for (const auto& win : _alwaysOnTopLayer) {
            _windows.push_back(win);
        }
    }

    std::vector<std::string> WindowManager::getVisibleWindowIds() const {
        std::vector<std::string> ids;
        ids.reserve(_windows.size());
        
        for (const auto& win : _windows) {
            if (win->isVisible()) {
                auto idIt = _windowIdMap.find(win.get());
                if (idIt != _windowIdMap.end()) {
                    ids.push_back(idIt->second);
                }
            }
        }
        
        return ids;
    }

    void WindowManager::renderModalOverlay() const {
        // Draw semi-transparent overlay behind modal window
        DrawRectangle(0, 0, _screenWidth, _screenHeight, {0, 0, 0, 128});
    }

} // namespace rtype::ui
