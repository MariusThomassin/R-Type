#pragma once

#include "../Widget.hpp"
#include "PanelWidget.hpp"
#include "ButtonWidget.hpp"
#include "TextWidget.hpp"
#include <memory>
#include <functional>

namespace rtype::ui {

/**
 * @brief Configuration structure for main menu
 */
struct MainMenuConfig {
    std::string title = "R-TYPE";
    std::string subtitle = "The classic side-scrolling shooter";
    int titleFontSize = 64;
    int subtitleFontSize = 24;
    float animationSpeed = 2.0f;
};

/**
 * @brief Callback functions for main menu events
 */
struct MainMenuCallbacks {
    std::function<void()> onPlay;
    std::function<void()> onMultiplayer;
    std::function<void()> onSettings;
    std::function<void()> onExit;
};

/**
 * @brief Main menu widget with animated title and styled buttons
 * 
 * This widget provides a complete main menu interface with:
 * - Animated glowing title with multiple layers for effects
 * - Styled buttons for Play, Settings, and Exit
 * - Semi-transparent panel background with border
 * - Proper positioning and sizing for different screen sizes
 */
class MainMenuWidget : public Widget {
public:
    /**
     * @brief Constructor
     * @param config Initial configuration for the menu
     */
    explicit MainMenuWidget(const MainMenuConfig& config = MainMenuConfig{});

    /**
     * @brief Set callback functions for menu events
     * @param callbacks Structure containing callback functions
     */
    void setCallbacks(const MainMenuCallbacks& callbacks);

    /**
     * @brief Initialize the widget after adding to UI manager
     * This must be called after the widget is added to the UI manager
     * to avoid shared_from_this issues in constructor
     */
    void initialize();

    /**
     * @brief Update the widget (handles animations)
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime) override;

    /**
     * @brief Render the widget background effects
     */
    void renderBackground() const;

    /**
     * @brief Render the widget itself (override for Widget base class)
     */
    void renderSelf() const override;

    /**
     * @brief Show the main menu
     */
    void show();

    /**
     * @brief Hide the main menu
     */
    void hide();

    /**
     * @brief Check if the menu is currently visible
     * @return True if visible, false otherwise
     */
    bool isVisible() const;

private:
    // Configuration
    MainMenuConfig config_;
    MainMenuCallbacks callbacks_;

    // UI Components
    std::shared_ptr<PanelWidget> menuPanel_;
    std::shared_ptr<TextWidget> gameTitle_;
    std::shared_ptr<TextWidget> titleGlow1_;
    std::shared_ptr<TextWidget> titleGlow2_;
    std::shared_ptr<TextWidget> subtitle_;
    std::shared_ptr<ButtonWidget> playButton_;
    std::shared_ptr<ButtonWidget> multiplayerButton_;
    std::shared_ptr<ButtonWidget> settingsButton_;
    std::shared_ptr<ButtonWidget> exitButton_;

    // Animation state
    float glowTime_;
    float titlePulse_;
    bool initialized_;

    /**
     * @brief Create the main menu panel
     */
    void createMenuPanel();

    /**
     * @brief Create the animated title with glow effects
     */
    void createTitle();

    /**
     * @brief Create the menu buttons
     */
    void createButtons();

    /**
     * @brief Update title animation
     * @param deltaTime Time elapsed since last update
     */
    void updateTitleAnimation(float deltaTime);
};

} // namespace rtype::ui