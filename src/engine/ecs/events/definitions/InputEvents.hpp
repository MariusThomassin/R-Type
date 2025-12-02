/*
** R-Type ECS - Input Events
** Events for input handling
*/

#pragma once

#include "../../core/EntityTypes.hpp"

namespace rtype::ecs::events {

    /**
     * @brief A key was pressed
     */
    struct KeyPressed {
        int keyCode;
        bool shift = false;
        bool ctrl = false;
        bool alt = false;
    };

    /**
     * @brief A key was released
     */
    struct KeyReleased {
        int keyCode;
    };

    /**
     * @brief A key is being held
     */
    struct KeyHeld {
        int keyCode;
        float duration;  // seconds held
    };

    /**
     * @brief Mouse button pressed
     */
    struct MousePressed {
        int button;
        float x;
        float y;
    };

    /**
     * @brief Mouse button released
     */
    struct MouseReleased {
        int button;
        float x;
        float y;
    };

    /**
     * @brief Mouse moved
     */
    struct MouseMoved {
        float x;
        float y;
        float deltaX;
        float deltaY;
    };

    /**
     * @brief Mouse wheel scrolled
     */
    struct MouseScrolled {
        float scrollX;
        float scrollY;
    };

    /**
     * @brief Gamepad connected/disconnected
     */
    struct GamepadConnectionChanged {
        int gamepadId;
        bool connected;
    };

    /**
     * @brief Gamepad button pressed
     */
    struct GamepadButtonPressed {
        int gamepadId;
        int button;
    };

    /**
     * @brief Gamepad button released
     */
    struct GamepadButtonReleased {
        int gamepadId;
        int button;
    };

    /**
     * @brief Gamepad axis moved
     */
    struct GamepadAxisMoved {
        int gamepadId;
        int axis;
        float value;  // -1.0 to 1.0
    };

    /**
     * @brief High-level action event (mapped from input)
     */
    struct InputAction {
        enum class Action {
            MoveUp, MoveDown, MoveLeft, MoveRight,
            Fire, AltFire, Boost, Pause,
            MenuUp, MenuDown, MenuSelect, MenuBack
        } action;

        EntityId player = 0;  // Which player triggered this
        bool pressed;         // true = started, false = stopped
        float value = 1.0f;   // Analog value for gradual actions
    };

} // namespace rtype::ecs::events
