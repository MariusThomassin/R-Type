/*
** R-Type ECS - Input Events
** Event types for input handling
*/

#pragma once

#include <cstdint>

namespace rtype::ecs::events {

    /**
     * @brief Key codes (platform-independent)
     */
    enum class KeyCode : int {
        // Movement
        Up = 0, Down, Left, Right,
        W, A, S, D,
        
        // Actions  
        Space, Enter, Escape,
        Tab, Backspace,
        
        // Function keys
        F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        
        // Letters
        B, C, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, T, U, V, X, Y, Z,
        
        // Numbers
        Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
        
        // Modifiers
        LeftShift, RightShift, LeftControl, RightControl, LeftAlt, RightAlt,
        
        // Special
        Equal, Minus,
        
        COUNT
    };

    /**
     * @brief Mouse button codes
     */
    enum class MouseButton : int {
        Left = 0,
        Right,
        Middle,
        COUNT
    };

    /**
     * @brief Key state (held keys)
     */
    struct KeyState {
        bool up = false, down = false, left = false, right = false;
        bool w = false, a = false, s = false, d = false;
        bool space = false, enter = false, escape = false;
        bool tab = false;
        bool shift = false, ctrl = false, alt = false;
        
        // Convenience methods
        bool moveUp() const { return up || w; }
        bool moveDown() const { return down || s; }
        bool moveLeft() const { return left || a; }
        bool moveRight() const { return right || d; }
    };

    /**
     * @brief Mouse state
     */
    struct MouseState {
        float x = 0, y = 0;
        float deltaX = 0, deltaY = 0;
        float wheelDelta = 0;
        bool leftDown = false, rightDown = false, middleDown = false;
    };

    // ==================== Events ====================

    /**
     * @brief Emitted when a key is pressed (single frame)
     */
    struct KeyPressedEvent {
        KeyCode key;
    };

    /**
     * @brief Emitted when a key is released
     */
    struct KeyReleasedEvent {
        KeyCode key;
    };

    /**
     * @brief Emitted every frame with current key state
     */
    struct KeyStateEvent {
        KeyState state;
    };

    /**
     * @brief Emitted when mouse moves
     */
    struct MouseMoveEvent {
        float x, y;
        float deltaX, deltaY;
    };

    /**
     * @brief Emitted when mouse button is pressed
     */
    struct MouseButtonPressedEvent {
        MouseButton button;
        float x, y;
    };

    /**
     * @brief Emitted when mouse button is released
     */
    struct MouseButtonReleasedEvent {
        MouseButton button;
        float x, y;
    };

    /**
     * @brief Emitted when mouse wheel scrolls
     */
    struct MouseWheelEvent {
        float delta;
    };

    // ==================== Game Events ====================

    /**
     * @brief Player wants to shoot
     */
    struct ShootEvent {
        uint64_t shooterId;
    };

    /**
     * @brief Spawn danmaku pattern
     */
    struct DanmakuEvent {
        float x, y;
    };

    /**
     * @brief Toggle debug mode
     */
    struct DebugToggleEvent {};

    /**
     * @brief Switch debug tab
     */
    struct DebugTabEvent {
        int tabIndex;
    };

    /**
     * @brief Start showoff/demonstration mode
     */
    struct ShowoffStartEvent {};

    /**
     * @brief End showoff/demonstration mode
     */
    struct ShowoffEndEvent {};

} // namespace rtype::ecs::events
