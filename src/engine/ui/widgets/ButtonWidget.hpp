/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ButtonWidget - Interactive button widget
*/

#ifndef BUTTONWIDGET_HPP_
#define BUTTONWIDGET_HPP_

#include "src/engine/ui/Widget.hpp"
#include <string>
#include <functional>

namespace rtype::ui {

    /**
     * @brief Button state for visual feedback
     */
    enum class ButtonState {
        Normal,
        Hovered,
        Pressed,
        Disabled
    };

    /**
     * @brief An interactive button widget
     * 
     * ButtonWidget provides a clickable button with text label and
     * visual feedback for different states (normal, hovered, pressed, disabled).
     * Supports custom click callbacks.
     */
    class ButtonWidget : public Widget {
    public:
        using ClickCallback = std::function<void()>;

        /**
         * @brief Construct a new ButtonWidget
         * @param text Button label text
         */
        explicit ButtonWidget(const std::string& text = "Button");

        ~ButtonWidget() override = default;

        /**
         * @brief Set the button label text
         * @param text The text to display
         */
        void setText(const std::string& text);

        /**
         * @brief Get the button label text
         * @return The button text
         */
        const std::string& getText() const;

        /**
         * @brief Set the click callback
         * @param callback Function to call when button is clicked
         */
        void setOnClick(ClickCallback callback);

        /**
         * @brief Set color for a specific button state
         * @param state The button state
         * @param color The color to use for that state
         */
        void setStateColor(ButtonState state, const Color& color);

        /**
         * @brief Get color for a specific button state
         * @param state The button state
         * @return The color used for that state
         */
        Color getStateColor(ButtonState state) const;

        /**
         * @brief Get the current button state
         * @return Current state
         */
        ButtonState getState() const;

        // Event handlers
        bool onMouseEnter() override;
        bool onMouseLeave() override;
        bool onMouseClick(float x, float y) override;

        /**
         * @brief Render the button
         */
        void renderSelf() override;

    protected:
        std::string _text;
        ClickCallback _onClick;
        ButtonState _state = ButtonState::Normal;

        // Colors for each state
        Color _normalColor = Color(80, 80, 80, 255);
        Color _hoverColor = Color(100, 100, 100, 255);
        Color _pressedColor = Color(60, 60, 60, 255);
        Color _disabledColor = Color(50, 50, 50, 128);
    };

} // namespace rtype::ui

#endif /* !BUTTONWIDGET_HPP_ */
