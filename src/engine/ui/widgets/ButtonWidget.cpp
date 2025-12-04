/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ButtonWidget implementation
*/

#include "ButtonWidget.hpp"
#include <raylib.h>

namespace rtype::ui {

    ButtonWidget::ButtonWidget(const std::string& text) : ButtonWidget(text, ButtonState::NORMAL)
    {
    }

    ButtonWidget::ButtonWidget(const std::string& text, ButtonState state) : _text(text), _state(state)
    {
        initializeStyles();
        _m_style = getStateStyle(_state);
    }

    void ButtonWidget::initializeStyles()
    {
        _normalStyle = getStyle();

        _hoveredStyle = _normalStyle;
        auto hoveredBg = _normalStyle.backgroundColor;
        hoveredBg.setRed(std::min(255, (int)hoveredBg.getRed() + 20));
        hoveredBg.setGreen(std::min(255, (int)hoveredBg.getGreen() + 20));
        hoveredBg.setBlue(std::min(255, (int)hoveredBg.getBlue() + 20));
        _hoveredStyle.backgroundColor = hoveredBg;

        _pressedStyle = _normalStyle;
        auto pressedBg = _normalStyle.backgroundColor;
        pressedBg.setRed(std::max(0, (int)pressedBg.getRed() - 30));
        pressedBg.setGreen(std::max(0, (int)pressedBg.getGreen() - 30));
        pressedBg.setBlue(std::max(0, (int)pressedBg.getBlue() - 30));
        _pressedStyle.backgroundColor = pressedBg;

        _disabledStyle = _normalStyle;
        _disabledStyle.backgroundColor = UIColor(128, 128, 128, 255);
        _disabledStyle.textColor = UIColor(64, 64, 64, 255);
    }

    void ButtonWidget::setText(const std::string& text)
    {
        _text = text;
    }

    const std::string& ButtonWidget::getText() const
    {
        return _text;
    }

    void ButtonWidget::setState(ButtonState state)
    {
        _state = state;
        _m_style = getStateStyle(_state);
    }

    ButtonState ButtonWidget::getState() const
    {
        return _m_enabled ? _state : ButtonState::DISABLED;
    }

    void ButtonWidget::setBackgroundColor(UIColor color)
    {
        Widget::setBackgroundColor(color);

        // Update the current state's style
        UIStyle currentStateStyle = getStateStyle(_state);
        currentStateStyle.backgroundColor = color;
        setStateStyle(_state, currentStateStyle);

        // Apply the style immediately
        _m_style = currentStateStyle;
    }

    void ButtonWidget::setBorderColor(UIColor color)
    {
        Widget::setBorderColor(color);
        
        // Update the current state's style
        UIStyle currentStateStyle = getStateStyle(_state);
        currentStateStyle.borderColor = color;
        setStateStyle(_state, currentStateStyle);
        
        // Apply the style immediately
        _m_style = currentStateStyle;
    }

    void ButtonWidget::setTextColor(UIColor color)
    {
        Widget::setTextColor(color);
        
        // Update the current state's style
        UIStyle currentStateStyle = getStateStyle(_state);
        currentStateStyle.textColor = color;
        setStateStyle(_state, currentStateStyle);
        
        // Apply the style immediately
        _m_style = currentStateStyle;
    }

    void ButtonWidget::setFontSize(size_t size)
    {
        Widget::setFontSize(size);
        
        // Update the current state's style
        UIStyle currentStateStyle = getStateStyle(_state);
        currentStateStyle.fontSize = size;
        setStateStyle(_state, currentStateStyle);
        
        // Apply the style immediately
        _m_style = currentStateStyle;
    }

    void ButtonWidget::setBorderWidth(float width)
    {
        Widget::setBorderWidth(width);
        
        // Update the current state's style
        UIStyle currentStateStyle = getStateStyle(_state);
        currentStateStyle.borderWidth = width;
        setStateStyle(_state, currentStateStyle);
        
        // Apply the style immediately
        _m_style = currentStateStyle;
    }

    void ButtonWidget::setPadding(float padding)
    {
        Widget::setPadding(padding);
        
        // Update the current state's style
        UIStyle currentStateStyle = getStateStyle(_state);
        currentStateStyle.padding = padding;
        setStateStyle(_state, currentStateStyle);
        
        // Apply the style immediately
        _m_style = currentStateStyle;
    }



    void ButtonWidget::setOnClick(ClickCallback callback)
    {
        _onClick = std::move(callback);
    }

    void ButtonWidget::setStateStyle(ButtonState state, const UIStyle& style)
    {
        switch (state) {
            case ButtonState::NORMAL:
                _normalStyle = style;
                break;
            case ButtonState::HOVERED:
                _hoveredStyle = style;
                break;
            case ButtonState::PRESSED:
                _pressedStyle = style;
                break;
            case ButtonState::DISABLED:
                _disabledStyle = style;
                break;
        }
    }

    UIStyle ButtonWidget::getStateStyle(ButtonState state) const
    {
        switch (state) {
            case ButtonState::NORMAL:
                return _normalStyle;
            case ButtonState::HOVERED:
                return _hoveredStyle;
            case ButtonState::PRESSED:
                return _pressedStyle;
            case ButtonState::DISABLED:
                return _disabledStyle;
            default:
                return _normalStyle;
        }
    }

    bool ButtonWidget::onMouseEnter()
    {
        if (_m_enabled && _state != ButtonState::PRESSED) {
            setState(ButtonState::HOVERED);
        }
        return true;
    }

    bool ButtonWidget::onMouseLeave()
    {
        if (_m_enabled) {
            setState(ButtonState::NORMAL);
        }
        return true;
    }

    bool ButtonWidget::onMouseClick()
    {
        if (!_m_enabled) return false;

        setState(ButtonState::PRESSED);
        if (_onClick) {
            _onClick();
        }
        setState(ButtonState::HOVERED); // Return to hover after click
        return true;
    }

    const UIStyle& ButtonWidget::getNormalStyle() const
    {
        return _normalStyle;
    }

    const UIStyle& ButtonWidget::getHoveredStyle() const
    {
        return _hoveredStyle;
    }

    const UIStyle& ButtonWidget::getPressedStyle() const
    {
        return _pressedStyle;
    }

    const UIStyle& ButtonWidget::getDisabledStyle() const
    {
        return _disabledStyle;
    }

    void ButtonWidget::renderSelf() const
    {
        auto UItransform = getAbsoluteTransform();
        
        rtype::ecs::RenderUtils::drawUiRect(UItransform, _m_style.backgroundColor);

        if (_m_style.borderWidth > 0.0f) {
            rtype::ecs::RenderUtils::drawUiRectOutline(UItransform, _m_style.borderWidth, _m_style.borderColor);
        }

        rtype::ecs::RenderUtils::drawUiCenteredText(_text, UItransform, _m_style.fontSize, _m_style.textColor);
    }

} // namespace rtype::ui
