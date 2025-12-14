/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SliderWidget - Interactive slider implementation
*/

#include "SliderWidget.hpp"
#include "raylib.h"
#include <algorithm>

namespace rtype::ui {

    SliderWidget::SliderWidget(float value)
        : ProgressBarWidget(value) {
        // Set default label format to show percentage
        setLabelFormat("%.0f%%");
        setShowLabel(true);
        
        // Default handle color that matches the style
        _handleColor = {220, 220, 220, 255};
    }

    void SliderWidget::setOnValueChange(const ValueChangeCallback& callback) {
        _onValueChange = callback;
    }

    void SliderWidget::setRange(float min, float max) {
        _minValue = min;
        _maxValue = max;
    }

    float SliderWidget::getMinValue() const {
        return _minValue;
    }

    float SliderWidget::getMaxValue() const {
        return _maxValue;
    }

    void SliderWidget::setRangeValue(float value) {
        float clampedValue = std::clamp(value, _minValue, _maxValue);
        float normalizedValue = (_maxValue == _minValue) ? 0.0f : 
                               (clampedValue - _minValue) / (_maxValue - _minValue);
        setValue(normalizedValue);
    }

    float SliderWidget::getRangeValue() const {
        return _minValue + (_value * (_maxValue - _minValue));
    }

    void SliderWidget::setHandleColor(const Color& color) {
        _handleColor = color;
    }

    Color SliderWidget::getHandleColor() const {
        return _handleColor;
    }

    void SliderWidget::setHandleSize(float size) {
        _handleSize = size;
    }

    float SliderWidget::getHandleSize() const {
        return _handleSize;
    }

    void SliderWidget::setDraggable(bool draggable) {
        _draggable = draggable;
    }

    bool SliderWidget::isDraggable() const {
        return _draggable;
    }

    void SliderWidget::renderSelf() const {
        // Just render the progress bar - clean and simple
        ProgressBarWidget::renderSelf();
    }

    bool SliderWidget::onMouseClick() {
        if (!_draggable || !isEnabled()) return false;

        // UIManager already verified the click is on this widget
        // Use GetMousePosition but convert to local coordinates using absolute transform
        Vector2 mousePos = GetMousePosition();
        UITransform absTransform = getAbsoluteTransform();
        float localX = mousePos.x - absTransform.x;
        float localY = mousePos.y - absTransform.y;
        
        _isDragging = true;
        updateValueFromMouse(localX, localY);
        return true; // Consume the event
    }

    bool SliderWidget::onMouseMove(float x, float y) {
        if (!isEnabled()) return false;

        // Handle dragging anywhere on the slider
        if (_isDragging && _draggable) {
            updateValueFromMouse(x, y);
            return true; // Consume the event while dragging
        }

        return false;
    }

    bool SliderWidget::onMouseRelease() {
        if (_isDragging) {
            _isDragging = false;
            return true; // Consume the event
        }
        return false;
    }

    bool SliderWidget::onMouseEnter() {
        return false; // Don't consume, let parent handle
    }

    bool SliderWidget::onMouseLeave() {
        return false; // Don't consume, let parent handle
    }

    void SliderWidget::updateValueFromMouse(float mouseX, float mouseY) {
        const UITransform& transform = getTransform();
        
        float newValue;
        if (_orientation == ProgressOrientation::Horizontal) {
            // mouseX and mouseY are already relative to the widget
            newValue = (transform.width > 0) ? (mouseX / transform.width) : 0.0f;
        } else {
            // Handle vertical slider (inverted - top is max)
            newValue = (transform.height > 0) ? (1.0f - (mouseY / transform.height)) : 0.0f;
        }

        // Clamp value to valid range
        newValue = std::clamp(newValue, 0.0f, 1.0f);

        // Always update and call callback
        float oldRangeValue = getRangeValue();
        setValue(newValue);
        float newRangeValue = getRangeValue();
        
        // Call callback with the new range value
        if (_onValueChange) {
            if (std::abs(newRangeValue - oldRangeValue) > 1.0f) { // Less sensitive - only update every 1%
                _onValueChange(newRangeValue);
            }
        }
    }

    Rectangle SliderWidget::getHandleRect() const {
        const UITransform& transform = getTransform();
        
        if (_orientation == ProgressOrientation::Horizontal) {
            // Horizontal slider handle
            float handleX = transform.x + (_value * transform.width) - (_handleSize / 2.0f);
            float handleY = transform.y + (transform.height / 2.0f) - (_handleSize / 2.0f);
            return {handleX, handleY, _handleSize, _handleSize};
        } else {
            // Vertical slider handle
            float handleX = transform.x + (transform.width / 2.0f) - (_handleSize / 2.0f);
            float handleY = transform.y + ((1.0f - _value) * transform.height) - (_handleSize / 2.0f);
            return {handleX, handleY, _handleSize, _handleSize};
        }
    }

    bool SliderWidget::isMouseOverHandle(float mouseX, float mouseY) const {
        Rectangle handleRect = getHandleRect();
        return CheckCollisionPointRec({mouseX, mouseY}, handleRect);
    }

} // namespace rtype::ui