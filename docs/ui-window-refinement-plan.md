# Window UI Library Refinement Plan

## Current Issues Analysis

### 1. Architectural Issues

| Issue | Location | Severity | Description |
|-------|----------|----------|-------------|
| **Content Drawer Anti-Pattern** | `WindowedDebugSystem.cpp` | High | Uses raw drawing callbacks instead of Widget child system, bypassing the UI framework's benefits |
| **Dual Manager Problem** | `WindowManager.hpp` + `UIManager.hpp` | Medium | Two separate managers for UI; WindowManager duplicates event handling logic already in UIManager |
| **Inefficient Window Lookup** | `WindowManager.cpp:299-316` | Medium | Linear search through `_windows` + reverse map lookup in `findWindowAt()` and `removeWindow()` |

### 2. Missing Features

| Feature | Priority | Impact |
|---------|----------|--------|
| **Modal Windows** | High | `WindowFlags::Modal` exists but unimplemented |
| **Cursor Management** | High | Resize edges don't change cursor (line 157: "Could set cursor here") |
| **Window State Persistence** | Medium | Positions/size lost between sessions |
| **Interactive Scrollbar** | Medium | Scrollbar is visual only; no drag-to-scroll |
| **Window Minimization** | Low | Only collapse (to title bar) exists |
| **Always-On-Top Layering** | Low | `WindowFlags::AlwaysOnTop` flag exists but unused |
| **Tab Navigation** | Low | No keyboard cycling between windows |

### 3. Design Issues

| Issue | Location | Problem |
|-------|----------|---------|
| **Hardcoded Window Positions** | `WindowedDebugSystem.cpp:64-113` | Windows created at fixed coordinates; no auto-layout |
| **No Content Widget System** | `WindowWidget.cpp:439-442` | `renderContent()` is empty; relies on external callbacks |
| **Tight Coupling** | `WindowedDebugSystem` | Direct Raylib drawing calls mixed with UI logic |
| **Inconsistent Scissor Management** | `WindowedDebugSystem.cpp:174-182` | Manual scissor mode per window; should be automatic |
| **No Animation Support** | `WindowWidget` | Open/close/resize are instant |

---

## Implementation Plan

### Phase 1: Core Architecture Fixes

#### 1.1 Integrate WindowManager with UIManager

**Problem:** WindowManager duplicates EventBus subscription and event handling logic that UIManager already provides.

**Solution:** Make WindowManager use UIManager for widget management instead of maintaining its own window list.

```cpp
// New approach: WindowManager becomes a UI "layer manager"
class WindowManager {
    // Instead of:
    // std::vector<std::shared_ptr<WindowWidget>> _windows;
    // std::unordered_map<std::string, std::shared_ptr<WindowWidget>> _windowMap;

    // Use UIManager's widget system:
    UIManager& _uiManager;
    std::unordered_map<std::string, std::weak_ptr<WindowWidget>> _windowMap;

    // Focus management becomes a layer concern
    std::string _focusedWindowId;
    std::vector<std::string> _zOrder;  // Window IDs in z-order
};
```

**Benefits:**
- Single source of truth for event routing
- Automatic hit-testing through UIManager
- Eliminates duplicate event subscriptions
- Windows participate in normal UI hierarchy

---

#### 1.2 Implement Content Widget System

**Problem:** `WindowedDebugSystem` uses raw drawing callbacks instead of the Widget child system.

**Solution:** Add `setContentWidget()` to WindowWidget and use child widgets for content.

```cpp
class WindowWidget : public Widget {
public:
    void setContentWidget(std::shared_ptr<Widget> content);
    std::shared_ptr<Widget> getContentWidget() const;

    // Content drawer becomes optional fallback
    void setContentDrawer(WindowContentDrawer drawer);

protected:
    void renderContent() const override {
        if (_contentWidget) {
            _contentWidget->render();
        } else if (_contentDrawer) {
            _contentDrawer(...);
        }
    }

private:
    std::shared_ptr<Widget> _contentWidget;
    WindowContentDrawer _contentDrawer;
};
```

