/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ToggleButtonWidget implementation
*/

#include "ToggleButtonWidget.hpp"
#include <raylib.h>

namespace rtype::ui {

    ToggleButtonWidget::ToggleButtonWidget(const std::string& text, bool toggled)
        : _text(text), _toggled(toggled)
    {
        _m_transform.width = 100.0f;
        _m_transform.height = 28.0f;
        
        _m_style.textColor = Color::White();
        _m_style.borderWidth = 1.0f;
        _m_style.borderColor = Color(80, 80, 100, 255);
    }

    void ToggleButtonWidget::setText(const std::string& text)
    {
        _text = text;
    }

    const std::string& ToggleButtonWidget::getText() const
    {
        return _text;
    }

    void ToggleButtonWidget::setToggled(bool toggled)
    {
        if (_toggled != toggled) {
            _toggled = toggled;
            if (_onToggle) {
                _onToggle(_toggled);
            }
        }
    }

    bool ToggleButtonWidget::isToggled() const
    {
        return _toggled;
    }

    void ToggleButtonWidget::setOnToggle(ToggleCallback callback)
    {
        _onToggle = std::move(callback);
    }

    void ToggleButtonWidget::setOffColor(const Color& color)
    {
        _offColor = color;
    }

    void ToggleButtonWidget::setOnColor(const Color& color)
    {
        _onColor = color;
    }

    void ToggleButtonWidget::setHoverColor(const Color& color)
    {
        _hoverColor = color;
    }

    bool ToggleButtonWidget::onMouseEnter()
    {
        if (_m_enabled) {
            _hovered = true;
        }
        return true;
    }

    bool ToggleButtonWidget::onMouseLeave()
    {
        _hovered = false;
        return true;
    }

    bool ToggleButtonWidget::onMouseClick(float x, float y)
    {
        (void)x;
        (void)y;
        if (!_m_enabled) return false;

        _toggled = !_toggled;
        if (_onToggle) {
            _onToggle(_toggled);
        }
        return true;
    }

    void ToggleButtonWidget::renderSelf()
    {
        auto transform = getAbsoluteTransform();
        
        // Determine background color
        Color bgColor;
        if (_toggled) {
            bgColor = _hovered ? _onColor.withAlpha(220) : _onColor;
        } else {
            bgColor = _hovered ? _hoverColor : _offColor;
        }

        if (!_m_enabled) {
            bgColor = bgColor.withAlpha(128);
        }

        // Draw background
        DrawRectangle(
            static_cast<int>(transform.x),
            static_cast<int>(transform.y),
            static_cast<int>(transform.width),
            static_cast<int>(transform.height),
            bgColor.toRaylib()
        );

        // Draw border (brighter when toggled)
        Color borderColor = _toggled ? 
            Color(120, 180, 120, 255) : 
            _m_style.borderColor;
        
        if (_m_style.borderWidth > 0) {
            DrawRectangleLinesEx(
                Rectangle{transform.x, transform.y, transform.width, transform.height},
                _m_style.borderWidth,
                borderColor.toRaylib()
            );
        }

        // Draw centered text
        int fontSize = static_cast<int>(_m_style.fontSize > 0 ? _m_style.fontSize : 14);
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
