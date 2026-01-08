#include "InputFieldWidget.hpp"
#include "../UIColor.hpp"
#include <raylib.h>
#include <algorithm>
#include <iostream>

namespace rtype::ui {

InputFieldWidget::InputFieldWidget(const std::string& placeholder)
    : Widget()
    , placeholder_(placeholder)
    , focused_(false)
    , cursorBlink_(0.0f)
{
    // Default styling for input field
    setSize(200.0f, 40.0f);
    setBackgroundColor(UIColor(40, 40, 50, 255));
    setBorderColor(UIColor(80, 80, 100, 255));
    setBorderWidth(2.0f);
    setTextColor(UIColor(255, 255, 255, 255));
}

void InputFieldWidget::setText(const std::string& text) {
    text_ = text;
    if (onTextChanged_) {
        onTextChanged_(text_);
    }
}

const std::string& InputFieldWidget::getText() const {
    return text_;
}

void InputFieldWidget::setPlaceholder(const std::string& placeholder) {
    placeholder_ = placeholder;
}

void InputFieldWidget::setOnTextChanged(const TextChangedCallback& callback) {
    onTextChanged_ = callback;
}

void InputFieldWidget::setFocused(bool focused) {
    focused_ = focused;
}

bool InputFieldWidget::isFocused() const {
    return focused_;
}

void InputFieldWidget::renderSelf() const {
    auto transform = getAbsoluteTransform();
    const auto& style = getStyle();

    // Draw background
    DrawRectangle(
        static_cast<int>(transform.x),
        static_cast<int>(transform.y),
        static_cast<int>(transform.width),
        static_cast<int>(transform.height),
        style.backgroundColor.toRaylib()
    );

    // Draw border with focus highlight
    Color borderColor = focused_ ? 
        Color{100, 150, 255, 255} : // Blue when focused
        style.borderColor.toRaylib();
    
    DrawRectangleLinesEx(
        Rectangle{transform.x, transform.y, transform.width, transform.height},
        style.borderWidth,
        borderColor
    );

    // Draw text or placeholder
    std::string displayText = text_.empty() ? placeholder_ : text_;
    Color textColor = text_.empty() ? 
        Color{150, 150, 150, 255} : // Gray for placeholder
        style.textColor.toRaylib();

    if (!displayText.empty()) {
        int fontSize = static_cast<int>(style.fontSize);
        int textWidth = MeasureText(displayText.c_str(), fontSize);
        
        // Left-align text with padding
        float textX = transform.x + 8.0f;
        float textY = transform.y + (transform.height - fontSize) / 2.0f;

        // Clip text if it's too long
        std::string clippedText = displayText;
        if (textWidth > transform.width - 16) {
            while (textWidth > transform.width - 16 && !clippedText.empty()) {
                clippedText.pop_back();
                textWidth = MeasureText(clippedText.c_str(), fontSize);
            }
        }

        DrawText(
            clippedText.c_str(),
            static_cast<int>(textX),
            static_cast<int>(textY),
            fontSize,
            textColor
        );
    }

    // Draw cursor when focused
    if (focused_) {
        int fontSize = static_cast<int>(style.fontSize);
        int textWidth = text_.empty() ? 0 : MeasureText(text_.c_str(), fontSize);
        
        float cursorX = transform.x + 8.0f + textWidth;
        float cursorY = transform.y + 4.0f;
        
        // Blinking cursor
        if (static_cast<int>(cursorBlink_ * 2) % 2 == 0) {
            DrawRectangle(
                static_cast<int>(cursorX),
                static_cast<int>(cursorY),
                2,
                static_cast<int>(transform.height - 8),
                Color{255, 255, 255, 255}
            );
        }
    }
}

bool InputFieldWidget::onMouseClick() {
    focused_ = true;
    return true; // Consume the event
}

void InputFieldWidget::update(float deltaTime) {
    Widget::update(deltaTime);
    
    if (focused_) {
        cursorBlink_ += deltaTime;
        processKeyboardInput();
    }
}

void InputFieldWidget::processKeyboardInput() {
    // Get character input
    int key = GetCharPressed();
    while (key > 0) {
        // Only allow printable ASCII characters
        if (key >= 32 && key <= 125) {
            text_ += static_cast<char>(key);
            if (onTextChanged_) {
                onTextChanged_(text_);
            }
        }
        key = GetCharPressed();
    }

    // Handle backspace
    if (IsKeyPressed(KEY_BACKSPACE) && !text_.empty()) {
        text_.pop_back();
        if (onTextChanged_) {
            onTextChanged_(text_);
        }
    }

    // Handle delete (clear entire text)
    if (IsKeyPressed(KEY_DELETE)) {
        text_.clear();
        if (onTextChanged_) {
            onTextChanged_(text_);
        }
    }

    // Handle enter (lose focus)
    if (IsKeyPressed(KEY_ENTER)) {
        focused_ = false;
    }

    // Handle escape (lose focus)
    if (IsKeyPressed(KEY_ESCAPE)) {
        focused_ = false;
    }

    // Handle tab (lose focus) 
    if (IsKeyPressed(KEY_TAB)) {
        focused_ = false;
    }
}

} // namespace rtype::ui