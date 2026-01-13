#pragma once

#include "../Widget.hpp"
#include "PanelWidget.hpp"
#include "ButtonWidget.hpp"
#include "TextWidget.hpp"
#include <memory>
#include <functional>
#include <string>

namespace rtype::ui {

/**
 * @brief Configuration for error messages
 */
struct ErrorMessageConfig {
    std::string title = "ERROR";
    int titleFontSize = 32;
    int textFontSize = 18;
    float autoCloseDelay = 0.0f; // 0 = no auto-close
};

/**
 * @brief Callback for error message events
 */
struct ErrorMessageCallbacks {
    std::function<void()> onClose;
    std::function<void()> onRetry; // Optional retry action
};

/**
 * @brief Modal error message dialog widget
 * 
 * Displays error messages with optional retry functionality
 * Can be auto-closing or require manual dismissal
 */
class ErrorMessageWidget : public Widget {
    public:
        /**
         * @brief Constructor
         * @param message The error message to display
         * @param config Configuration for the error dialog
         */
        explicit ErrorMessageWidget(const std::string& message, const ErrorMessageConfig& config = ErrorMessageConfig{});

        /**
         * @brief Set callback functions
         * @param callbacks Callback functions for events
         */
        void setCallbacks(const ErrorMessageCallbacks& callbacks);

        /**
         * @brief Initialize the widget
         */
        void initialize();

        /**
         * @brief Show the error message
         */
        void show();

        /**
         * @brief Hide the error message
         */
        void hide();

        /**
         * @brief Check if visible
         */
        bool isVisible() const;

        /**
         * @brief Update for auto-close timer
         * @param deltaTime Time since last update
         */
        void update(float deltaTime) override;

        /**
         * @brief Render the error dialog
         */
        void renderSelf() const override;

        /**
         * @brief Set the error message
         * @param message New error message
         */
        void setMessage(const std::string& message);

        /**
         * @brief Enable/disable retry button
         * @param enabled Whether to show retry button
         * @param retryText Text for retry button
         */
        void setRetryEnabled(bool enabled, const std::string& retryText = "RETRY");

    private:
        // Configuration
        ErrorMessageConfig config_;
        ErrorMessageCallbacks callbacks_;
        std::string message_;
        bool retryEnabled_;
        std::string retryText_;

        // UI Components
        std::shared_ptr<PanelWidget> backgroundPanel_;
        std::shared_ptr<PanelWidget> dialogPanel_;
        std::shared_ptr<TextWidget> titleText_;
        std::shared_ptr<TextWidget> messageText_;
        std::shared_ptr<ButtonWidget> okButton_;
        std::shared_ptr<ButtonWidget> retryButton_;

        // State
        bool initialized_;
        float autoCloseTimer_;

        /**
         * @brief Create the background overlay
         */
        void createBackground();

        /**
         * @brief Create the dialog panel
         */
        void createDialog();

        /**
         * @brief Create dialog buttons
         */
        void createButtons();
    };
} // namespace rtype::ui
