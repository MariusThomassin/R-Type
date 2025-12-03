/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Button
*/

#include "Button.hpp"

namespace rtype::ui {
    Button::Button(const std::string& text) : Button(text, ButtonState::NORMAL)
    {
    }

    Button::Button(const std::string& text, ButtonState state) : Widget(), _m_text(text), _m_state(state)
    {
        // Initialize styles for different states
        _m_normalStyle = getStyle();
        _m_hoveredStyle = _m_normalStyle;
        _m_pressedStyle = _m_normalStyle;

        _m_hoveredStyle.backgroundColor.setRed(_m_normalStyle.backgroundColor.getRed() + 20);
        _m_pressedStyle.backgroundColor.setRed(_m_normalStyle.backgroundColor.getRed() - 20);
    }

    void Button::setText(const std::string& text)
    {
        _m_text = text;
    }

    const std::string& Button::getText() const
    {
        return _m_text;
    }

    void Button::setState(ButtonState state)
    {
        _m_state = state;
    }

    ButtonState Button::getState() const
    {
        return _m_state;
    }

    void Button::setBackgroundColor(UIColor color)
    {
        Widget::setBackgroundColor(color);
        synchronizeStyle();

        if (_m_state == ButtonState::HOVERED) {
            _m_style = _m_hoveredStyle;
        } else if (_m_state == ButtonState::PRESSED) {
            _m_style = _m_pressedStyle;
        }
    }

    void Button::setBorderColor(UIColor color)
    {
        Widget::setBorderColor(color);
        synchronizeStyle();

        if (_m_state == ButtonState::HOVERED) {
            _m_style = _m_hoveredStyle;
        } else if (_m_state == ButtonState::PRESSED) {
            _m_style = _m_pressedStyle;
        }
    }

    void Button::setTextColor(UIColor color)
    {
        Widget::setTextColor(color);
        synchronizeStyle();

        if (_m_state == ButtonState::HOVERED) {
            _m_style = _m_hoveredStyle;
        } else if (_m_state == ButtonState::PRESSED) {
            _m_style = _m_pressedStyle;
        }
    }

    void Button::setFontSize(size_t size)
    {
        Widget::setFontSize(size);
        synchronizeStyle();

        if (_m_state == ButtonState::HOVERED) {
            _m_style = _m_hoveredStyle;
        } else if (_m_state == ButtonState::PRESSED) {
            _m_style = _m_pressedStyle;
        }
    }

    void Button::setBorderWidth(float width)
    {
        Widget::setBorderWidth(width);
        synchronizeStyle();

        if (_m_state == ButtonState::HOVERED) {
            _m_style = _m_hoveredStyle;
        } else if (_m_state == ButtonState::PRESSED) {
            _m_style = _m_pressedStyle;
        }
    }

    void Button::setPadding(float padding)
    {
        Widget::setPadding(padding);
        synchronizeStyle();

        if (_m_state == ButtonState::HOVERED) {
            _m_style = _m_hoveredStyle;
        } else if (_m_state == ButtonState::PRESSED) {
            _m_style = _m_pressedStyle;
        }
    }

    void Button::synchronizeStyle()
    {
        if (_m_state == ButtonState::NORMAL) {
            _m_normalStyle = getStyle();

            _m_hoveredStyle = _m_normalStyle;
            _m_pressedStyle = _m_normalStyle;

            _m_hoveredStyle.backgroundColor.setRed(_m_normalStyle.backgroundColor.getRed() + 20);
            _m_pressedStyle.backgroundColor.setRed(_m_normalStyle.backgroundColor.getRed() - 20);
        }
    }

    void Button::setOnClick(std::function<void()> callback)
    {
        _m_onClick = callback;
    }

    bool Button::onMouseEnter()
    {
        setState(ButtonState::HOVERED);
        _m_style = _m_hoveredStyle;
        return false;
    }

    bool Button::onMouseLeave()
    {
        setState(ButtonState::NORMAL);
        _m_style = _m_normalStyle;
        return false;
    }

    bool Button::onMouseClick(float x, float y)
    {
        setState(ButtonState::PRESSED);
        _m_style = _m_pressedStyle;
        
        // Exécuter le callback immédiatement
        if (_m_onClick) {
            _m_onClick();
        }
        
        return true;
    }

    bool Button::onMouseRelease(float x, float y)
    {
        if (_m_state == ButtonState::PRESSED) {
            if (_m_onClick) {
                _m_onClick();
            }
            setState(ButtonState::HOVERED);
            _m_style = _m_hoveredStyle;
        }
        return true;
    }

    void Button::update(float deltaTime)
    {
        (void)deltaTime;
    }

    void Button::renderSelf(const rtype::ecs::RenderContext& ctx) const
    {
        UITransform absTransform = getAbsoluteTransform();
        const UIStyle& style = getStyle();

        rtype::ecs::RenderUtils::drawUiRect(absTransform, style.backgroundColor);

        if (style.borderWidth > 0.0f) {
            rtype::ecs::RenderUtils::drawUiRectOutline(absTransform, style.borderWidth, style.borderColor);
        }

        rtype::ecs::RenderUtils::drawUiCenteredText(_m_text, absTransform, style.fontSize, style.textColor);
    }
}
