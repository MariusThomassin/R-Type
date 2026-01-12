#include "ErrorMessageWidget.hpp"
#include "../UIColor.hpp"
#include <raylib.h>
#include <iostream>

namespace rtype::ui {

    ErrorMessageWidget::ErrorMessageWidget(const std::string& message, const ErrorMessageConfig& config)
        : Widget()
        , config_(config)
        , message_(message)
        , retryEnabled_(false)
        , retryText_("RETRY")
        , initialized_(false)
        , autoCloseTimer_(0.0f)
    {
        // Set full screen size for modal overlay
        setSize(1200.0f, 650.0f);
        setPosition(-240.0f, 0.0f);
    }

    void ErrorMessageWidget::setCallbacks(const ErrorMessageCallbacks& callbacks) {
        callbacks_ = callbacks;
    }

    void ErrorMessageWidget::initialize() {
        if (initialized_) return;

        createBackground();
        createDialog();
        createButtons();

        initialized_ = true;
    }

    void ErrorMessageWidget::createBackground() {
        // Semi-transparent background overlay
        backgroundPanel_ = std::make_shared<PanelWidget>();
        backgroundPanel_->setPosition(0.0f, 0.0f);
        backgroundPanel_->setSize(1280.0f, 720.0f);
        backgroundPanel_->setBackgroundColor(UIColor(0, 0, 0, 180)); // Semi-transparent black
        backgroundPanel_->setBorderWidth(0.0f);
        addChild(backgroundPanel_);
    }

    void ErrorMessageWidget::createDialog() {
        // Main dialog panel
        dialogPanel_ = std::make_shared<PanelWidget>();
        dialogPanel_->setPosition(340.0f, 240.0f); // Centered
        dialogPanel_->setSize(600.0f, 240.0f);
        dialogPanel_->setBackgroundColor(UIColor(40, 20, 20, 255)); // Dark red background
        dialogPanel_->setBorderColor(UIColor(255, 100, 100, 255)); // Red border
        dialogPanel_->setBorderWidth(3.0f);
        backgroundPanel_->addChild(dialogPanel_);

        auto dialogBounds = dialogPanel_->getContentBounds();

        // Error title
        titleText_ = std::make_shared<TextWidget>(config_.title, config_.titleFontSize);
        titleText_->setPosition(dialogBounds.width / 2.0f - 60, 20.0f);
        titleText_->setSize(120.0f, 40.0f);
        titleText_->setBackgroundColor(UIColor::Transparent());
        titleText_->setTextColor(UIColor(255, 150, 150, 255)); // Light red
        dialogPanel_->addChild(titleText_);

        // Error message
        messageText_ = std::make_shared<TextWidget>(message_, config_.textFontSize);
        messageText_->setPosition(20.0f, 70.0f);
        messageText_->setSize(dialogBounds.width - 40.0f, 80.0f);
        messageText_->setBackgroundColor(UIColor::Transparent());
        messageText_->setTextColor(UIColor(255, 255, 255, 255)); // White text
        dialogPanel_->addChild(messageText_);
    }

    void ErrorMessageWidget::createButtons() {
        auto dialogBounds = dialogPanel_->getContentBounds();

        // OK button (always present)
        float buttonY = dialogBounds.height - 60.0f;
        
        if (retryEnabled_) {
            // Two buttons: Retry and OK
            retryButton_ = std::make_shared<ButtonWidget>(retryText_);
            retryButton_->setPosition(100.0f, buttonY);
            retryButton_->setSize(140.0f, 40.0f);
            retryButton_->setBackgroundColor(UIColor(0, 100, 200, 200)); // Blue
            retryButton_->setBorderColor(UIColor(0, 150, 255, 255));
            retryButton_->setBorderWidth(2.0f);
            retryButton_->setTextColor(UIColor::White());
            retryButton_->setOnClick([this]() {
                std::cout << "Error dialog retry clicked" << std::endl;
                if (callbacks_.onRetry) {
                    callbacks_.onRetry();
                }
                hide();
            });
            dialogPanel_->addChild(retryButton_);

            okButton_ = std::make_shared<ButtonWidget>("OK");
            okButton_->setPosition(360.0f, buttonY);
            okButton_->setSize(140.0f, 40.0f);
        } else {
            // Single OK button (centered)
            okButton_ = std::make_shared<ButtonWidget>("OK");
            okButton_->setPosition(dialogBounds.width / 2.0f - 70.0f, buttonY);
            okButton_->setSize(140.0f, 40.0f);
        }

        okButton_->setBackgroundColor(UIColor(100, 100, 100, 200)); // Gray
        okButton_->setBorderColor(UIColor(150, 150, 150, 255));
        okButton_->setBorderWidth(2.0f);
        okButton_->setTextColor(UIColor::White());
        okButton_->setOnClick([this]() {
            std::cout << "Error dialog OK clicked" << std::endl;
            if (callbacks_.onClose) {
                callbacks_.onClose();
            }
            hide();
        });
        dialogPanel_->addChild(okButton_);
    }

    void ErrorMessageWidget::show() {
        setVisible(true);
        autoCloseTimer_ = config_.autoCloseDelay;
    }

    void ErrorMessageWidget::hide() {
        setVisible(false);
    }

    bool ErrorMessageWidget::isVisible() const {
        return Widget::isVisible();
    }

    void ErrorMessageWidget::update(float deltaTime) {
        Widget::update(deltaTime);
        
        if (isVisible() && config_.autoCloseDelay > 0.0f) {
            autoCloseTimer_ -= deltaTime;
            if (autoCloseTimer_ <= 0.0f) {
                hide();
                if (callbacks_.onClose) {
                    callbacks_.onClose();
                }
            }
        }
    }

    void ErrorMessageWidget::renderSelf() const {
        // The panels and children handle their own rendering
    }

    void ErrorMessageWidget::setMessage(const std::string& message) {
        message_ = message;
        if (messageText_) {
            messageText_->setText(message);
        }
    }

    void ErrorMessageWidget::setRetryEnabled(bool enabled, const std::string& retryText) {
        retryEnabled_ = enabled;
        retryText_ = retryText;
        
        // Recreate buttons if already initialized
        if (initialized_) {
            if (retryButton_) {
                dialogPanel_->removeChild(retryButton_);
                retryButton_.reset();
            }
            if (okButton_) {
                dialogPanel_->removeChild(okButton_);
                okButton_.reset();
            }
            createButtons();
        }
    }

} // namespace rtype::ui