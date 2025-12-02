/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TextWidget implementation
*/

#include "TextWidget.hpp"
#include <raylib.h>

namespace rtype::ui {

    TextWidget::TextWidget(const std::string& text, size_t fontSize)
        : _text(text)
    {
        _m_style.fontSize = fontSize;
        _m_style.backgroundColor = Color::Transparent();
    }

    void TextWidget::setText(const std::string& text)
    {
        _text = text;
    }

    const std::string& TextWidget::getText() const
    {
        return _text;
    }

    void TextWidget::setTextAlign(TextAlign align)
    {
        _textAlign = align;
    }

    TextAlign TextWidget::getTextAlign() const
    {
        return _textAlign;
    }

    void TextWidget::setVerticalAlign(VerticalAlign align)
    {
        _verticalAlign = align;
    }

    VerticalAlign TextWidget::getVerticalAlign() const
    {
        return _verticalAlign;
    }

    void TextWidget::setWordWrap(bool wrap)
    {
        _wordWrap = wrap;
    }

    bool TextWidget::getWordWrap() const
    {
        return _wordWrap;
    }

    void TextWidget::renderSelf()
    {
        if (_text.empty()) return;

        auto transform = getAbsoluteTransform();
        int fontSize = static_cast<int>(_m_style.fontSize);
        
        // Draw background if not transparent
        if (_m_style.backgroundColor.getAlpha() > 0) {
            DrawRectangle(
                static_cast<int>(transform.x),
                static_cast<int>(transform.y),
                static_cast<int>(transform.width),
                static_cast<int>(transform.height),
                _m_style.backgroundColor.toRaylib()
            );
        }

        // Draw border if specified
        if (_m_style.borderWidth > 0) {
            DrawRectangleLinesEx(
                Rectangle{transform.x, transform.y, transform.width, transform.height},
                _m_style.borderWidth,
                _m_style.borderColor.toRaylib()
            );
        }

        // Measure text
        int textWidth = MeasureText(_text.c_str(), fontSize);
        int textHeight = fontSize;

        // Calculate X position based on alignment
        float textX = transform.x + _m_style.padding;
        float availableWidth = transform.width - 2 * _m_style.padding;

        switch (_textAlign) {
            case TextAlign::Center:
                textX = transform.x + (transform.width - textWidth) / 2.0f;
                break;
            case TextAlign::Right:
                textX = transform.x + transform.width - textWidth - _m_style.padding;
                break;
            case TextAlign::Left:
            default:
                break;
        }

        // Calculate Y position based on vertical alignment
        float textY = transform.y + _m_style.padding;
        float availableHeight = transform.height - 2 * _m_style.padding;

        switch (_verticalAlign) {
            case VerticalAlign::Middle:
                textY = transform.y + (transform.height - textHeight) / 2.0f;
                break;
            case VerticalAlign::Bottom:
                textY = transform.y + transform.height - textHeight - _m_style.padding;
                break;
            case VerticalAlign::Top:
            default:
                break;
        }

        DrawText(
            _text.c_str(),
            static_cast<int>(textX),
            static_cast<int>(textY),
            fontSize,
            _m_style.textColor.toRaylib()
        );
    }

} // namespace rtype::ui
