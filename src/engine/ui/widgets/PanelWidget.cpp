/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** PanelWidget implementation
*/

#include "PanelWidget.hpp"
#include <raylib.h>

namespace rtype::ui {

    PanelWidget::PanelWidget()
    {
        // Default panel styling
        _m_style.backgroundColor = UIColor(40, 40, 50, 220);
        _m_style.borderColor = UIColor(80, 80, 100, 255);
        _m_style.borderWidth = 1.0f;
        _m_transform.width = 200.0f;
        _m_transform.height = 150.0f;
    }

    void PanelWidget::setHasHeader(bool hasHeader)
    {
        _hasHeader = hasHeader;
    }

    bool PanelWidget::hasHeader() const
    {
        return _hasHeader;
    }

    void PanelWidget::setTitle(const std::string& title)
    {
        _title = title;
        _hasHeader = true; // Setting title implies header
    }

    const std::string& PanelWidget::getTitle() const
    {
        return _title;
    }

    void PanelWidget::setHeaderHeight(float height)
    {
        _headerHeight = height;
    }

    float PanelWidget::getHeaderHeight() const
    {
        return _headerHeight;
    }

    void PanelWidget::setHeaderColor(const Color& color)
    {
        _headerColor = color;
    }

    Color PanelWidget::getHeaderColor() const
    {
        return _headerColor;
    }

    UITransform PanelWidget::getContentBounds() const
    {
        auto transform = getAbsoluteTransform();
        UITransform content;
        content.x = transform.x + _m_style.padding;
        content.y = transform.y + _m_style.padding;
        content.width = transform.width - 2 * _m_style.padding;
        content.height = transform.height - 2 * _m_style.padding;

        if (_hasHeader) {
            content.y += _headerHeight;
            content.height -= _headerHeight;
        }

        return content;
    }

    void PanelWidget::renderSelf() const
    {
        auto transform = getAbsoluteTransform();

        // Draw main background
        DrawRectangle(
            static_cast<int>(transform.x),
            static_cast<int>(transform.y),
            static_cast<int>(transform.width),
            static_cast<int>(transform.height),
            _m_style.backgroundColor.toRaylib()
        );

        // Draw header if enabled
        if (_hasHeader) {
            // Header background
            DrawRectangle(
                static_cast<int>(transform.x),
                static_cast<int>(transform.y),
                static_cast<int>(transform.width),
                static_cast<int>(_headerHeight),
                _headerColor
            );

            // Header separator line
            DrawLine(
                static_cast<int>(transform.x),
                static_cast<int>(transform.y + _headerHeight),
                static_cast<int>(transform.x + transform.width),
                static_cast<int>(transform.y + _headerHeight),
                _m_style.borderColor.toRaylib()
            );

            // Title text
            if (!_title.empty()) {
                int fontSize = static_cast<int>(_m_style.fontSize);
                int textWidth = MeasureText(_title.c_str(), fontSize);
                
                // Center title in header
                float textX = transform.x + (transform.width - textWidth) / 2.0f;
                float textY = transform.y + (_headerHeight - fontSize) / 2.0f;

                DrawText(
                    _title.c_str(),
                    static_cast<int>(textX),
                    static_cast<int>(textY),
                    fontSize,
                    _m_style.textColor.toRaylib()
                );
            }
        }

        // Draw border
        if (_m_style.borderWidth > 0) {
            DrawRectangleLinesEx(
                Rectangle{transform.x, transform.y, transform.width, transform.height},
                _m_style.borderWidth,
                _m_style.borderColor.toRaylib()
            );
        }
    }

} // namespace rtype::ui
