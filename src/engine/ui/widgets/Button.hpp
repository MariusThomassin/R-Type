/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Button
*/

#ifndef BUTTON_HPP_
#define BUTTON_HPP_

#include <functional>
#include <iostream>
#include "../Widget.hpp"
#include "../../graphics/RenderUtils.hpp"

#define DEFAULT_BUTTON_TEXT "Button"

namespace rtype::ui {
    enum struct ButtonState {
        NORMAL,
        HOVERED,
        PRESSED,
        DISABLED
    };

    class Button : public Widget {
        public:
            using ClickCallback = std::function<void()>;

            Button(const std::string& text = DEFAULT_BUTTON_TEXT);
            Button(const std::string& text, ButtonState state);

            ~Button() = default;

            void setText(const std::string& text);
            const std::string& getText() const;

            void setState(ButtonState state);
            ButtonState getState() const;

            void setBackgroundColor(UIColor color) override;
            void setBorderColor(UIColor color) override;
            void setTextColor(UIColor color) override;
            void setFontSize(size_t size) override;
            void setBorderWidth(float width) override;
            void setPadding(float padding) override;
            void synchronizeStyle();

            void setOnClick(std::function<void()> callback);

            bool onMouseEnter() override;
            bool onMouseLeave() override;
            bool onMouseClick(float x, float y) override;
            bool onMouseRelease(float x, float y) override;

            void update(float deltaTime) override;
            void renderSelf(const rtype::ecs::RenderContext& ctx) const override;

        private:
            std::string _m_text;
            ButtonState _m_state;
            ClickCallback _m_onClick;

            UIStyle _m_normalStyle;
            UIStyle _m_hoveredStyle;
            UIStyle _m_pressedStyle;
    };
} // namespace rtype::ui

#endif /* !BUTTON_HPP_ */
