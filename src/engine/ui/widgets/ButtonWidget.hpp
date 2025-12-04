/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ButtonWidget - Interactive button widget
*/

#ifndef BUTTONWIDGET_HPP_
#define BUTTONWIDGET_HPP_


#include <string>
#include <functional>
#include "../Widget.hpp"
#include "../../graphics/RenderUtils.hpp"

#define DEFAULT_BUTTON_TEXT "Button"

namespace rtype::ui {

    /**
     * @brief Button state for visual feedback
     */
    enum class ButtonState {
        NORMAL,
        HOVERED,
        PRESSED,
        DISABLED
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
        ButtonWidget(const std::string& text = DEFAULT_BUTTON_TEXT);
        ButtonWidget(const std::string& text, ButtonState state);

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
         * @brief Set the current button state
         * @param state The state to set
         */
        void setState(ButtonState state);

        /**
         * @brief Get the current button state
         * @return Current state
         */
        ButtonState getState() const;

        void setBackgroundColor(UIColor color) override;
        void setBorderColor(UIColor color) override;
        void setTextColor(UIColor color) override;
        void setFontSize(size_t size) override;
        void setBorderWidth(float width) override;
        void setPadding(float padding) override;

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
        void setStateStyle(ButtonState state, const UIStyle& style);

        /**
         * @brief Get color for a specific button state
         * @param state The button state
         * @return The color used for that state
         */
        UIStyle getStateStyle(ButtonState state) const;

        /**
         * @brief Initialize default styles for all button states
         */
        void initializeStyles();

        /**
         * @brief Get the normal state style
         * @return Normal state style
         */
        const UIStyle& getNormalStyle() const;

        /**
         * @brief Get the hovered state style
         * @return Hovered state style
         */
        const UIStyle& getHoveredStyle() const;

        /**
         * @brief Get the pressed state style
         * @return Pressed state style
         */
        const UIStyle& getPressedStyle() const;

        /**
         * @brief Get the disabled state style
         * @return Disabled state style
         */
        const UIStyle& getDisabledStyle() const;

        // Event handlers
        bool onMouseEnter() override;
        bool onMouseLeave() override;
        bool onMouseClick() override;

        /**
         * @brief Render the button
         */
        void renderSelf() const override;

    protected:
        std::string _text;
        ButtonState _state = ButtonState::NORMAL;
        ClickCallback _onClick;

        // Colors for each state
        UIStyle _normalStyle;
        UIStyle _hoveredStyle;
        UIStyle _pressedStyle;
        UIStyle _disabledStyle;
    };

} // namespace rtype::ui

#endif /* !BUTTONWIDGET_HPP_ */
