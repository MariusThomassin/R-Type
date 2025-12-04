/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** ProgressBarWidget implementation
*/

#include "ProgressBarWidget.hpp"
#include <raylib.h>
#include <algorithm>
#include <cstdio>

namespace rtype::ui {

    ProgressBarWidget::ProgressBarWidget(float value)
        : _value(std::clamp(value, 0.0f, 1.0f))
    {
        // Default progress bar styling
        _m_style.backgroundColor = UIColor(40, 40, 50, 255);
        _m_style.borderColor = UIColor(80, 80, 100, 255);
        _m_style.borderWidth = 1.0f;
        _m_transform.width = 200.0f;
        _m_transform.height = 20.0f;
    }

    void ProgressBarWidget::setValue(float value)
    {
        _value = std::clamp(value, 0.0f, 1.0f);
    }

    float ProgressBarWidget::getValue() const
    {
        return _value;
    }

    void ProgressBarWidget::setFillColor(const Color& color)
    {
        _fillColor = color;
    }

    Color ProgressBarWidget::getFillColor() const
    {
        return _fillColor;
    }

    void ProgressBarWidget::setOrientation(ProgressOrientation orientation)
    {
        _orientation = orientation;
    }

    ProgressOrientation ProgressBarWidget::getOrientation() const
    {
        return _orientation;
    }

    void ProgressBarWidget::setShowLabel(bool showLabel)
    {
        _showLabel = showLabel;
    }

    bool ProgressBarWidget::isShowLabel() const
    {
        return _showLabel;
    }

    void ProgressBarWidget::setLabelFormat(const std::string& format)
    {
        _labelFormat = format;
    }

    const std::string& ProgressBarWidget::getLabelFormat() const
    {
        return _labelFormat;
    }

    void ProgressBarWidget::renderSelf() const
    {
        auto transform = getAbsoluteTransform();

        // Draw background
        DrawRectangle(
            static_cast<int>(transform.x),
            static_cast<int>(transform.y),
            static_cast<int>(transform.width),
            static_cast<int>(transform.height),
            _m_style.backgroundColor.toRaylib()
        );

        // Calculate fill dimensions
        float fillX = transform.x + _m_style.borderWidth;
        float fillY = transform.y + _m_style.borderWidth;
        float fillWidth = transform.width - 2 * _m_style.borderWidth;
        float fillHeight = transform.height - 2 * _m_style.borderWidth;

        if (_orientation == ProgressOrientation::Horizontal) {
            fillWidth *= _value;
        } else {
            // For vertical, fill from bottom to top
            float fullHeight = fillHeight;
            fillHeight *= _value;
            fillY += fullHeight - fillHeight;
        }

        // Draw fill
        if (_value > 0.0f) {
            DrawRectangle(
                static_cast<int>(fillX),
                static_cast<int>(fillY),
                static_cast<int>(fillWidth),
                static_cast<int>(fillHeight),
                _fillColor
            );
        }

        // Draw border
        if (_m_style.borderWidth > 0) {
            DrawRectangleLinesEx(
                Rectangle{transform.x, transform.y, transform.width, transform.height},
                _m_style.borderWidth,
                _m_style.borderColor.toRaylib()
            );
        }

        // Draw label
        if (_showLabel) {
            char labelBuffer[32];
            snprintf(labelBuffer, sizeof(labelBuffer), _labelFormat.c_str(), _value * 100.0f);

            int fontSize = static_cast<int>(_m_style.fontSize);
            int textWidth = MeasureText(labelBuffer, fontSize);

            // Center label in the bar
            float textX = transform.x + (transform.width - textWidth) / 2.0f;
            float textY = transform.y + (transform.height - fontSize) / 2.0f;

            DrawText(
                labelBuffer,
                static_cast<int>(textX),
                static_cast<int>(textY),
                fontSize,
                _m_style.textColor.toRaylib()
            );
        }
    }

} // namespace rtype::ui
