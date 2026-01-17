#include "LobbyWidget.hpp"
#include "../UIColor.hpp"
#include <raylib.h>
#include <iostream>
#include <string>

namespace rtype::ui {

    LobbyWidget::LobbyWidget(const LobbyConfig& config) : Widget() , _config(config) , _initialized(false)
    {
        setSize(1200.0f, 650.0f);
    }

    void LobbyWidget::setCallbacks(const LobbyCallbacks& callbacks) {
        _callbacks = callbacks;
    }

    void LobbyWidget::initialize() {
        if (_initialized) return;
        createLobbyPanel();
        createTitle();
        createContent();
        createButtons();

        _initialized = true;
    }

    void LobbyWidget::createLobbyPanel() {
        _lobbyPanel = std::make_shared<PanelWidget>();
        _lobbyPanel->setPosition(-200.0f, -25.0f);
        auto transform = getTransform();
        _lobbyPanel->setSize(transform.width, transform.height);
        _lobbyPanel->setBackgroundColor(UIColor(10, 20, 40, 220)); // Dark blue background
        _lobbyPanel->setBorderColor(UIColor(100, 150, 255, 255)); // Blue border
        _lobbyPanel->setBorderWidth(3.0f);
        _lobbyPanel->setTitle("LOBBY");
        
        addChild(_lobbyPanel);
    }

    void LobbyWidget::createTitle() {
        auto contentBounds = _lobbyPanel->getContentBounds();
        _lobbyTitle = std::make_shared<TextWidget>(_config.title, _config.titleFontSize);
        _lobbyTitle->setPosition(contentBounds.width / 2.0f - 100, 20.0f);
        _lobbyTitle->setSize(200.0f, 60.0f);
        _lobbyTitle->setBackgroundColor(UIColor::Transparent());
        _lobbyTitle->setTextColor(UIColor(100, 200, 255, 255));
        _lobbyPanel->addChild(_lobbyTitle);

        _roomNameText = std::make_shared<TextWidget>("Room: " + _config.roomName, _config.textFontSize + 4);
        _roomNameText->setPosition(50.0f, 80.0f);
        _roomNameText->setSize(400.0f, 40.0f);
        _roomNameText->setBackgroundColor(UIColor::Transparent());
        _roomNameText->setTextColor(UIColor(200, 220, 255, 255));
        _lobbyPanel->addChild(_roomNameText);

        _connectionInfoText = std::make_shared<TextWidget>("Share this to join: 127.0.0.1:4242", _config.textFontSize - 2);
        _connectionInfoText->setPosition(50.0f, 120.0f);
        _connectionInfoText->setSize(600.0f, 30.0f);
        _connectionInfoText->setBackgroundColor(UIColor::Transparent());
        _connectionInfoText->setTextColor(UIColor(150, 200, 150, 255));
        _lobbyPanel->addChild(_connectionInfoText);
    }

    void LobbyWidget::createContent() {
        auto contentBounds = _lobbyPanel->getContentBounds();
        _playersLabel = std::make_shared<TextWidget>("Players:", _config.textFontSize);
        _playersLabel->setPosition(50.0f, 160.0f);
        _playersLabel->setSize(200.0f, 30.0f);
        _playersLabel->setBackgroundColor(UIColor::Transparent());
        _playersLabel->setTextColor(UIColor(180, 200, 255, 255));
        _lobbyPanel->addChild(_playersLabel);

        std::string playersText = "• Player 1 (You) - Host\n• Waiting for players...\n• Empty slot\n• Empty slot";
        _playersList = std::make_shared<TextWidget>(playersText, _config.textFontSize - 2);
        _playersList->setPosition(70.0f, 200.0f);
        _playersList->setSize(600.0f, 200.0f);
        _playersList->setBackgroundColor(UIColor(5, 10, 20, 100));
        _playersList->setBorderColor(UIColor(50, 80, 120, 255));
        _playersList->setBorderWidth(1.0f);
        _playersList->setTextColor(UIColor(160, 180, 220, 255));
        _lobbyPanel->addChild(_playersList);
    }

