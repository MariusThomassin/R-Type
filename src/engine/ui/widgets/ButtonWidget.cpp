/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ButtonWidget implementation
*/

#include "ButtonWidget.hpp"
#include <raylib.h>

namespace rtype::ui {

    ButtonWidget::ButtonWidget(const std::string& text)
        : _text(text)
    {
        _m_transform.width = 120.0f;
        _m_transform.height = 30.0f;
        
        _m_style.textColor = Color::White();
        _m_style.borderWidth = 1.0f;
        _m_style.borderColor = Color(60, 60, 60, 255);
    }

    void ButtonWidget::setText(const std::string& text)
    {
        _text = text;
    }

    const std::string& ButtonWidget::getText() const
    {
        return _text;
    }

    void ButtonWidget::setOnClick(ClickCallback callback)
    {
        _onClick = std::move(callback);
    }

    void ButtonWidget::setStateColor(ButtonState state, const Color& color)
    {
        switch (state) {
            case ButtonState::Normal:
                _normalColor = color;
                break;
            case ButtonState::Hovered:
                _hoverColor = color;
                break;
            case ButtonState::Pressed:
                _pressedColor = color;
                break;
            case ButtonState::Disabled:
                _disabledColor = color;
                break;
        }
    }

    Color ButtonWidget::getStateColor(ButtonState state) const
    {
        switch (state) {
            case ButtonState::Hovered:
                return _hoverColor;
            case ButtonState::Pressed:
                return _pressedColor;
            case ButtonState::Disabled:
                return _disabledColor;
            default:
                return _normalColor;
        }
    }

    ButtonState ButtonWidget::getState() const
    {
        return _m_enabled ? _state : ButtonState::Disabled;
    }

    bool ButtonWidget::onMouseEnter()
    {
        if (_m_enabled && _state != ButtonState::Pressed) {
            _state = ButtonState::Hovered;
        }
        return true;
    }

    bool ButtonWidget::onMouseLeave()
    {
        if (_m_enabled) {
            _state = ButtonState::Normal;
        }
        return true;
    }

    bool ButtonWidget::onMouseClick(float x, float y)
    {
        if (!_m_enabled) return false;

        if (contains(x, y)) {
            _state = ButtonState::Pressed;
            if (_onClick) {
                _onClick();
            }
            _state = ButtonState::Hovered; // Return to hover after click
            return true;
        }
        return false;
    }

    void ButtonWidget::renderSelf()
    {
        auto transform = getAbsoluteTransform();
        
        // Determine background color based on state
        Color bgColor;
        switch (getState()) {
            case ButtonState::Hovered:
                bgColor = _hoverColor;
                break;
            case ButtonState::Pressed:
                bgColor = _pressedColor;
                break;
            case ButtonState::Disabled:
                bgColor = _disabledColor;
                break;
            default:
                bgColor = _normalColor;
                break;
        }

        // Draw button background
        DrawRectangle(
            static_cast<int>(transform.x),
            static_cast<int>(transform.y),
            static_cast<int>(transform.width),
            static_cast<int>(transform.height),
            bgColor.toRaylib()
        );

        // Draw border
        if (_m_style.borderWidth > 0) {
            DrawRectangleLinesEx(
                Rectangle{transform.x, transform.y, transform.width, transform.height},
                _m_style.borderWidth,
                _m_style.borderColor.toRaylib()
            );
        }

        // Draw centered text
        int fontSize = static_cast<int>(_m_style.fontSize);
        int textWidth = MeasureText(_text.c_str(), fontSize);
        
        float textX = transform.x + (transform.width - textWidth) / 2.0f;
        float textY = transform.y + (transform.height - fontSize) / 2.0f;

        Color textColor = _m_enabled ? _m_style.textColor : _m_style.textColor.withAlpha(128);

        DrawText(
            _text.c_str(),
            static_cast<int>(textX),
            static_cast<int>(textY),
            fontSize,
            textColor.toRaylib()
        );
    }

} // namespace rtype::ui