**Benefits:**
- Content widgets get automatic event handling
- Scissor mode handled by WindowWidget render
- Content can use normal UI widgets (buttons, text, etc.)
- Preserves callback flexibility for debug use-cases

---

### Phase 2: Critical Features

#### 2.1 Modal Window Support

**Files:** `WindowManager.hpp/cpp`, `WindowWidget.hpp/cpp`

```cpp
class WindowManager {
public:
    // Modal API
    void showModal(std::shared_ptr<WindowWidget> modal, std::string blockerId = "");
    void closeModal();
    std::shared_ptr<WindowWidget> getModalWindow() const;

private:
    std::shared_ptr<WindowWidget> _modalWindow;
    std::string _blockerWindowId;  // Window that triggered the modal

    // Modal prevents interaction with other windows
    bool isInputBlocked() const;
};
```

**Implementation:**
- Modal window rendered on top of all windows
- All input events blocked except to modal window
- Dim/blocker background behind modal
- Auto-close on blocker window destruction

---

#### 2.2 Cursor Management

**Files:** `WindowWidget.cpp`, `WindowManager.cpp`

```cpp
enum class CursorType {
    Default,
    Arrow,
    IBeam,
    ResizeH,
    ResizeV,
    ResizeNWSE,
    ResizeNESW,
    Hand,
    Pointer
};

class WindowWidget {
protected:
    CursorType getCursorForPosition(float x, float y) const;

    // In isOnResizeEdge, also set suggested cursor
    bool isOnResizeEdge(float x, float y, int& edgeX, int& edgeY, CursorType& cursor) const;
};

class WindowManager {
public:
    CursorType getSuggestedCursor() const;
    void updateCursor();

private:
    CursorType _currentCursor = CursorType::Default;
};
```

**Implementation:**
- Each window suggests cursor based on mouse position
- WindowManager updates global cursor each frame
- Caches cursor state to avoid redundant Raylib calls

---

#### 2.3 Interactive Scrollbar

**Files:** `WindowWidget.cpp`

```cpp
class WindowWidget {
private:
    bool _scrollbarHovered = false;
    bool _scrollbarDragging = false;
    float _scrollbarDragOffset = 0;

public:
    bool onMouseClick() override {
        // ... existing code ...

        if (_scrollbarHovered) {
            _scrollbarDragging = true;
            float thumbY = getScrollbarThumbY();
            _scrollbarDragOffset = _lastMouseY - thumbY;
            return true;
        }
    }

    void handleMouseDrag(float x, float y, bool leftDown) override {
        // ... existing code ...

        if (_scrollbarDragging) {
            float thumbY = y - _scrollbarDragOffset;
            float scrollRatio = (thumbY - scrollbarY) / (scrollbarHeight - thumbHeight);
            float maxScroll = _contentHeight - getContentBounds().height;
            _scrollOffset = scrollRatio * maxScroll;
            setScrollOffset(_scrollOffset);
        }
    }

private:
    bool isInScrollbar(float x, float y) const;
    float getScrollbarThumbY() const;
};
```

---

### Phase 3: Quality of Life

#### 3.1 Window State Persistence

**Files:** New `WindowSerializer.hpp/cpp`

```cpp
struct WindowState {
    std::string id;
    float x, y, width, height;
    bool visible, collapsed, focused;
    float scrollOffset;
};

class WindowManager {
public:
    void saveStates(const std::string& filepath);
    void loadStates(const std::string& filepath);

private:
    std::vector<WindowState> captureStates();
    void restoreStates(const std::vector<WindowState>& states);
};
```

**Format:** JSON for human editability

---

#### 3.2 Auto-Layout System

**Files:** New `WindowLayout.hpp/cpp`

```cpp
enum class LayoutPattern {
    Cascade,        // Windows offset from each other
    TileHorizontal, // Side by side
    TileVertical,   // Stacked
    Grid,           // N x M grid
    Custom
};

class WindowManager {
public:
    void layoutWindows(LayoutPattern pattern);
    void arrangeWindows(const std::vector<std::string>& windowIds, LayoutPattern pattern);
};
```