    void LobbyWidget::createButtons() {
        auto contentBounds = _lobbyPanel->getContentBounds();

        _backButton = std::make_shared<ButtonWidget>("BACK TO MENU");
        _backButton->setPosition(25.0f, 580.0f);
        _backButton->setSize(200.0f, 50.0f);
        _backButton->setBackgroundColor(UIColor(100, 100, 100, 200));
        _backButton->setBorderColor(UIColor(150, 150, 150, 255));
        _backButton->setBorderWidth(2.0f);
        _backButton->setTextColor(UIColor::White());
        _backButton->setOnClick([this]() {
            std::cout << "Back to menu button clicked!" << std::endl;
            if (_callbacks.onBack) {
                _callbacks.onBack();
            }
        });
        _lobbyPanel->addChild(_backButton);

        _startGameButton = std::make_shared<ButtonWidget>("START GAME");
        _startGameButton->setPosition(contentBounds.width - 215.0f, 580.0f);
        _startGameButton->setSize(200.0f, 50.0f);
        _startGameButton->setBackgroundColor(UIColor(0, 150, 0, 200));
        _startGameButton->setBorderColor(UIColor(0, 255, 0, 255));
        _startGameButton->setBorderWidth(2.0f);
        _startGameButton->setTextColor(UIColor::White());
        _startGameButton->setOnClick([this]() {
            std::cout << "Start game button clicked!" << std::endl;
            if (_callbacks.onStartGame) {
                _callbacks.onStartGame();
            }
        });
        _lobbyPanel->addChild(_startGameButton);
    }

    void LobbyWidget::show() {
        setVisible(true);
        if (_lobbyPanel) {
            _lobbyPanel->setVisible(true);
        }
    }

    void LobbyWidget::hide() {
        setVisible(false);
        if (_lobbyPanel) {
            _lobbyPanel->setVisible(false);
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
        _config.roomName = name;
        if (_roomNameText) {
            _roomNameText->setText("Room: " + name);
        }
    }

    void LobbyWidget::setBackgroundColor(const UIColor& color) {
        if (_lobbyPanel) {
            _lobbyPanel->setBackgroundColor(color);
        }
    }

    void LobbyWidget::updatePlayersList(const std::vector<LobbyPlayerInfo>& players) {
        if (!_playersList) return;

        std::string playersText;
        size_t slot = 0;
        
        for (const auto& player : players) {
            std::string marker = player.isLocal ? "(You)" : "";
            std::string hostMarker = player.isHost ? " - Host" : "";
            std::string readyMarker = player.isReady ? " [Ready]" : "";
            
            playersText += "• " + player.name + " " + marker + hostMarker + readyMarker + "\n";
            slot++;
        }
        
        // Fill remaining slots
        for (size_t i = slot; i < 4; i++) {
            playersText += "• Empty slot\n";
        }
        
        _playersList->setText(playersText);
    }

    void LobbyWidget::updateRoomList(const std::vector<LobbyRoomInfo>& rooms) {
        if (!_playersList) return;

        std::string roomsText;
        
        if (rooms.empty()) {
            roomsText = "No rooms available.\nCreate a new room to start playing!";
        } else {
            for (const auto& room : rooms) {
                std::string status = room.isPlaying ? " [In Game]" : "";
                roomsText += "• " + room.name + " (" + 
                             std::to_string(room.playerCount) + "/" + 
                             std::to_string(room.maxPlayers) + ")" + status + "\n";
            }
        }
        
        _playersList->setText(roomsText);
    }

    void LobbyWidget::setIsHost(bool isHost) {
        _config.isHost = isHost;
        
        if (_startGameButton) {
            if (isHost) {
                _startGameButton->setBackgroundColor(UIColor(0, 150, 0, 200)); // Green - enabled
                _startGameButton->setVisible(true);
            } else {
                _startGameButton->setBackgroundColor(UIColor(80, 80, 80, 200)); // Gray - disabled look
                // Still visible but indicates waiting for host
            }
        }
    }

    void LobbyWidget::setConnectionInfo(const std::string& info) {
        if (_connectionInfoText) {
            _connectionInfoText->setText(info);
        }
    }

} // namespace rtype::ui
