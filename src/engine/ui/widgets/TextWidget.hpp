/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** TextWidget - A simple text display widget
*/

#ifndef TEXTWIDGET_HPP_
#define TEXTWIDGET_HPP_

#include "../Widget.hpp"
#include <string>

namespace rtype::ui {

    /**
     * @brief Text alignment options
     */
    enum class TextAlign {
        Left,
        Center,
        Right
    };

    /**
     * @brief Vertical alignment options
     */
    enum class VerticalAlign {
        Top,
        Middle,
        Bottom
    };

    /**
     * @brief A widget that displays text
     * 
     * TextWidget renders a text string with configurable font size, color,
     * and alignment. It automatically handles text positioning based on
     * the widget bounds.
     */
    class TextWidget : public Widget {
    public:
        /**
         * @brief Construct a new TextWidget
         * @param text The text to display
         * @param fontSize Size of the font
         */
        explicit TextWidget(const std::string& text = "", size_t fontSize = DEFAULT_FONT_SIZE);

        ~TextWidget() override = default;

        /**
         * @brief Set the displayed text
         * @param text The new text to display
         */
        void setText(const std::string& text);

        /**
         * @brief Get the current text
         * @return The displayed text
         */
        const std::string& getText() const;

        /**
         * @brief Set horizontal text alignment
         * @param align The alignment mode
         */
        void setTextAlign(TextAlign align);

        /**
         * @brief Get horizontal text alignment
         * @return Current alignment mode
         */
        TextAlign getTextAlign() const;

        /**
         * @brief Set vertical text alignment
         * @param align The vertical alignment mode
         */
        void setVerticalAlign(VerticalAlign align);

        /**
         * @brief Get vertical text alignment
         * @return Current vertical alignment mode
         */
        VerticalAlign getVerticalAlign() const;

        /**
         * @brief Enable or disable word wrapping
         * @param wrap True to enable word wrapping
         */
        void setWordWrap(bool wrap);

        /**
         * @brief Check if word wrapping is enabled
         * @return True if word wrapping is enabled
         */
        bool getWordWrap() const;

        /**
         * @brief Render the text widget
         */
        void renderSelf() const override;

    protected:
        std::string _text;
        TextAlign _textAlign = TextAlign::Left;
        VerticalAlign _verticalAlign = VerticalAlign::Top;
        bool _wordWrap = false;
    };

} // namespace rtype::ui

#endif /* !TEXTWIDGET_HPP_ */
