/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InputUtils - Implementation of input utility functions
*/

#include "InputUtils.hpp"
#include <unordered_map>

namespace rtype::ecs::events {

    // Static member initialization
    bool InputUtils::s_initialized = false;
    std::unordered_map<int, KeyCode> InputUtils::s_raylibToKeyCode;
    std::unordered_map<KeyCode, int> InputUtils::s_keyCodeToRaylib;

    void InputUtils::initializeMappings() {
        if (s_initialized) return;

        // Movement keys
        s_keyCodeToRaylib[KeyCode::Up] = KEY_UP;
        s_keyCodeToRaylib[KeyCode::Down] = KEY_DOWN;
        s_keyCodeToRaylib[KeyCode::Left] = KEY_LEFT;
        s_keyCodeToRaylib[KeyCode::Right] = KEY_RIGHT;
        s_keyCodeToRaylib[KeyCode::W] = KEY_W;
        s_keyCodeToRaylib[KeyCode::A] = KEY_A;
        s_keyCodeToRaylib[KeyCode::S] = KEY_S;
        s_keyCodeToRaylib[KeyCode::D] = KEY_D;

        // Action keys
        s_keyCodeToRaylib[KeyCode::Space] = KEY_SPACE;
        s_keyCodeToRaylib[KeyCode::Enter] = KEY_ENTER;
        s_keyCodeToRaylib[KeyCode::Escape] = KEY_ESCAPE;
        s_keyCodeToRaylib[KeyCode::Tab] = KEY_TAB;
        s_keyCodeToRaylib[KeyCode::Backspace] = KEY_BACKSPACE;

        // Function keys
        s_keyCodeToRaylib[KeyCode::F1] = KEY_F1;
        s_keyCodeToRaylib[KeyCode::F2] = KEY_F2;
        s_keyCodeToRaylib[KeyCode::F3] = KEY_F3;
        s_keyCodeToRaylib[KeyCode::F4] = KEY_F4;
        s_keyCodeToRaylib[KeyCode::F5] = KEY_F5;
        s_keyCodeToRaylib[KeyCode::F6] = KEY_F6;
        s_keyCodeToRaylib[KeyCode::F7] = KEY_F7;
        s_keyCodeToRaylib[KeyCode::F8] = KEY_F8;
        s_keyCodeToRaylib[KeyCode::F9] = KEY_F9;
        s_keyCodeToRaylib[KeyCode::F10] = KEY_F10;
        s_keyCodeToRaylib[KeyCode::F11] = KEY_F11;
        s_keyCodeToRaylib[KeyCode::F12] = KEY_F12;

        // Letter keys
        s_keyCodeToRaylib[KeyCode::B] = KEY_B;
        s_keyCodeToRaylib[KeyCode::C] = KEY_C;
        s_keyCodeToRaylib[KeyCode::E] = KEY_E;
        s_keyCodeToRaylib[KeyCode::F] = KEY_F;
        s_keyCodeToRaylib[KeyCode::G] = KEY_G;
        s_keyCodeToRaylib[KeyCode::H] = KEY_H;
        s_keyCodeToRaylib[KeyCode::I] = KEY_I;
        s_keyCodeToRaylib[KeyCode::J] = KEY_J;
        s_keyCodeToRaylib[KeyCode::K] = KEY_K;
        s_keyCodeToRaylib[KeyCode::L] = KEY_L;
        s_keyCodeToRaylib[KeyCode::M] = KEY_M;
        s_keyCodeToRaylib[KeyCode::N] = KEY_N;
        s_keyCodeToRaylib[KeyCode::O] = KEY_O;
        s_keyCodeToRaylib[KeyCode::P] = KEY_P;
        s_keyCodeToRaylib[KeyCode::Q] = KEY_Q;
        s_keyCodeToRaylib[KeyCode::R] = KEY_R;
        s_keyCodeToRaylib[KeyCode::T] = KEY_T;
        s_keyCodeToRaylib[KeyCode::U] = KEY_U;
        s_keyCodeToRaylib[KeyCode::V] = KEY_V;
        s_keyCodeToRaylib[KeyCode::X] = KEY_X;
        s_keyCodeToRaylib[KeyCode::Y] = KEY_Y;
        s_keyCodeToRaylib[KeyCode::Z] = KEY_Z;

        // Number keys
        s_keyCodeToRaylib[KeyCode::Num0] = KEY_ZERO;
        s_keyCodeToRaylib[KeyCode::Num1] = KEY_ONE;
        s_keyCodeToRaylib[KeyCode::Num2] = KEY_TWO;
        s_keyCodeToRaylib[KeyCode::Num3] = KEY_THREE;
        s_keyCodeToRaylib[KeyCode::Num4] = KEY_FOUR;
        s_keyCodeToRaylib[KeyCode::Num5] = KEY_FIVE;
        s_keyCodeToRaylib[KeyCode::Num6] = KEY_SIX;
        s_keyCodeToRaylib[KeyCode::Num7] = KEY_SEVEN;
        s_keyCodeToRaylib[KeyCode::Num8] = KEY_EIGHT;
        s_keyCodeToRaylib[KeyCode::Num9] = KEY_NINE;

        // Modifier keys
        s_keyCodeToRaylib[KeyCode::LeftShift] = KEY_LEFT_SHIFT;
        s_keyCodeToRaylib[KeyCode::RightShift] = KEY_RIGHT_SHIFT;
        s_keyCodeToRaylib[KeyCode::LeftControl] = KEY_LEFT_CONTROL;
        s_keyCodeToRaylib[KeyCode::RightControl] = KEY_RIGHT_CONTROL;
        s_keyCodeToRaylib[KeyCode::LeftAlt] = KEY_LEFT_ALT;
        s_keyCodeToRaylib[KeyCode::RightAlt] = KEY_RIGHT_ALT;

        // Symbol keys
        s_keyCodeToRaylib[KeyCode::Equal] = KEY_EQUAL;
        s_keyCodeToRaylib[KeyCode::Minus] = KEY_MINUS;

        // Build reverse mapping
        for (const auto& [keyCode, raylibKey] : s_keyCodeToRaylib) {
            s_raylibToKeyCode[raylibKey] = keyCode;
        }

        s_initialized = true;
    }

