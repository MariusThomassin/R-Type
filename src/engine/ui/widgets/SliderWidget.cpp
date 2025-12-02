/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** SliderWidget implementation
*/

#include "SliderWidget.hpp"
#include <raylib.h>
#include <algorithm>
#include <cstdio>

namespace rtype::ui {

    SliderWidget::SliderWidget(float minValue, float maxValue, float value)
        : _minValue(minValue), _maxValue(maxValue)
    {
        _m_transform.width = 200.0f;
        _m_transform.height = 24.0f;
        setValue(value);
    }

    void SliderWidget::setValue(float value)
    {
        float newValue = std::clamp(value, _minValue, _maxValue);
        if (_value != newValue) {
            _value = newValue;
            if (_onChange && !_dragging) {
                _onChange(_value);
            }
        }
    }

    float SliderWidget::getValue() const
    {
        return _value;
    }

    void SliderWidget::setRange(float minValue, float maxValue)
    {
        _minValue = minValue;
        _maxValue = maxValue;
        setValue(_value);  // Reclamp current value
    }

    float SliderWidget::getMinValue() const
    {
        return _minValue;
    }

    float SliderWidget::getMaxValue() const
    {
        return _maxValue;
    }

    void SliderWidget::setOnChange(ValueCallback callback)
    {
        _onChange = std::move(callback);
    }

    void SliderWidget::setTrackColor(const Color& color)
    {
        _trackColor = color;
    }

    void SliderWidget::setFillColor(const Color& color)
    {
        _fillColor = color;
    }

    void SliderWidget::setThumbColor(const Color& color)
    {
        _thumbColor = color;
    }

    void SliderWidget::setThumbSize(float width, float height)
    {
        _thumbWidth = width;
        _thumbHeight = height;
    }

    void SliderWidget::setShowLabel(bool show)
    {
        _showLabel = show;
    }

    void SliderWidget::setLabelFormat(const std::string& format)
    {
        _labelFormat = format;
    }

    float SliderWidget::valueFromX(float x) const
    {
        float trackWidth = _m_transform.width - _thumbWidth;
        float normalizedX = (x - _thumbWidth / 2.0f) / trackWidth;
        normalizedX = std::clamp(normalizedX, 0.0f, 1.0f);
        return _minValue + normalizedX * (_maxValue - _minValue);
    }

    float SliderWidget::xFromValue() const
    {
        float trackWidth = _m_transform.width - _thumbWidth;
        float normalized = (_value - _minValue) / (_maxValue - _minValue);
        return _thumbWidth / 2.0f + normalized * trackWidth;
    }

    bool SliderWidget::onMouseClick(float x, float y)
    {
        (void)y;
        if (!_m_enabled) return false;

        _dragging = true;
        auto transform = getAbsoluteTransform();
        float localX = x - transform.x;
        float newValue = valueFromX(localX);
        
        if (_value != newValue) {
            _value = newValue;
            if (_onChange) {
                _onChange(_value);
            }
        }
        return true;
    }

    bool SliderWidget::onMouseRelease(float x, float y)
    {
        (void)x;
        (void)y;
        if (_dragging) {
            _dragging = false;
            if (_onChange) {
                _onChange(_value);
            }
        }
        return true;
    }

    bool SliderWidget::onMouseMove(float x, float y)
    {
        (void)y;
        if (!_m_enabled || !_dragging) return false;

        auto transform = getAbsoluteTransform();
        float localX = x - transform.x;
        float newValue = valueFromX(localX);
        
        if (_value != newValue) {
            _value = newValue;
            if (_onChange) {
                _onChange(_value);
            }
        }
        return true;
    }

    void SliderWidget::renderSelf()
    {
        auto transform = getAbsoluteTransform();
        
        float trackHeight = 6.0f;
        float trackY = transform.y + (transform.height - trackHeight) / 2.0f;

        // Draw track background
        DrawRectangle(
            static_cast<int>(transform.x),
            static_cast<int>(trackY),
            static_cast<int>(transform.width),
            static_cast<int>(trackHeight),
            _trackColor.toRaylib()
        );

        // Draw fill (left of thumb)
        float thumbX = xFromValue();
        DrawRectangle(
            static_cast<int>(transform.x),
            static_cast<int>(trackY),
            static_cast<int>(thumbX),
            static_cast<int>(trackHeight),
            _fillColor.toRaylib()
        );

        // Draw thumb
        float thumbY = transform.y + (transform.height - _thumbHeight) / 2.0f;
        Color thumbCol = _dragging ? _thumbColor.withAlpha(255) : 
                        (_m_enabled ? _thumbColor : _thumbColor.withAlpha(128));
        
        DrawRectangle(
            static_cast<int>(transform.x + thumbX - _thumbWidth / 2.0f),
            static_cast<int>(thumbY),
            static_cast<int>(_thumbWidth),
            static_cast<int>(_thumbHeight),
            thumbCol.toRaylib()
        );

        // Draw label if enabled
        if (_showLabel) {
            char buf[32];
            snprintf(buf, sizeof(buf), _labelFormat.c_str(), _value);
            int textWidth = MeasureText(buf, 12);
            DrawText(
                buf,
                static_cast<int>(transform.x + transform.width + 8),
                static_cast<int>(transform.y + (transform.height - 12) / 2.0f),
                12,
                _m_style.textColor.toRaylib()
            );
        }
    }

} // namespace rtype::ui
