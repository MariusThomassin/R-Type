/*
** R-Type Engine - WindowSerializer
** Saves and loads window state for persistence across sessions
*/

#ifndef WINDOWSERIALIZER_HPP_
#define WINDOWSERIALIZER_HPP_

#include "../WindowManager.hpp"
#include <string>
#include <vector>

namespace rtype::ui {

    /**
     * @brief Represents the serializable state of a window
     */
    struct WindowState {
        std::string id;
        float x = 0.0f;
        float y = 0.0f;
        float width = 400.0f;
        float height = 300.0f;
        bool visible = true;
        bool collapsed = false;
        float scrollOffset = 0.0f;
    };

    /**
     * @brief Serializer for window states to/from JSON
     * 
     * Allows saving window positions, sizes, and states to a file
     * and restoring them on application startup.
     */
    class WindowSerializer {
    public:
        /**
         * @brief Save all window states from a WindowManager to a file
         * @param manager The WindowManager to save states from
         * @param filepath Path to the JSON file
         * @return true if save was successful
         */
        static bool saveStates(const WindowManager& manager, const std::string& filepath);

        /**
         * @brief Load window states from a file and apply to WindowManager
         * @param manager The WindowManager to apply states to
         * @param filepath Path to the JSON file
         * @return true if load was successful
         */
        static bool loadStates(WindowManager& manager, const std::string& filepath);

        /**
         * @brief Capture all current window states from a WindowManager
         * @param manager The WindowManager to capture from
         * @return Vector of WindowState structs
         */
        static std::vector<WindowState> captureStates(const WindowManager& manager);

        /**
         * @brief Apply window states to a WindowManager
         * @param manager The WindowManager to apply to
         * @param states Vector of WindowState structs to apply
         */
        static void applyStates(WindowManager& manager, const std::vector<WindowState>& states);

        /**
         * @brief Serialize window states to JSON string
         * @param states Vector of WindowState structs
         * @return JSON string representation
         */
        static std::string toJson(const std::vector<WindowState>& states);

        /**
         * @brief Deserialize window states from JSON string
         * @param json JSON string
         * @return Vector of WindowState structs
         */
        static std::vector<WindowState> fromJson(const std::string& json);
    };

} // namespace rtype::ui

#endif // WINDOWSERIALIZER_HPP_
