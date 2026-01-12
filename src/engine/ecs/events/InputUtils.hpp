/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** InputUtils - Utility functions for input handling and key conversion
*/

#pragma once

#include "../events/InputEvents.hpp"
#include <raylib.h>
#include <string>
#include <vector>
#include <unordered_map>

namespace rtype::ecs::events {

    /**
     * @brief Utility functions for input handling
     */
    class InputUtils {
    public:
        /**
         * @brief Convert raylib key code to our KeyCode enum
         * @param raylibKey Raylib key constant
         * @return Our KeyCode enum value, or KeyCode::Up if not found
         */
        static KeyCode raylibToKeyCode(int raylibKey);

        /**
         * @brief Convert our KeyCode enum to raylib key code
         * @param keyCode Our KeyCode enum value
         * @return Raylib key constant, or -1 if not found
         */
        static int keyCodeToRaylib(KeyCode keyCode);

        /**
         * @brief Convert KeyCode to human-readable string
         * @param key KeyCode to convert
         * @return Human-readable key name
         */
        static std::string keyCodeToString(KeyCode key);

        /**
         * @brief Get all supported key codes
         * @return Vector of all supported KeyCode values
         */
        static std::vector<KeyCode> getAllSupportedKeys();

    private:
        /**
         * @brief Initialize key mappings (called internally)
         */
        static void initializeMappings();

        /**
         * @brief Check if mappings are initialized
         */
        static bool s_initialized;
        
        /**
         * @brief Mapping from raylib keys to our KeyCode
         */
        static std::unordered_map<int, KeyCode> s_raylibToKeyCode;
        
        /**
         * @brief Mapping from our KeyCode to raylib keys  
         */
        static std::unordered_map<KeyCode, int> s_keyCodeToRaylib;
    };

} // namespace rtype::ecs::events