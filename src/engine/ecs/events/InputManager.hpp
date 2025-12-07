/*
** R-Type ECS - Input Manager
** Platform-agnostic input polling and event dispatching
*/

#pragma once

#include "engine/ecs/core/EventBus.hpp"
#include "InputEvents.hpp"

#include <raylib.h>
#include <unordered_map>

namespace rtype::ecs::events {

    /**
     * @brief Polls platform input and dispatches events via EventBus
     * 
     * Call pollInput() exactly once per frame, before any game logic.
     * This ensures:
     * - Consistent input state across fixed-timestep iterations
     * - IsKeyPressed events aren't lost
     * - Platform-specific code is isolated here
     */
    class InputManager {
    public:
        /**
         * @brief Construct a new Input Manager object
         * @param eventBus Reference to the EventBus for dispatching events
         */
        InputManager(EventBus& eventBus) : m_eventBus(eventBus) {
            initKeyMap();
        }

        /**
         * @brief Poll all input and dispatch events (call once per frame)
         */
        void pollInput() {
            pollKeyboard();
            pollMouse();
        }

        /**
         * @brief Get current key state (for continuous input like movement)
         * @return KeyState reference
         */
        const KeyState& getKeyState() const { return m_keyState; }

        /**
         * @brief Get current mouse state
         * @return MouseState reference
         */
        const MouseState& getMouseState() const { return m_mouseState; }

        /**
         * @brief Check if a specific key is currently held
         * @param key The KeyCode to check
         * @return true if key is down
         */
        bool isKeyDown(KeyCode key) const {
            auto it = m_keyDown.find(key);
            return it != m_keyDown.end() && it->second;
        }

        /**
         * @brief Check if a key was pressed this frame
         * @param key The KeyCode to check
         * @return true if key was pressed this frame
         */
        bool wasKeyPressed(KeyCode key) const {
            auto it = m_keyPressed.find(key);
            return it != m_keyPressed.end() && it->second;
        }

    private:
        /**
         * @brief Reference to the EventBus for dispatching events
         */
        EventBus& m_eventBus;
        /**
         * @brief Current input states
         */
        KeyState m_keyState;
        /**
         * @brief Current mouse state
         */
        MouseState m_mouseState;

        /**
         * @brief Key state tracking (held and pressed states
         */
        std::unordered_map<KeyCode, bool> m_keyDown;
        /**
         * @brief Keys pressed this frame
         */
        std::unordered_map<KeyCode, bool> m_keyPressed;
        /**
         * @brief Mapping from KeyCode to raylib key codes
         */
        std::unordered_map<KeyCode, int> m_keyToRaylib;

        /**
         * @brief Initialize the key mapping between KeyCode and raylib keys
         */
        void initKeyMap() {
            // Map our KeyCode to raylib keys
            m_keyToRaylib[KeyCode::Up] = KEY_UP;
            m_keyToRaylib[KeyCode::Down] = KEY_DOWN;
            m_keyToRaylib[KeyCode::Left] = KEY_LEFT;
            m_keyToRaylib[KeyCode::Right] = KEY_RIGHT;
            m_keyToRaylib[KeyCode::W] = KEY_W;
            m_keyToRaylib[KeyCode::A] = KEY_A;
            m_keyToRaylib[KeyCode::S] = KEY_S;
            m_keyToRaylib[KeyCode::D] = KEY_D;
            m_keyToRaylib[KeyCode::Space] = KEY_SPACE;
            m_keyToRaylib[KeyCode::Enter] = KEY_ENTER;
            m_keyToRaylib[KeyCode::Escape] = KEY_ESCAPE;
            m_keyToRaylib[KeyCode::Tab] = KEY_TAB;
            m_keyToRaylib[KeyCode::Backspace] = KEY_BACKSPACE;
            m_keyToRaylib[KeyCode::G] = KEY_G;
            m_keyToRaylib[KeyCode::O] = KEY_O;
            m_keyToRaylib[KeyCode::C] = KEY_C;
            m_keyToRaylib[KeyCode::P] = KEY_P;
            m_keyToRaylib[KeyCode::LeftShift] = KEY_LEFT_SHIFT;
            m_keyToRaylib[KeyCode::RightShift] = KEY_RIGHT_SHIFT;
            m_keyToRaylib[KeyCode::LeftControl] = KEY_LEFT_CONTROL;
            m_keyToRaylib[KeyCode::RightControl] = KEY_RIGHT_CONTROL;
            m_keyToRaylib[KeyCode::LeftAlt] = KEY_LEFT_ALT;
            m_keyToRaylib[KeyCode::RightAlt] = KEY_RIGHT_ALT;
            m_keyToRaylib[KeyCode::Equal] = KEY_EQUAL;
            m_keyToRaylib[KeyCode::Minus] = KEY_MINUS;
            m_keyToRaylib[KeyCode::Num1] = KEY_ONE;
            m_keyToRaylib[KeyCode::Num2] = KEY_TWO;
            m_keyToRaylib[KeyCode::Num3] = KEY_THREE;
            m_keyToRaylib[KeyCode::Num4] = KEY_FOUR;
            m_keyToRaylib[KeyCode::Num5] = KEY_FIVE;
            m_keyToRaylib[KeyCode::Num6] = KEY_SIX;
            m_keyToRaylib[KeyCode::Num7] = KEY_SEVEN;
            m_keyToRaylib[KeyCode::Num8] = KEY_EIGHT;
            m_keyToRaylib[KeyCode::Num9] = KEY_NINE;
            m_keyToRaylib[KeyCode::Num0] = KEY_ZERO;
            m_keyToRaylib[KeyCode::F1] = KEY_F1;
            m_keyToRaylib[KeyCode::F2] = KEY_F2;
            m_keyToRaylib[KeyCode::F3] = KEY_F3;
        }