    KeyCode InputUtils::raylibToKeyCode(int raylibKey) {
        initializeMappings();
        
        auto it = s_raylibToKeyCode.find(raylibKey);
        return (it != s_raylibToKeyCode.end()) ? it->second : KeyCode::Up; // Default fallback
    }

    int InputUtils::keyCodeToRaylib(KeyCode keyCode) {
        initializeMappings();
        
        auto it = s_keyCodeToRaylib.find(keyCode);
        return (it != s_keyCodeToRaylib.end()) ? it->second : -1; // -1 = not found
    }

    std::string InputUtils::keyCodeToString(KeyCode key) {
        switch (key) {
            case KeyCode::Up: return "UP ARROW";
            case KeyCode::Down: return "DOWN ARROW";
            case KeyCode::Left: return "LEFT ARROW";
            case KeyCode::Right: return "RIGHT ARROW";
            case KeyCode::W: return "W";
            case KeyCode::A: return "A";
            case KeyCode::S: return "S";
            case KeyCode::D: return "D";
            case KeyCode::Space: return "SPACE";
            case KeyCode::Enter: return "ENTER";
            case KeyCode::Escape: return "ESCAPE";
            case KeyCode::Tab: return "TAB";
            case KeyCode::Backspace: return "BACKSPACE";
            case KeyCode::LeftShift: return "LEFT SHIFT";
            case KeyCode::RightShift: return "RIGHT SHIFT";
            case KeyCode::LeftControl: return "LEFT CTRL";
            case KeyCode::RightControl: return "RIGHT CTRL";
            case KeyCode::LeftAlt: return "LEFT ALT";
            case KeyCode::RightAlt: return "RIGHT ALT";
            case KeyCode::F1: return "F1";
            case KeyCode::F2: return "F2";
            case KeyCode::F3: return "F3";
            case KeyCode::F4: return "F4";
            case KeyCode::F5: return "F5";
            case KeyCode::F6: return "F6";
            case KeyCode::F7: return "F7";
            case KeyCode::F8: return "F8";
            case KeyCode::F9: return "F9";
            case KeyCode::F10: return "F10";
            case KeyCode::F11: return "F11";
            case KeyCode::F12: return "F12";
            case KeyCode::B: return "B";
            case KeyCode::C: return "C";
            case KeyCode::E: return "E";
            case KeyCode::F: return "F";
            case KeyCode::G: return "G";
            case KeyCode::H: return "H";
            case KeyCode::I: return "I";
            case KeyCode::J: return "J";
            case KeyCode::K: return "K";
            case KeyCode::L: return "L";
            case KeyCode::M: return "M";
            case KeyCode::N: return "N";
            case KeyCode::O: return "O";
            case KeyCode::P: return "P";
            case KeyCode::Q: return "Q";
            case KeyCode::R: return "R";
            case KeyCode::T: return "T";
            case KeyCode::U: return "U";
            case KeyCode::V: return "V";
            case KeyCode::X: return "X";
            case KeyCode::Y: return "Y";
            case KeyCode::Z: return "Z";
            case KeyCode::Num0: return "0";
            case KeyCode::Num1: return "1";
            case KeyCode::Num2: return "2";
            case KeyCode::Num3: return "3";
            case KeyCode::Num4: return "4";
            case KeyCode::Num5: return "5";
            case KeyCode::Num6: return "6";
            case KeyCode::Num7: return "7";
            case KeyCode::Num8: return "8";
            case KeyCode::Num9: return "9";
            case KeyCode::Equal: return "=";
            case KeyCode::Minus: return "-";
            default: return "UNKNOWN";
        }
    }

    std::vector<KeyCode> InputUtils::getAllSupportedKeys() {
        initializeMappings();
        
        std::vector<KeyCode> keys;
        for (const auto& [keyCode, raylibKey] : s_keyCodeToRaylib) {
            keys.push_back(keyCode);
        }
        return keys;
    }

} // namespace rtype::ecs::events