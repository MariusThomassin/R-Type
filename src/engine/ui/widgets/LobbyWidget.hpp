#pragma once

#include "../Widget.hpp"
#include "../UIColor.hpp"
#include "PanelWidget.hpp"
#include "ButtonWidget.hpp"
#include "TextWidget.hpp"
#include <memory>
#include <functional>

namespace rtype::ui {

/**
 * @brief Configuration structure for lobby
 */
struct LobbyConfig {
    std::string title = "LOBBY";
    std::string roomName = "Room #1";
    int titleFontSize = 48;
    int textFontSize = 20;
};

/**
 * @brief Callback functions for lobby events
 */
struct LobbyCallbacks {
    std::function<void()> onBack;
    std::function<void()> onStartGame;
};

/**
 * @brief Lobby widget for room creation and player management
 * 
 * This widget provides a lobby interface with:
 * - Room information display
 * - Player list (placeholder for now)
 * - Back button to return to main menu
 * - Start game button (for room host)
 */
class LobbyWidget : public Widget {
public:
    /**
     * @brief Constructor
     * @param config Initial configuration for the lobby
     */
    explicit LobbyWidget(const LobbyConfig& config = LobbyConfig{});

    /**
     * @brief Set callback functions for lobby events
     * @param callbacks Structure containing callback functions
     */
    void setCallbacks(const LobbyCallbacks& callbacks);

    /**
     * @brief Initialize the widget after adding to UI manager
     * This must be called after the widget is added to the UI manager
     * to avoid shared_from_this issues in constructor
     */
    void initialize();

    /**
     * @brief Show the lobby
     */
    void show();

    /**
     * @brief Hide the lobby
     */
    void hide();

    /**
     * @brief Check if the lobby is currently visible
     * @return True if visible, false otherwise
     */
    bool isVisible() const;

    /**
     * @brief Render the lobby widget
     */
    void renderSelf() const override;

    /**
     * @brief Set the room name
     * @param name New room name
     */
    void setRoomName(const std::string& name);

    /**
     * @brief Set the lobby background color
     * @param color New background color
     */
    void setBackgroundColor(const UIColor& color);

private:
    // Configuration
    LobbyConfig config_;
    LobbyCallbacks callbacks_;

    // UI Components
    std::shared_ptr<PanelWidget> lobbyPanel_;
    std::shared_ptr<TextWidget> lobbyTitle_;
    std::shared_ptr<TextWidget> roomNameText_;
    std::shared_ptr<TextWidget> playersLabel_;
    std::shared_ptr<TextWidget> playersList_;
    std::shared_ptr<ButtonWidget> backButton_;
    std::shared_ptr<ButtonWidget> startGameButton_;

    // State
    bool initialized_;

    /**
     * @brief Create the main lobby panel
     */
    void createLobbyPanel();

    /**
     * @brief Create the lobby title and room info
     */
    void createTitle();

    /**
     * @brief Create the lobby buttons
     */
    void createButtons();

    /**
     * @brief Create placeholder content
     */
    void createContent();
};

} // namespace rtype::ui