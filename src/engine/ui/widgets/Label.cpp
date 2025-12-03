/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Label
*/

#include "Label.hpp"

namespace rtype::ui {
    Label::Label(const std::string& text, float fontSize) : _m_text(text)
    {
        setFontSize(static_cast<size_t>(fontSize));
    }

    void Label::setText(const std::string& text)
    {
        _m_text = text;
    }

    const std::string& Label::getText() const
    {
        return _m_text;
    }

    void Label::setFontSize(size_t size)
    {
        Widget::setFontSize(size);
    }

    size_t Label::getFontSize() const
    {
        return getStyle().fontSize;
    }

    void Label::update(float deltaTime)
    {
        // std::cout << "Label has been updated !" << std::endl;
        (void)deltaTime;
    }

    void Label::renderSelf(const rtype::ecs::RenderContext& ctx) const
    {
        UITransform absTransform = getAbsoluteTransform();
        const UIStyle& style = getStyle();

        if (style.backgroundColor.getAlpha() > 0) {
            rtype::ecs::RenderUtils::drawUiRect(absTransform, style.backgroundColor);
        }

        rtype::ecs::RenderUtils::drawUiCenteredText(_m_text, absTransform, style.fontSize, style.textColor);
    }
} // namespace rtype::ui
