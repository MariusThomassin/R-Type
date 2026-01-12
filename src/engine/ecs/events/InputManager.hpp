/*
** R-Type ECS - Input Manager
** Platform-agnostic input polling and event dispatching
*/

#pragma once

#include "engine/ecs/core/EventBus.hpp"
#include "InputEvents.hpp"
#include "shared/SettingsManager.hpp"

#include <raylib.h>
#include <unordered_map>
#include <cmath>

namespace rtype::ecs::events {

    /**
     * @brief Polls platform input and dispatches events via EventBus
     * 
     * Call pollInput() exactly once per frame, before any game logic.
     * This ensures:
     * - Consistent input state across fixed-timestep iterations
     * - IsKeyPressed events aren't lost
     * - Platform-specific code is isolated here
     * 
     * Supports configurable key bindings via SettingsManager.
     */
    class InputManager {
    public:
        /**
         * @brief Construct a new Input Manager object
         * @param eventBus Reference to the EventBus for dispatching events
         * @param settings Optional pointer to SettingsManager for key bindings
         */
        InputManager(EventBus& eventBus, rtype::SettingsManager* settings = nullptr) 
            : m_eventBus(eventBus), m_settings(settings) {
            initKeyMap();
        }

        /**
         * @brief Set or update the SettingsManager reference
         * @param settings Pointer to SettingsManager (can be null for defaults)
         */
        void setSettings(rtype::SettingsManager* settings) {
            m_settings = settings;
        }

        /**
         * @brief Poll all input and dispatch events (call once per frame)
         */
        void pollInput() {
            pollKeyboard();
            pollMouse();
            pollGamepad();
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

        /**
         * @brief Get current gamepad state
         * @param gamepadId The gamepad index (0-3)
         * @return GamepadState reference
         */
        const GamepadState& getGamepadState(int gamepadId = 0) const {
            if (gamepadId >= 0 && gamepadId < MAX_GAMEPADS) {
                return m_gamepadStates[gamepadId];
            }
            return m_gamepadStates[0];
        }

        /**
         * @brief Check if a gamepad is connected
         * @param gamepadId The gamepad index (0-3)
         * @return true if connected
         */
        bool isGamepadConnected(int gamepadId = 0) const {
            if (gamepadId >= 0 && gamepadId < MAX_GAMEPADS) {
                return m_gamepadStates[gamepadId].connected;
            }
            return false;
        }

        /**
         * @brief Check if a gamepad button is currently held
         * @param button The button to check
         * @param gamepadId The gamepad index
         * @return true if button is down
         */
        bool isGamepadButtonDown(GamepadButton button, int gamepadId = 0) const {
            if (gamepadId >= 0 && gamepadId < MAX_GAMEPADS) {
                int idx = static_cast<int>(button);
                if (idx >= 0 && idx < static_cast<int>(GamepadButton::COUNT)) {
                    return m_gamepadStates[gamepadId].buttons[idx];
                }
            }
            return false;
        }

        /**
         * @brief Get gamepad axis value
         * @param axis The axis to read
         * @param gamepadId The gamepad index
         * @return Axis value (-1 to 1)
         */
        float getGamepadAxis(GamepadAxis axis, int gamepadId = 0) const {
            if (gamepadId >= 0 && gamepadId < MAX_GAMEPADS) {
                const auto& state = m_gamepadStates[gamepadId];
                switch (axis) {
                    case GamepadAxis::LeftX: return state.leftStickX;
                    case GamepadAxis::LeftY: return state.leftStickY;
                    case GamepadAxis::RightX: return state.rightStickX;
                    case GamepadAxis::RightY: return state.rightStickY;
                    case GamepadAxis::LeftTrigger: return state.leftTrigger;
                    case GamepadAxis::RightTrigger: return state.rightTrigger;
                    default: return 0.0f;
                }
            }
            return 0.0f;
        }

        static constexpr int MAX_GAMEPADS = 4;

    private:
        /**
         * @brief Reference to the EventBus for dispatching events
         */
        EventBus& m_eventBus;
        
        /**
         * @brief Pointer to SettingsManager for key bindings (can be null)
         */
        rtype::SettingsManager* m_settings = nullptr;
        
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
         * @brief Gamepad states for each connected gamepad
         */
        GamepadState m_gamepadStates[MAX_GAMEPADS];
        /**
         * @brief Previous button states for detecting press/release
         */
        bool m_prevGamepadButtons[MAX_GAMEPADS][static_cast<int>(GamepadButton::COUNT)] = {{false}};

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

            // Set raw key states (for legacy compatibility)
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

            // Set action flags based on key bindings
            updateActionFlags();

            m_eventBus.emit(KeyStateEvent{m_keyState});
        }

