/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ButtonWidget implementation
*/

#include "ButtonWidget.hpp"
#include <raylib.h>
#include <iostream>
#include <algorithm>

// Static member definitions
Sound rtype::ui::ButtonWidget::s_defaultClickSound = {};
bool rtype::ui::ButtonWidget::s_defaultSoundLoaded = false;
std::string rtype::ui::ButtonWidget::s_defaultSoundPath = "";
float rtype::ui::ButtonWidget::s_soundVolume = 0.75f;  // Default 75% volume

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

        // Beautiful hover effect - brighten colors while maintaining hue
        _hoveredStyle = _normalStyle;
        auto hoveredBg = _normalStyle.backgroundColor;
        
        // Calculate luminance and boost saturation/brightness intelligently
        int r = hoveredBg.getRed();
        int g = hoveredBg.getGreen();
        int b = hoveredBg.getBlue();
        
        // Find the dominant color and enhance it
        int maxComponent = std::max({r, g, b});
        float boostFactor = 1.4f; // 40% brighter
        
        // Apply intelligent color boost
        r = std::min(255, (int)(r * boostFactor));
        g = std::min(255, (int)(g * boostFactor));
        b = std::min(255, (int)(b * boostFactor));
        
        _hoveredStyle.backgroundColor = UIColor(r, g, b, hoveredBg.getAlpha());
        
        // Enhance border for glow effect
        auto hoveredBorder = _normalStyle.borderColor;
        int br = std::min(255, (int)(hoveredBorder.getRed() * 1.3f));
        int bg = std::min(255, (int)(hoveredBorder.getGreen() * 1.3f));
        int bb = std::min(255, (int)(hoveredBorder.getBlue() * 1.3f));
        _hoveredStyle.borderColor = UIColor(br, bg, bb, hoveredBorder.getAlpha());
        _hoveredStyle.borderWidth = _normalStyle.borderWidth + 1.0f; // Slightly thicker border
        
        // Ensure text stays crisp white
        _hoveredStyle.textColor = UIColor(255, 255, 255, 255);

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

        // Update the normal state's style
        _normalStyle.backgroundColor = color;
        setStateStyle(ButtonState::NORMAL, _normalStyle);
        
        // Regenerate beautiful hover style based on new color
        _hoveredStyle = _normalStyle;
        int r = color.getRed();
        int g = color.getGreen();
        int b = color.getBlue();
        
        // Apply intelligent color boost (40% brighter)
        float boostFactor = 1.4f;
        r = std::min(255, (int)(r * boostFactor));
        g = std::min(255, (int)(g * boostFactor));
        b = std::min(255, (int)(b * boostFactor));
        
        _hoveredStyle.backgroundColor = UIColor(r, g, b, color.getAlpha());
        
        // Enhance border for glow effect
        auto hoveredBorder = _normalStyle.borderColor;
        int br = std::min(255, (int)(hoveredBorder.getRed() * 1.3f));
        int bg = std::min(255, (int)(hoveredBorder.getGreen() * 1.3f));
        int bb = std::min(255, (int)(hoveredBorder.getBlue() * 1.3f));
        _hoveredStyle.borderColor = UIColor(br, bg, bb, hoveredBorder.getAlpha());
        _hoveredStyle.borderWidth = _normalStyle.borderWidth + 1.0f;
        _hoveredStyle.textColor = UIColor(255, 255, 255, 255);
        
        setStateStyle(ButtonState::HOVERED, _hoveredStyle);

        // Apply the current state style immediately
        _m_style = getStateStyle(_state);
    }

    void ButtonWidget::setBorderColor(UIColor color)
    {
        Widget::setBorderColor(color);
        
        // Update the normal state's style
        _normalStyle.borderColor = color;
        setStateStyle(ButtonState::NORMAL, _normalStyle);
        
        // Update hover style border (enhanced version)
        int br = std::min(255, (int)(color.getRed() * 1.3f));
        int bg = std::min(255, (int)(color.getGreen() * 1.3f));
        int bb = std::min(255, (int)(color.getBlue() * 1.3f));
        _hoveredStyle.borderColor = UIColor(br, bg, bb, color.getAlpha());
        setStateStyle(ButtonState::HOVERED, _hoveredStyle);
        
        // Apply the current state style immediately
        _m_style = getStateStyle(_state);
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
        
        // Play click sound if enabled
        if (_soundEnabled && IsAudioDeviceReady()) {
            if (_hasCustomSound) {
                SetSoundVolume(_clickSound, s_soundVolume);
                PlaySound(_clickSound);
            } else if (!s_defaultSoundPath.empty()) {
                // Lazy load default sound if not already loaded
                if (!s_defaultSoundLoaded) {
                    initializeDefaultSound();
                }
                if (s_defaultSoundLoaded) {
                    SetSoundVolume(s_defaultClickSound, s_soundVolume);
                    PlaySound(s_defaultClickSound);
                }
            }
        }
        
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

    void ButtonWidget::setClickSound(const std::string& soundPath)
    {
        if (!IsAudioDeviceReady()) {
            std::cout << "Warning: Audio device not ready, cannot load click sound: " << soundPath << std::endl;
            return;
        }

        // Unload previous custom sound if any
        if (_hasCustomSound) {
            UnloadSound(_clickSound);
        }

        // Load new sound
        _clickSound = LoadSound(soundPath.c_str());
        if (_clickSound.frameCount > 0) {  // Check if sound loaded successfully
            _hasCustomSound = true;
            _soundPath = soundPath;
            std::cout << "Click sound loaded: " << soundPath << std::endl;
        } else {
            _hasCustomSound = false;
            std::cout << "Failed to load click sound: " << soundPath << std::endl;
        }
    }

    void ButtonWidget::setClickSound(const Sound& sound)
    {
        // Unload previous custom sound if any
        if (_hasCustomSound) {
            UnloadSound(_clickSound);
        }

        _clickSound = sound;
        _hasCustomSound = true;
        _soundPath = "[Direct Sound Object]";
    }

    void ButtonWidget::setClickSoundEnabled(bool enabled)
    {
        _soundEnabled = enabled;
    }

    bool ButtonWidget::isClickSoundEnabled() const
    {
        return _soundEnabled;
    }

    void ButtonWidget::initializeDefaultSound()
    {
        if (!IsAudioDeviceReady()) {
            std::cout << "Warning: Audio device not ready, cannot initialize default button sound" << std::endl;
            return;
        }

        if (!s_defaultSoundPath.empty()) {
            s_defaultClickSound = LoadSound(s_defaultSoundPath.c_str());
            if (s_defaultClickSound.frameCount > 0) {  // Check if sound loaded successfully
                s_defaultSoundLoaded = true;
                std::cout << "✓ Default button click sound loaded: " << s_defaultSoundPath << std::endl;
            } else {
                s_defaultSoundLoaded = false;
                std::cout << "✗ Failed to load default button click sound: " << s_defaultSoundPath << std::endl;
            }
        }
    }

    void ButtonWidget::setDefaultClickSound(const std::string& soundPath)
    {
        // Unload previous default sound if any
        if (s_defaultSoundLoaded) {
            UnloadSound(s_defaultClickSound);
            s_defaultSoundLoaded = false;
        }

        s_defaultSoundPath = soundPath;
        
        // Try to initialize immediately if audio device is ready
        // Otherwise, it will be loaded lazily on first button click
        if (IsAudioDeviceReady()) {
            initializeDefaultSound();
        } else {
            std::cout << "Audio device not ready yet, default sound will be loaded when first button is clicked" << std::endl;
        }
    }

    void ButtonWidget::setSoundVolume(float volume)
    {
        s_soundVolume = std::clamp(volume, 0.0f, 1.0f);
    }

    float ButtonWidget::getSoundVolume()
    {
        return s_soundVolume;
    }

    void ButtonWidget::renderSelf() const
    {
        auto UItransform = getAbsoluteTransform();
        
        rtype::ecs::RenderUtils::drawUiRect(UItransform, _m_style.backgroundColor);

        if (_m_style.borderWidth > 0.0f) {
            rtype::ecs::RenderUtils::drawUiRectOutline(UItransform, _m_style.borderWidth, _m_style.borderColor);
        }

        rtype::ecs::RenderUtils::drawUiText(_text, UItransform, _m_style.fontSize, _m_style.textColor, rtype::ecs::TextAlign::Center, rtype::ecs::VerticalAlign::Middle, _m_style.padding);
    }

} // namespace rtype::ui
