/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** KeyBindingWidget - Implementation
*/

#include "KeyBindingWidget.hpp"
#include <raylib.h>
#include <algorithm>
#include <iostream>

namespace rtype::ui {

    KeyBindingWidget::KeyBindingWidget(const std::string& actionName, rtype::ecs::events::KeyCode currentKey)
        : _actionName(actionName)
        , _currentKey(currentKey)
        , _waitingForInput(false)
        , _blinkTimer(0.0f)
        , _onKeyChange(nullptr)
    {
        // Set default size and style
        setSize(300.0f, 40.0f);
        setBackgroundColor(UIColor(40, 40, 60, 200));
        setBorderColor(UIColor(100, 150, 255, 255));
        setBorderWidth(2.0f);
    }

    void KeyBindingWidget::setOnKeyChange(const KeyChangeCallback& callback) {
        _onKeyChange = callback;
    }

    void KeyBindingWidget::setCurrentKey(rtype::ecs::events::KeyCode newKey) {
        _currentKey = newKey;
        _waitingForInput = false;
    }

    rtype::ecs::events::KeyCode KeyBindingWidget::getCurrentKey() const {
        return _currentKey;
    }

    const std::string& KeyBindingWidget::getActionName() const {
        return _actionName;
    }

    void KeyBindingWidget::setWaitingForInput(bool waiting) {
        _waitingForInput = waiting;
        if (waiting) {
            _blinkTimer = 0.0f;
        }
    }

    bool KeyBindingWidget::isWaitingForInput() const {
        return _waitingForInput;
    }

    bool KeyBindingWidget::onMouseClick() {
        if (!isEnabled()) return false;
        
        setWaitingForInput(true);
        std::cout << "KeyBindingWidget: Waiting for input for " << _actionName << std::endl;
        return true; // Consume the event
    }

    bool KeyBindingWidget::onKeyPress(rtype::ecs::events::KeyCode key) {
        if (!_waitingForInput || !isEnabled()) return false;

        // Don't allow escape key to be bound (reserved for canceling)
        if (key == rtype::ecs::events::KeyCode::Escape) {
            setWaitingForInput(false);
            return true; // Consume but don't change binding
        }

        // Update the binding
        setCurrentKey(key);
        
        // Notify callback if set
        if (_onKeyChange) {
            _onKeyChange(key);
        }
        
        std::cout << "KeyBindingWidget: " << _actionName << " bound to " << keyCodeToString(key) << std::endl;
        return true; // Consume the event
    }

    void KeyBindingWidget::update(float deltaTime) {
        Widget::update(deltaTime);
        
        if (_waitingForInput) {
            _blinkTimer += deltaTime;
            if (_blinkTimer >= 2.0f) { // Reset after 2 seconds
                _blinkTimer = 0.0f;
            }
        }
    }

    void KeyBindingWidget::renderSelf() const {
        UITransform transform = getAbsoluteTransform();
        Rectangle rect = {transform.x, transform.y, transform.width, transform.height};
        
        // Get style colors
        const UIStyle& style = getStyle();
        UIColor bgColor = style.backgroundColor;
        
        if (_waitingForInput) {
            // Pulse effect when waiting for input
            float pulse = (sin(_blinkTimer * 6.0f) + 1.0f) * 0.5f;
            uint32_t newR = static_cast<uint32_t>(bgColor.getRed() + (255 - bgColor.getRed()) * pulse * 0.3f);
            uint32_t newG = static_cast<uint32_t>(bgColor.getGreen() + (255 - bgColor.getGreen()) * pulse * 0.3f);
            bgColor = UIColor(newR, newG, bgColor.getBlue(), bgColor.getAlpha());
        }
        
        // Draw background
        DrawRectangleRec(rect, bgColor.toRaylib());
        
        // Draw border
        UIColor borderColor = style.borderColor;
        if (_waitingForInput) {
            borderColor = UIColor(255, 200, 100, borderColor.getAlpha()); // Orange highlight when waiting
        }
        DrawRectangleLinesEx(rect, style.borderWidth, borderColor.toRaylib());
        
        // Draw text
        std::string displayText = getDisplayText();
        UIColor textColor = style.textColor;
        
        // Calculate text position (left-aligned action name, right-aligned key)
        float padding = 10.0f;
        float textY = transform.y + (transform.height - 20) * 0.5f; // Centered vertically
        
        // Draw action name on the left
        DrawText(_actionName.c_str(), static_cast<int>(transform.x + padding), static_cast<int>(textY), 
                 20, textColor.toRaylib());
        
        // Draw key binding on the right
        int keyTextWidth = MeasureText(displayText.c_str(), 20);
        float keyTextX = transform.x + transform.width - keyTextWidth - padding;
        DrawText(displayText.c_str(), static_cast<int>(keyTextX), static_cast<int>(textY), 
                 20, textColor.toRaylib());
    }

    std::string KeyBindingWidget::keyCodeToString(rtype::ecs::events::KeyCode key) const {
        return rtype::ecs::events::InputUtils::keyCodeToString(key);
    }

    std::string KeyBindingWidget::getDisplayText() const {
        if (_waitingForInput) {
            // Blink between "Press any key..." and "Press ESC to cancel"
            float blinkCycle = fmod(_blinkTimer, 1.0f);
            if (blinkCycle < 0.5f) {
                return "Press any key...";
            } else {
                return "ESC to cancel";
            }
        } else {
            return keyCodeToString(_currentKey);
        }
    }
} // namespace rtype::ui