        /**
         * @brief Check if a bound action key is currently pressed
         * @param action The action name (e.g., "up", "shoot")
         * @return true if the bound key is held down
         */
        bool isActionKeyDown(const std::string& action) const {
            if (!m_settings) {
                // Fallback to defaults if no settings
                return isDefaultActionDown(action);
            }
            
            int keyCode = m_settings->getBoundKey(action);
            if (keyCode > 0) {
                return IsKeyDown(keyCode);
            }
            return false;
        }

        /**
         * @brief Check default key bindings (when SettingsManager not available)
         */
        bool isDefaultActionDown(const std::string& action) const {
            if (action == "up") return m_keyDown.count(KeyCode::W) ? m_keyDown.at(KeyCode::W) : false || 
                                       m_keyDown.count(KeyCode::Up) ? m_keyDown.at(KeyCode::Up) : false;
            if (action == "down") return m_keyDown.count(KeyCode::S) ? m_keyDown.at(KeyCode::S) : false || 
                                         m_keyDown.count(KeyCode::Down) ? m_keyDown.at(KeyCode::Down) : false;
            if (action == "left") return m_keyDown.count(KeyCode::A) ? m_keyDown.at(KeyCode::A) : false || 
                                         m_keyDown.count(KeyCode::Left) ? m_keyDown.at(KeyCode::Left) : false;
            if (action == "right") return m_keyDown.count(KeyCode::D) ? m_keyDown.at(KeyCode::D) : false || 
                                          m_keyDown.count(KeyCode::Right) ? m_keyDown.at(KeyCode::Right) : false;
            if (action == "shoot") return m_keyDown.count(KeyCode::Space) ? m_keyDown.at(KeyCode::Space) : false;
            if (action == "bomb") return IsKeyDown(KEY_B);
            if (action == "orbSwitch") return m_keyDown.count(KeyCode::Tab) ? m_keyDown.at(KeyCode::Tab) : false;
            if (action == "pause") return m_keyDown.count(KeyCode::Escape) ? m_keyDown.at(KeyCode::Escape) : false;
            if (action == "confirm") return m_keyDown.count(KeyCode::Enter) ? m_keyDown.at(KeyCode::Enter) : false;
            if (action == "cancel") return m_keyDown.count(KeyCode::Escape) ? m_keyDown.at(KeyCode::Escape) : false;
            return false;
        }

