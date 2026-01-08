#pragma once

#include "../Widget.hpp"
#include <string>
#include <functional>

namespace rtype::ui {

/**
 * @brief Simple text input field widget
 */
class InputFieldWidget : public Widget {
public:
    using TextChangedCallback = std::function<void(const std::string&)>;

    /**
     * @brief Constructor
     * @param placeholder Placeholder text when empty
     */
    explicit InputFieldWidget(const std::string& placeholder = "");

    /**
     * @brief Set the text content
     * @param text New text content
     */
    void setText(const std::string& text);

    /**
     * @brief Get the current text content
     * @return Current text content
     */
    const std::string& getText() const;

    /**
     * @brief Set placeholder text
     * @param placeholder Placeholder text
     */
    void setPlaceholder(const std::string& placeholder);

    /**
     * @brief Set callback for text changes
     * @param callback Function called when text changes
     */
    void setOnTextChanged(const TextChangedCallback& callback);

    /**
     * @brief Set if the field is focused
     * @param focused Focus state
     */
    void setFocused(bool focused);

    /**
     * @brief Check if the field is focused
     * @return True if focused
     */
    bool isFocused() const;

    /**
     * @brief Render the input field
     */
    void renderSelf() const override;

    /**
     * @brief Handle mouse click events
     */
    bool onMouseClick() override;

    /**
     * @brief Update the widget (handle text input)
     * @param deltaTime Time since last update
     */
    void update(float deltaTime) override;

private:
    std::string text_;
    std::string placeholder_;
    bool focused_;
    float cursorBlink_;
    TextChangedCallback onTextChanged_;

    /**
     * @brief Process keyboard input
     */
    void processKeyboardInput();
};

} // namespace rtype::ui