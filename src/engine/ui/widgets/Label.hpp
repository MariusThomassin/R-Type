/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** Label
*/

#ifndef LABEL_HPP_
#define LABEL_HPP_

#include <iostream>
#include "../Widget.hpp"
#include "../../graphics/RenderUtils.hpp"

#define DEFAULT_TEXT "Text here"

namespace rtype::ui {
    class Label : public Widget {
        public:
            Label(const std::string& text = DEFAULT_TEXT, float fontSize = DEFAULT_FONT_SIZE);
            ~Label() = default;

            void setText(const std::string& text);
            const std::string& getText() const;

            void setFontSize(size_t size);
            size_t getFontSize() const;

            void update(float deltaTime) override;
            void renderSelf(const rtype::ecs::RenderContext& ctx) const override;

        private:
            std::string _m_text;
    };
}

#endif /* !LABEL_HPP_ */
