/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ToggleButtonWidget - A button that maintains on/off state
*/

#ifndef TOGGLEBUTTONWIDGET_HPP_
#define TOGGLEBUTTONWIDGET_HPP_

#include "src/engine/ui/Widget.hpp"
#include <string>
#include <functional>

namespace rtype::ui {

    /**
     * @brief A toggle button that maintains pressed/unpressed state
     * 
     * ToggleButtonWidget is like ButtonWidget but maintains an on/off state.
     * Useful for options, filters, and selection groups.
     */
    class ToggleButtonWidget : public Widget {
    public:
        using ToggleCallback = std::function<void(bool toggled)>;

        /**
         * @brief Construct a new ToggleButtonWidget
         * @param text Button label text
         * @param toggled Initial toggle state
         */
        explicit ToggleButtonWidget(const std::string& text = "Toggle", bool toggled = false);

        ~ToggleButtonWidget() override = default;

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
         * @brief Set the toggle state
         * @param toggled True for on, false for off
         */
        void setToggled(bool toggled);

        /**
         * @brief Get the current toggle state
         * @return True if toggled on
         */
        bool isToggled() const;

        /**
         * @brief Set the toggle callback
         * @param callback Function called when toggle state changes
         */
        void setOnToggle(ToggleCallback callback);

        /**
         * @brief Set color for off (normal) state
         * @param color The color
         */
        void setOffColor(const Color& color);

        /**
         * @brief Set color for on (toggled) state
         * @param color The color
         */
        void setOnColor(const Color& color);

        /**
         * @brief Set color for hover state
         * @param color The color
         */
        void setHoverColor(const Color& color);

        // Event handlers
        bool onMouseEnter() override;
        bool onMouseLeave() override;
        bool onMouseClick(float x, float y) override;

        /**
         * @brief Render the toggle button
         */
        void renderSelf() override;

    protected:
        std::string _text;
        bool _toggled = false;
        bool _hovered = false;
        
        ToggleCallback _onToggle;

        Color _offColor = Color(60, 60, 70, 255);
        Color _onColor = Color(60, 120, 60, 255);
        Color _hoverColor = Color(80, 80, 90, 255);
    };

} // namespace rtype::ui

#endif /* !TOGGLEBUTTONWIDGET_HPP_ */