---

#### 3.3 Always-On-Top Layers

**Files:** `WindowManager.hpp/cpp`

```cpp
class WindowManager {
private:
    // Separate z-order lists for layers
    std::vector<std::string> _normalLayer;
    std::vector<std::string> _alwaysOnTopLayer;

    void render() const {
        // Render normal layer, then always-on-top layer
        for (const auto& id : _normalLayer) { /* ... */ }
        for (const auto& id : _alwaysOnTopLayer) { /* ... */ }
    }

    std::string findWindowAt(float x, float y) {
        // Search always-on-top first, then normal
        auto result = searchLayer(_alwaysOnTopLayer, x, y);
        if (!result.empty()) return result;
        return searchLayer(_normalLayer, x, y);
    }
};
```

---

### Phase 4: Polish

#### 4.1 Animation System

**Files:** New `WindowAnimator.hpp/cpp`

```cpp
enum class AnimType {
    Open,
    Close,
    Collapse,
    Expand,
    Move,
    Resize
};

class WindowAnimator {
public:
    void animate(std::shared_ptr<WindowWidget> window, AnimType type, float duration);
    void update(float dt);
    bool isAnimating(const std::string& windowId) const;

private:
    struct ActiveAnimation {
        std::weak_ptr<WindowWidget> window;
        AnimType type;
        float elapsed;
        float duration;
        // Start/end states
    };
    std::vector<ActiveAnimation> _animations;
};
```

**Easing functions:** Linear, EaseOutQuad, EaseInOutCubic

---

#### 4.2 Minimize to Tray

**Files:** `WindowWidget.hpp/cpp`

```cpp
enum class WindowState {
    Normal,
    Collapsed,  // Title bar only
    Minimized,  // Hidden, shown in tray
    Maximized
};

class WindowWidget {
public:
    WindowState getWindowState() const;
    void setWindowState(WindowState state);
    void minimize();
    void restore();
};
```

**Tray widget:** Small buttons at screen edge to show minimized windows

---

#### 4.3 Keyboard Navigation

**Files:** `WindowManager.cpp`

```cpp
class WindowManager {
private:
    size_t _focusIndex = 0;

public:
    void focusNextWindow();
    void focusPrevWindow();
    void setFocusIndex(size_t index);
};
```

**Key bindings:**
- `Ctrl+Tab`: Next window
- `Ctrl+Shift+Tab`: Previous window
- `Ctrl+W`: Close current window

---

## Migration Strategy

### Step 1: Refactor WindowManager (Breaking Change)
- Integrate with UIManager
- Update all call sites

### Step 2: Update WindowedDebugSystem
- Convert content drawers to content widgets where applicable
- Keep drawer API for complex debug views

### Step 3: Add Features Incrementally
- Modal support
- Cursor management
- Interactive scrollbar

### Step 4: Polish
- Animations
- Persistence
- Auto-layout

---

## File Structure Changes

```
src/engine/ui/
├── WindowManager.{hpp,cpp}          (Refactored)
├── widgets/
│   └── WindowWidget.{hpp,cpp}       (Enhanced)
├── layout/
│   ├── WindowLayout.hpp             (New)
│   └── LayoutPatterns.hpp           (New)
├── animation/
│   └── WindowAnimator.{hpp,cpp}     (New)
└── serialization/
    └── WindowSerializer.{hpp,cpp}   (New)
```

---

## Testing Strategy

1. **Unit Tests**: Widget interaction, layout calculations
2. **Integration Tests**: Multi-window scenarios, modal blocking
3. **Visual Tests**: Screenshot comparison for rendering
4. **Stress Tests**: 100+ windows, rapid open/close

---

## Success Metrics

- [ ] WindowManager integration reduces event handling code by 40%
- [ ] Modal windows block input correctly
- [ ] Cursor changes on resize edges
- [ ] Content widgets work without manual scissor management
- [ ] Window positions persist across restarts
- [ ] Auto-layout prevents window overlap
