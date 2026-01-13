/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** KeyBindingWidget - Widget for displaying and remapping key bindings
*/

#pragma once

#include "../Widget.hpp"
#include "ButtonWidget.hpp"
#include "../../ecs/events/InputEvents.hpp"
#include "../../ecs/events/InputUtils.hpp"
#include <functional>
#include <string>

namespace rtype::ui {

    /**
     * @brief Widget for displaying and remapping key bindings
     * 
     * This widget shows the current key binding for an action and allows
     * the user to click on it to remap the key. When clicked, it enters
     * a "waiting for input" state and captures the next key press.
     */
    class KeyBindingWidget : public Widget {
        public:
            using KeyChangeCallback = std::function<void(rtype::ecs::events::KeyCode newKey)>;
            
            /**
             * @brief Construct a KeyBindingWidget
             * @param actionName The name of the action (e.g., "Move Up")
             * @param currentKey The currently bound key
             */
            KeyBindingWidget(const std::string& actionName, rtype::ecs::events::KeyCode currentKey);

            /**
             * @brief Destroy the KeyBindingWidget
             */
            ~KeyBindingWidget() override = default;

            /**
             * @brief Set the callback for when the key binding changes
             * @param callback Function to call when a new key is bound
             */
            void setOnKeyChange(const KeyChangeCallback& callback);

            /**
             * @brief Update the current key binding
             * @param newKey The new key to display
             */
            void setCurrentKey(rtype::ecs::events::KeyCode newKey);

            /**
             * @brief Get the current key binding
             * @return The currently bound key
             */
            rtype::ecs::events::KeyCode getCurrentKey() const;

            /**
             * @brief Get the action name
             * @return The action name
             */
            const std::string& getActionName() const;

            /**
             * @brief Set whether this widget is waiting for key input
             * @param waiting True if waiting for input
             */
            void setWaitingForInput(bool waiting);

            /**
             * @brief Check if this widget is waiting for key input
             * @return True if waiting for input
             */
            bool isWaitingForInput() const;

            /**
             * @brief Handle mouse click event
             * @return true if event was consumed
             */
            bool onMouseClick() override;
            /**
             * @brief Handle key press event
             * @param key The key that was pressed
             * @return true if event was consumed
             */
            bool onKeyPress(rtype::ecs::events::KeyCode key) override;
            /**
             * @brief Render the key binding widget
             */
            void renderSelf() const override;
            /**
             * @brief Update the widget (for blinking text when waiting for input)
             * @param deltaTime Time since last update
             */
            void update(float deltaTime) override;

        private:
            /**
             * @brief Name of the action (e.g., "Move Up")
             */
            std::string _actionName;
            /**
             * @brief Currently bound key
             */
            rtype::ecs::events::KeyCode _currentKey;
            /**
             * @brief Whether the widget is waiting for key input
             */
            bool _waitingForInput;
            /**
             * @brief Timer for blinking text when waiting for input
             */
            float _blinkTimer;
            /**
             * @brief Callback function when key binding changes
             */
            KeyChangeCallback _onKeyChange;

            /**
             * @brief Convert KeyCode to display string
             * @param key The key code to convert
             * @return Human-readable key name
             */
            std::string keyCodeToString(rtype::ecs::events::KeyCode key) const;

            /**
             * @brief Get the display text for the current state
             * @return Display text
             */
            std::string getDisplayText() const;
        };
} // namespace rtype::ui
