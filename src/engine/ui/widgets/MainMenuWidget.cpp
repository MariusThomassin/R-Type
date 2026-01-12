#include "MainMenuWidget.hpp"
#include "../UIColor.hpp"
#include <raylib.h>
#include <iostream>
#include <cmath>

namespace rtype::ui {

MainMenuWidget::MainMenuWidget(const MainMenuConfig& config)
    : Widget()
    , config_(config)
    , glowTime_(0.0f)
    , titlePulse_(1.0f)
    , initialized_(false)
{
    // Set default size and position (will be adjusted based on screen)
    setSize(500.0f, 500.0f);
}

void MainMenuWidget::setCallbacks(const MainMenuCallbacks& callbacks) {
    callbacks_ = callbacks;
}

void MainMenuWidget::initialize() {
    if (initialized_) return;

    createMenuPanel();
    createTitle();
    createButtons();

    initialized_ = true;
}

void MainMenuWidget::createMenuPanel() {
    menuPanel_ = std::make_shared<PanelWidget>();
    menuPanel_->setPosition(0.0f, 0.0f); // Relative to this widget
    auto transform = getTransform();
    menuPanel_->setSize(transform.width, transform.height);
    menuPanel_->setBackgroundColor(UIColor(0, 0, 0, 180)); // Semi-transparent black
    menuPanel_->setBorderColor(UIColor(100, 100, 255, 200)); // Blue border
    menuPanel_->setBorderWidth(2.0f);
    
    addChild(menuPanel_);
}

void MainMenuWidget::createTitle() {
    auto contentBounds = menuPanel_->getContentBounds();

    // Create title glow layers (back to front for layered effect)
    titleGlow2_ = std::make_shared<TextWidget>(config_.title, config_.titleFontSize);
    titleGlow2_->setPosition(contentBounds.width / 2.0f - 118, 44.0f);
    titleGlow2_->setSize(200.0f, 80.0f);
    titleGlow2_->setBackgroundColor(UIColor::Transparent());
    titleGlow2_->setTextColor(UIColor(255, 30, 30, 60)); // Very dim red glow
    menuPanel_->addChild(titleGlow2_);

    titleGlow1_ = std::make_shared<TextWidget>(config_.title, config_.titleFontSize);
    titleGlow1_->setPosition(contentBounds.width / 2.0f - 119, 46.0f);
    titleGlow1_->setSize(200.0f, 80.0f);
    titleGlow1_->setBackgroundColor(UIColor::Transparent());
    titleGlow1_->setTextColor(UIColor(255, 50, 50, 100)); // Dim red glow
    menuPanel_->addChild(titleGlow1_);

    // Main title (on top)
    gameTitle_ = std::make_shared<TextWidget>(config_.title, config_.titleFontSize);
    gameTitle_->setPosition(contentBounds.width / 2.0f - 120, 48.0f);
    gameTitle_->setSize(200.0f, 80.0f);
    gameTitle_->setBackgroundColor(UIColor::Transparent());
    gameTitle_->setTextColor(UIColor(255, 100, 100, 255)); // Bright red
    menuPanel_->addChild(gameTitle_);

    // Subtitle
    subtitle_ = std::make_shared<TextWidget>(config_.subtitle, config_.subtitleFontSize);
    subtitle_->setPosition(contentBounds.width / 2.0f - 200, 140.0f);
    subtitle_->setSize(400.0f, 40.0f);
    subtitle_->setBackgroundColor(UIColor::Transparent());
    subtitle_->setTextColor(UIColor(200, 200, 255, 255)); // Light blue
    menuPanel_->addChild(subtitle_);
}

void MainMenuWidget::createButtons() {
    auto contentBounds = menuPanel_->getContentBounds();

    // Play button
    playButton_ = std::make_shared<ButtonWidget>("PLAY");
    playButton_->setPosition(contentBounds.width / 2.0f - 120, 200.0f);
    playButton_->setSize(240.0f, 60.0f);
    playButton_->setBackgroundColor(UIColor(0, 150, 0, 200)); // Semi-transparent green
    playButton_->setBorderColor(UIColor(0, 255, 0, 255)); // Bright green border
    playButton_->setBorderWidth(2.0f);
    playButton_->setTextColor(UIColor::White());
    playButton_->setOnClick([this]() {
        std::cout << "Play button clicked! Starting game..." << std::endl;
        if (callbacks_.onPlay) {
            callbacks_.onPlay();
        }
    });
    menuPanel_->addChild(playButton_);

    // Multiplayer button
    multiplayerButton_ = std::make_shared<ButtonWidget>("MULTIPLAYER");
    multiplayerButton_->setPosition(contentBounds.width / 2.0f - 120, 270.0f);
    multiplayerButton_->setSize(240.0f, 50.0f);
    multiplayerButton_->setBackgroundColor(UIColor(0, 100, 200, 200)); // Blue
    multiplayerButton_->setBorderColor(UIColor(0, 150, 255, 255)); // Light blue border
    multiplayerButton_->setBorderWidth(2.0f);
    multiplayerButton_->setTextColor(UIColor::White());
    multiplayerButton_->setOnClick([this]() {
        std::cout << "Multiplayer button clicked!" << std::endl;
        if (callbacks_.onMultiplayer) {
            callbacks_.onMultiplayer();
        }
    });
    menuPanel_->addChild(multiplayerButton_);

    // Settings button
    settingsButton_ = std::make_shared<ButtonWidget>("SETTINGS");
    settingsButton_->setPosition(contentBounds.width / 2.0f - 120, 330.0f);
    settingsButton_->setSize(240.0f, 50.0f);
    settingsButton_->setBackgroundColor(UIColor(100, 100, 100, 200)); // Semi-transparent gray
    settingsButton_->setBorderColor(UIColor(150, 150, 150, 255)); // Light gray border
    settingsButton_->setBorderWidth(2.0f);
    settingsButton_->setTextColor(UIColor::White());
    settingsButton_->setOnClick([this]() {
        std::cout << "Settings button clicked!" << std::endl;
        if (callbacks_.onSettings) {
            callbacks_.onSettings();
        }
    });
    menuPanel_->addChild(settingsButton_);

    // Exit button
    exitButton_ = std::make_shared<ButtonWidget>("EXIT");
    exitButton_->setPosition(contentBounds.width / 2.0f - 120, 390.0f);
    exitButton_->setSize(240.0f, 50.0f);
    exitButton_->setBackgroundColor(UIColor(150, 0, 0, 200)); // Semi-transparent red
    exitButton_->setBorderColor(UIColor(255, 0, 0, 255)); // Bright red border
    exitButton_->setBorderWidth(2.0f);
    exitButton_->setTextColor(UIColor::White());
    exitButton_->setOnClick([this]() {
        std::cout << "Exit button clicked! Closing window..." << std::endl;
        if (callbacks_.onExit) {
            callbacks_.onExit();
        }
    });
    menuPanel_->addChild(exitButton_);
}

void MainMenuWidget::update(float deltaTime) {
    Widget::update(deltaTime);
    
    if (initialized_ && isVisible()) {
        updateTitleAnimation(deltaTime);
    }
}

void MainMenuWidget::updateTitleAnimation(float deltaTime) {
    // Update glow animation
    glowTime_ += deltaTime * config_.animationSpeed;
    titlePulse_ = 0.7f + 0.3f * (std::sin(glowTime_) + 1.0f) / 2.0f; // Pulse between 0.7 and 1.0

    // Update title glow colors dynamically
    int baseRed = 255;
    int glowRed = static_cast<int>(baseRed * titlePulse_);
    int glowAlpha1 = static_cast<int>(150 * titlePulse_);
    int glowAlpha2 = static_cast<int>(80 * titlePulse_);

    if (gameTitle_) {
        gameTitle_->setTextColor(UIColor(glowRed, static_cast<int>(100 * titlePulse_), static_cast<int>(100 * titlePulse_), 255));
    }
    if (titleGlow1_) {
        titleGlow1_->setTextColor(UIColor(255, 50, 50, glowAlpha1));
    }
    if (titleGlow2_) {
        titleGlow2_->setTextColor(UIColor(255, 30, 30, glowAlpha2));
    }
}

void MainMenuWidget::renderSelf() const {
    if (!isVisible()) return;
    
    renderBackground();
}

void MainMenuWidget::renderBackground() const {
    // Render animated star field background
    for (int i = 0; i < 100; i++) {
        int x = static_cast<int>(std::fmod(i * 137.5f, static_cast<float>(GetScreenWidth())));
        int y = static_cast<int>(std::fmod(i * 273.7f, static_cast<float>(GetScreenHeight())));
        float twinkle = std::sin(glowTime_ + i * 0.1f) * 0.5f + 0.5f;
        int alpha = static_cast<int>(50 + 100 * twinkle);
        DrawCircle(x, y, 1.0f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
    }

    // Draw subtle gradient overlay
    int screenHeight = GetScreenHeight();
    int screenWidth = GetScreenWidth();
    for (int y = 0; y < screenHeight; y += 4) {
        float intensity = static_cast<float>(y) / screenHeight;
        int alpha = static_cast<int>(20 * intensity);
        DrawRectangle(0, y, screenWidth, 4, Color{0, 0, 50, static_cast<unsigned char>(alpha)});
    }
}

void MainMenuWidget::show() {
    setVisible(true);
    if (menuPanel_) {
        menuPanel_->setVisible(true);
    }
}

void MainMenuWidget::hide() {
    setVisible(false);
    if (menuPanel_) {
        menuPanel_->setVisible(false);
    }
}

bool MainMenuWidget::isVisible() const {
    return Widget::isVisible();
}

} // namespace rtype::ui