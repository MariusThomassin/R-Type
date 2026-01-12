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
     * 
     * Includes both raw key states and action-based flags.
     * The action flags (actionUp, actionShoot, etc.) are set based on 
     * configurable key bindings from SettingsManager.
     */
    struct KeyState {
        // Raw key states (for legacy compatibility)
        bool up = false, down = false, left = false, right = false;
        bool w = false, a = false, s = false, d = false;
        bool space = false, enter = false, escape = false;
        bool tab = false;
        bool shift = false, ctrl = false, alt = false;

        // Action-based flags (set from key bindings)
        bool actionUp = false;
        bool actionDown = false;
        bool actionLeft = false;
        bool actionRight = false;
        bool actionShoot = false;
        bool actionBomb = false;
        bool actionOrbSwitch = false;
        bool actionPause = false;
        bool actionConfirm = false;
        bool actionCancel = false;
        
        // Convenience methods - use action flags which respect bindings
        bool moveUp() const { return actionUp; }
        bool moveDown() const { return actionDown; }
        bool moveLeft() const { return actionLeft; }
        bool moveRight() const { return actionRight; }
        bool shoot() const { return actionShoot; }
        bool bomb() const { return actionBomb; }
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

    /**
     * @brief Toggle stress test mode (Shift+P)
     */
    struct StressTestToggleEvent {};

    // ==================== Gamepad Events ====================

    /**
     * @brief Gamepad button IDs (platform-independent)
     */
    enum class GamepadButton : int {
        Unknown = -1,
        // Face buttons
        FaceDown = 0,   // A on Xbox, Cross on PS
        FaceRight = 1,  // B on Xbox, Circle on PS
        FaceLeft = 2,   // X on Xbox, Square on PS
        FaceUp = 3,     // Y on Xbox, Triangle on PS
        // Shoulder buttons
        LeftBumper = 4,
        RightBumper = 5,
        // Center buttons
        Back = 6,
        Start = 7,
        Guide = 8,
        // Stick buttons
        LeftThumb = 9,
        RightThumb = 10,
        // D-pad
        DpadUp = 11,
        DpadRight = 12,
        DpadDown = 13,
        DpadLeft = 14,
        COUNT
    };

    /**
     * @brief Gamepad axis IDs
     */
    enum class GamepadAxis : int {
        LeftX = 0,
        LeftY = 1,
        RightX = 2,
        RightY = 3,
        LeftTrigger = 4,
        RightTrigger = 5,
        COUNT
    };

    /**
     * @brief Current gamepad state
     */
    struct GamepadState {
        bool connected = false;
        int gamepadId = 0;
        float leftStickX = 0.0f;
        float leftStickY = 0.0f;
        float rightStickX = 0.0f;
        float rightStickY = 0.0f;
        float leftTrigger = 0.0f;
        float rightTrigger = 0.0f;
        bool buttons[static_cast<int>(GamepadButton::COUNT)] = {false};
    };

    /**
     * @brief Gamepad button pressed event
     */
    struct GamepadButtonPressedEvent {
        int gamepadId;
        GamepadButton button;
    };

    /**
     * @brief Gamepad button released event
     */
    struct GamepadButtonReleasedEvent {
        int gamepadId;
        GamepadButton button;
    };

    /**
     * @brief Gamepad axis changed event
     */
    struct GamepadAxisEvent {
        int gamepadId;
        GamepadAxis axis;
        float value;
    };

    /**
     * @brief Gamepad connected/disconnected event
     */
    struct GamepadConnectedEvent {
        int gamepadId;
        bool connected;
    };

    /**
     * @brief Gamepad state event (emitted every frame)
     */
    struct GamepadStateEvent {
        GamepadState state;
    };

    // ==================== Game Actions ====================

    /**
     * @brief Game action types (high-level input abstraction)
     */
    enum class GameAction {
        MoveUp,
        MoveDown,
        MoveLeft,
        MoveRight,
        Shoot,
        Bomb,
        SwitchOrbSide,
        Pause,
        Confirm,
        Cancel,
        COUNT
    };

    /**
     * @brief Game action event
     */
    struct GameActionEvent {
        GameAction action;
        bool pressed;        // true = started, false = released
        float value = 1.0f;  // Analog value (0-1 for triggers/sticks)
    };

} // namespace rtype::ecs::events
