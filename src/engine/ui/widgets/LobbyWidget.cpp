#include "LobbyWidget.hpp"
#include "../UIColor.hpp"
#include <raylib.h>
#include <iostream>

namespace rtype::ui {

LobbyWidget::LobbyWidget(const LobbyConfig& config)
    : Widget()
    , config_(config)
    , initialized_(false)
{
    // Set larger size to match multiplayer widget
    setSize(1200.0f, 650.0f);
}

void LobbyWidget::setCallbacks(const LobbyCallbacks& callbacks) {
    callbacks_ = callbacks;
}

void LobbyWidget::initialize() {
    if (initialized_) return;

    createLobbyPanel();
    createTitle();
    createContent();
    createButtons();

    initialized_ = true;
}

void LobbyWidget::createLobbyPanel() {
    lobbyPanel_ = std::make_shared<PanelWidget>();
    lobbyPanel_->setPosition(-200.0f, -25.0f);
    auto transform = getTransform();
    lobbyPanel_->setSize(transform.width, transform.height);
    lobbyPanel_->setBackgroundColor(UIColor(10, 20, 40, 220)); // Dark blue background
    lobbyPanel_->setBorderColor(UIColor(100, 150, 255, 255)); // Blue border
    lobbyPanel_->setBorderWidth(3.0f);
    lobbyPanel_->setTitle("LOBBY");
    
    addChild(lobbyPanel_);
}

void LobbyWidget::createTitle() {
    auto contentBounds = lobbyPanel_->getContentBounds();

    // Lobby title
    lobbyTitle_ = std::make_shared<TextWidget>(config_.title, config_.titleFontSize);
    lobbyTitle_->setPosition(contentBounds.width / 2.0f - 100, 20.0f);
    lobbyTitle_->setSize(200.0f, 60.0f);
    lobbyTitle_->setBackgroundColor(UIColor::Transparent());
    lobbyTitle_->setTextColor(UIColor(100, 200, 255, 255)); // Light blue
    lobbyPanel_->addChild(lobbyTitle_);

    // Room name
    roomNameText_ = std::make_shared<TextWidget>("Room: " + config_.roomName, config_.textFontSize + 4);
    roomNameText_->setPosition(50.0f, 80.0f);
    roomNameText_->setSize(400.0f, 40.0f);
    roomNameText_->setBackgroundColor(UIColor::Transparent());
    roomNameText_->setTextColor(UIColor(200, 220, 255, 255)); // Lighter blue
    lobbyPanel_->addChild(roomNameText_);

    // Connection info (IP:Port for sharing)
    connectionInfoText_ = std::make_shared<TextWidget>("Share this to join: 127.0.0.1:4242", config_.textFontSize - 2);
    connectionInfoText_->setPosition(50.0f, 120.0f);
    connectionInfoText_->setSize(600.0f, 30.0f);
    connectionInfoText_->setBackgroundColor(UIColor::Transparent());
    connectionInfoText_->setTextColor(UIColor(150, 200, 150, 255)); // Light green
    lobbyPanel_->addChild(connectionInfoText_);
}

void LobbyWidget::createContent() {
    auto contentBounds = lobbyPanel_->getContentBounds();

    // Players section label
    playersLabel_ = std::make_shared<TextWidget>("Players:", config_.textFontSize);
    playersLabel_->setPosition(50.0f, 160.0f);
    playersLabel_->setSize(200.0f, 30.0f);
    playersLabel_->setBackgroundColor(UIColor::Transparent());
    playersLabel_->setTextColor(UIColor(180, 200, 255, 255));
    lobbyPanel_->addChild(playersLabel_);

    // Placeholder players list
    std::string playersText = "• Player 1 (You) - Host\n• Waiting for players...\n• Empty slot\n• Empty slot";
    playersList_ = std::make_shared<TextWidget>(playersText, config_.textFontSize - 2);
    playersList_->setPosition(70.0f, 200.0f);
    playersList_->setSize(600.0f, 200.0f);
    playersList_->setBackgroundColor(UIColor(5, 10, 20, 100)); // Semi-transparent dark background
    playersList_->setBorderColor(UIColor(50, 80, 120, 255));
    playersList_->setBorderWidth(1.0f);
    playersList_->setTextColor(UIColor(160, 180, 220, 255));
    lobbyPanel_->addChild(playersList_);
}

void LobbyWidget::createButtons() {
    auto contentBounds = lobbyPanel_->getContentBounds();

    // Back button
    backButton_ = std::make_shared<ButtonWidget>("BACK TO MENU");
    backButton_->setPosition(25.0f, 580.0f);
    backButton_->setSize(200.0f, 50.0f);
    backButton_->setBackgroundColor(UIColor(100, 100, 100, 200)); // Gray
    backButton_->setBorderColor(UIColor(150, 150, 150, 255));
    backButton_->setBorderWidth(2.0f);
    backButton_->setTextColor(UIColor::White());
    backButton_->setOnClick([this]() {
        std::cout << "Back to menu button clicked!" << std::endl;
        if (callbacks_.onBack) {
            callbacks_.onBack();
        }
    });
    lobbyPanel_->addChild(backButton_);

    // Start game button (for host)
    startGameButton_ = std::make_shared<ButtonWidget>("START GAME");
    startGameButton_->setPosition(contentBounds.width - 215.0f, 580.0f);
    startGameButton_->setSize(200.0f, 50.0f);
    startGameButton_->setBackgroundColor(UIColor(0, 150, 0, 200)); // Green
    startGameButton_->setBorderColor(UIColor(0, 255, 0, 255));
    startGameButton_->setBorderWidth(2.0f);
    startGameButton_->setTextColor(UIColor::White());
    startGameButton_->setOnClick([this]() {
        std::cout << "Start game button clicked!" << std::endl;
        if (callbacks_.onStartGame) {
            callbacks_.onStartGame();
        }
    });
    lobbyPanel_->addChild(startGameButton_);
}

void LobbyWidget::show() {
    setVisible(true);
    if (lobbyPanel_) {
        lobbyPanel_->setVisible(true);
    }
}

void LobbyWidget::hide() {
    setVisible(false);
    if (lobbyPanel_) {
        lobbyPanel_->setVisible(false);
    }
}

bool LobbyWidget::isVisible() const {
    return Widget::isVisible();
}

void LobbyWidget::renderSelf() const {
    // The lobby panel and its children handle their own rendering
    // This method is required by the Widget interface but the actual
    // rendering is done by the child widgets (lobbyPanel_ and its children)
}

void LobbyWidget::setRoomName(const std::string& name) {
    config_.roomName = name;
    if (roomNameText_) {
        roomNameText_->setText("Room: " + name);
    }
}

void LobbyWidget::setBackgroundColor(const UIColor& color) {
    if (lobbyPanel_) {
        lobbyPanel_->setBackgroundColor(color);
    }
}

} // namespace rtype::ui