        /**
         * @brief Poll keyboard state and emit events
         */
        void pollKeyboard() {
            m_keyPressed.clear();

            for (const auto& [keyCode, raylibKey] : m_keyToRaylib) {
                bool wasDown = m_keyDown[keyCode];
                bool isDown = IsKeyDown(raylibKey);
                
                m_keyDown[keyCode] = isDown;

                if (isDown && !wasDown) {
                    m_keyPressed[keyCode] = true;
                    m_eventBus.emit(KeyPressedEvent{keyCode});
                }
                else if (!isDown && wasDown) {
                    m_eventBus.emit(KeyReleasedEvent{keyCode});
                }
            }

            m_keyState.up = m_keyDown[KeyCode::Up];
            m_keyState.down = m_keyDown[KeyCode::Down];
            m_keyState.left = m_keyDown[KeyCode::Left];
            m_keyState.right = m_keyDown[KeyCode::Right];
            m_keyState.w = m_keyDown[KeyCode::W];
            m_keyState.a = m_keyDown[KeyCode::A];
            m_keyState.s = m_keyDown[KeyCode::S];
            m_keyState.d = m_keyDown[KeyCode::D];
            m_keyState.space = m_keyDown[KeyCode::Space];
            m_keyState.enter = m_keyDown[KeyCode::Enter];
            m_keyState.escape = m_keyDown[KeyCode::Escape];
            m_keyState.tab = m_keyDown[KeyCode::Tab];
            m_keyState.shift = m_keyDown[KeyCode::LeftShift] || m_keyDown[KeyCode::RightShift];
            m_keyState.ctrl = m_keyDown[KeyCode::LeftControl] || m_keyDown[KeyCode::RightControl];
            m_keyState.alt = m_keyDown[KeyCode::LeftAlt] || m_keyDown[KeyCode::RightAlt];

            m_eventBus.emit(KeyStateEvent{m_keyState});
        }

        /**
         * @brief Poll mouse state and emit events
         */
        void pollMouse() {
            float newX = static_cast<float>(GetMouseX());
            float newY = static_cast<float>(GetMouseY());

            m_mouseState.deltaX = newX - m_mouseState.x;
            m_mouseState.deltaY = newY - m_mouseState.y;
            m_mouseState.x = newX;
            m_mouseState.y = newY;

            if (m_mouseState.deltaX != 0 || m_mouseState.deltaY != 0) {
                m_eventBus.emit(MouseMoveEvent{
                    m_mouseState.x, m_mouseState.y,
                    m_mouseState.deltaX, m_mouseState.deltaY
                });
            }

            bool leftWasDown = m_mouseState.leftDown;
            bool rightWasDown = m_mouseState.rightDown;
            bool middleWasDown = m_mouseState.middleDown;

            m_mouseState.leftDown = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
            m_mouseState.rightDown = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
            m_mouseState.middleDown = IsMouseButtonDown(MOUSE_MIDDLE_BUTTON);

            if (m_mouseState.leftDown && !leftWasDown) {
                m_eventBus.emit(MouseButtonPressedEvent{MouseButton::Left, m_mouseState.x, m_mouseState.y});
            } else if (!m_mouseState.leftDown && leftWasDown) {
                m_eventBus.emit(MouseButtonReleasedEvent{MouseButton::Left, m_mouseState.x, m_mouseState.y});
            }

            if (m_mouseState.rightDown && !rightWasDown) {
                m_eventBus.emit(MouseButtonPressedEvent{MouseButton::Right, m_mouseState.x, m_mouseState.y});
            } else if (!m_mouseState.rightDown && rightWasDown) {
                m_eventBus.emit(MouseButtonReleasedEvent{MouseButton::Right, m_mouseState.x, m_mouseState.y});
            }

            if (m_mouseState.middleDown && !middleWasDown) {
                m_eventBus.emit(MouseButtonPressedEvent{MouseButton::Middle, m_mouseState.x, m_mouseState.y});
            } else if (!m_mouseState.middleDown && middleWasDown) {
                m_eventBus.emit(MouseButtonReleasedEvent{MouseButton::Middle, m_mouseState.x, m_mouseState.y});
            }

            float wheel = GetMouseWheelMove();
            if (wheel != 0) {
                m_mouseState.wheelDelta = wheel;
                m_eventBus.emit(MouseWheelEvent{wheel});
            } else {
                m_mouseState.wheelDelta = 0;
            }
        }
    };

} // namespace rtype::ecs::events