        /**
         * @brief Update action flags based on current key bindings
         */
        void updateActionFlags() {
            // Movement actions
            m_keyState.actionUp = isActionKeyDown("up");
            m_keyState.actionDown = isActionKeyDown("down");
            m_keyState.actionLeft = isActionKeyDown("left");
            m_keyState.actionRight = isActionKeyDown("right");
            
            // Combat actions
            m_keyState.actionShoot = isActionKeyDown("shoot");
            m_keyState.actionBomb = isActionKeyDown("bomb");
            m_keyState.actionOrbSwitch = isActionKeyDown("orbSwitch");
            
            // Menu actions
            m_keyState.actionPause = isActionKeyDown("pause");
            m_keyState.actionConfirm = isActionKeyDown("confirm");
            m_keyState.actionCancel = isActionKeyDown("cancel");
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

        /**
         * @brief Poll gamepad state and emit events
         */
        void pollGamepad() {
            for (int gp = 0; gp < MAX_GAMEPADS; ++gp) {
                bool wasConnected = m_gamepadStates[gp].connected;
                bool isConnected = IsGamepadAvailable(gp);

                m_gamepadStates[gp].connected = isConnected;
                m_gamepadStates[gp].gamepadId = gp;

                // Emit connection/disconnection events
                if (isConnected && !wasConnected) {
                    m_eventBus.emit(GamepadConnectedEvent{gp, true});
                } else if (!isConnected && wasConnected) {
                    m_eventBus.emit(GamepadConnectedEvent{gp, false});
                }

                if (!isConnected) {
                    continue;
                }

                // Poll axes with deadzone
                constexpr float DEADZONE = 0.15f;
                
                auto applyDeadzone = [](float value) -> float {
                    constexpr float DZ = 0.15f;
                    if (value > -DZ && value < DZ) return 0.0f;
                    return value;
                };

                float leftX = applyDeadzone(GetGamepadAxisMovement(gp, GAMEPAD_AXIS_LEFT_X));
                float leftY = applyDeadzone(GetGamepadAxisMovement(gp, GAMEPAD_AXIS_LEFT_Y));
                float rightX = applyDeadzone(GetGamepadAxisMovement(gp, GAMEPAD_AXIS_RIGHT_X));
                float rightY = applyDeadzone(GetGamepadAxisMovement(gp, GAMEPAD_AXIS_RIGHT_Y));
                float leftTrigger = GetGamepadAxisMovement(gp, GAMEPAD_AXIS_LEFT_TRIGGER);
                float rightTrigger = GetGamepadAxisMovement(gp, GAMEPAD_AXIS_RIGHT_TRIGGER);

                // Normalize triggers from [-1,1] to [0,1]
                leftTrigger = (leftTrigger + 1.0f) * 0.5f;
                rightTrigger = (rightTrigger + 1.0f) * 0.5f;

                // Emit axis events only on significant change
                constexpr float AXIS_THRESHOLD = 0.05f;
                
                if (std::abs(leftX - m_gamepadStates[gp].leftStickX) > AXIS_THRESHOLD) {
                    m_eventBus.emit(GamepadAxisEvent{gp, GamepadAxis::LeftX, leftX});
                }
                if (std::abs(leftY - m_gamepadStates[gp].leftStickY) > AXIS_THRESHOLD) {
                    m_eventBus.emit(GamepadAxisEvent{gp, GamepadAxis::LeftY, leftY});
                }
                if (std::abs(rightX - m_gamepadStates[gp].rightStickX) > AXIS_THRESHOLD) {
                    m_eventBus.emit(GamepadAxisEvent{gp, GamepadAxis::RightX, rightX});
                }
                if (std::abs(rightY - m_gamepadStates[gp].rightStickY) > AXIS_THRESHOLD) {
                    m_eventBus.emit(GamepadAxisEvent{gp, GamepadAxis::RightY, rightY});
                }
                if (std::abs(leftTrigger - m_gamepadStates[gp].leftTrigger) > AXIS_THRESHOLD) {
                    m_eventBus.emit(GamepadAxisEvent{gp, GamepadAxis::LeftTrigger, leftTrigger});
                }
                if (std::abs(rightTrigger - m_gamepadStates[gp].rightTrigger) > AXIS_THRESHOLD) {
                    m_eventBus.emit(GamepadAxisEvent{gp, GamepadAxis::RightTrigger, rightTrigger});
                }

                m_gamepadStates[gp].leftStickX = leftX;
                m_gamepadStates[gp].leftStickY = leftY;
                m_gamepadStates[gp].rightStickX = rightX;
                m_gamepadStates[gp].rightStickY = rightY;
                m_gamepadStates[gp].leftTrigger = leftTrigger;
                m_gamepadStates[gp].rightTrigger = rightTrigger;

                // Poll buttons - map raylib buttons to our enum
                struct ButtonMapping {
                    int raylibButton;
                    GamepadButton ourButton;
                };
                
                static const ButtonMapping buttonMappings[] = {
                    {GAMEPAD_BUTTON_RIGHT_FACE_DOWN, GamepadButton::FaceDown},
                    {GAMEPAD_BUTTON_RIGHT_FACE_RIGHT, GamepadButton::FaceRight},
                    {GAMEPAD_BUTTON_RIGHT_FACE_LEFT, GamepadButton::FaceLeft},
                    {GAMEPAD_BUTTON_RIGHT_FACE_UP, GamepadButton::FaceUp},
                    {GAMEPAD_BUTTON_LEFT_FACE_UP, GamepadButton::DpadUp},
                    {GAMEPAD_BUTTON_LEFT_FACE_RIGHT, GamepadButton::DpadRight},
                    {GAMEPAD_BUTTON_LEFT_FACE_DOWN, GamepadButton::DpadDown},
                    {GAMEPAD_BUTTON_LEFT_FACE_LEFT, GamepadButton::DpadLeft},
                    {GAMEPAD_BUTTON_LEFT_TRIGGER_1, GamepadButton::LeftBumper},
                    {GAMEPAD_BUTTON_RIGHT_TRIGGER_1, GamepadButton::RightBumper},
                    {GAMEPAD_BUTTON_MIDDLE_LEFT, GamepadButton::Back},
                    {GAMEPAD_BUTTON_MIDDLE_RIGHT, GamepadButton::Start},
                    {GAMEPAD_BUTTON_MIDDLE, GamepadButton::Guide},
                    {GAMEPAD_BUTTON_LEFT_THUMB, GamepadButton::LeftThumb},
                    {GAMEPAD_BUTTON_RIGHT_THUMB, GamepadButton::RightThumb},
                };

                for (const auto& mapping : buttonMappings) {
                    int idx = static_cast<int>(mapping.ourButton);
                    bool wasDown = m_prevGamepadButtons[gp][idx];
                    bool isDown = IsGamepadButtonDown(gp, mapping.raylibButton);

                    m_gamepadStates[gp].buttons[idx] = isDown;

                    if (isDown && !wasDown) {
                        m_eventBus.emit(GamepadButtonPressedEvent{gp, mapping.ourButton});
                    } else if (!isDown && wasDown) {
                        m_eventBus.emit(GamepadButtonReleasedEvent{gp, mapping.ourButton});
                    }

                    m_prevGamepadButtons[gp][idx] = isDown;
                }

                // Emit gamepad state event
                m_eventBus.emit(GamepadStateEvent{m_gamepadStates[gp]});
            }
        }
    };

} // namespace rtype::ecs::events
