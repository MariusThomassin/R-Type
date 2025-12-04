/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TextWidget implementation
*/

#include "TextWidget.hpp"

namespace rtype::ui {

    TextWidget::TextWidget(const std::string& text, size_t fontSize, TextAlign align, VerticalAlign verticalAlign)
        : _text(text), _textAlign(align), _verticalAlign(verticalAlign)
    {
        _m_style.fontSize = fontSize;
        _m_style.backgroundColor = UIColor::Transparent();
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

    void TextWidget::renderSelf() const
    {
        if (_text.empty()) return;

        auto transform = getAbsoluteTransform();
        int fontSize = static_cast<int>(_m_style.fontSize);

        if (_m_style.backgroundColor.getAlpha() > 0) {
            rtype::ecs::RenderUtils::drawUiRect(transform, _m_style.backgroundColor);
        }

        if (_m_style.borderWidth > 0) {
            rtype::ecs::RenderUtils::drawUiRectOutline(transform, _m_style.borderWidth, _m_style.borderColor);
        }

        rtype::ecs::RenderUtils::drawUiText(_text, transform, _m_style.fontSize, _m_style.textColor, _textAlign, _verticalAlign, _m_style.padding);

    }

} // namespace rtype::ui
