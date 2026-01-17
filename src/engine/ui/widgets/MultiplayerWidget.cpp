#include "MultiplayerWidget.hpp"
#include "../UIColor.hpp"
#include <raylib.h>
#include <iostream>
#include <sstream>

namespace rtype::ui {

    MultiplayerWidget::MultiplayerWidget(const MultiplayerConfig& config) : Widget(), _config(config), _initialized(false), _currentTab(MultiplayerTab::JOIN)
    {
        setSize(1200.0f, 650.0f);
    }

    void MultiplayerWidget::setCallbacks(const MultiplayerCallbacks& callbacks) {
        _callbacks = callbacks;
    }

    void MultiplayerWidget::initialize() {
        if (_initialized) return;
        createMainPanel();
        createTabButtons();
        createJoinInterface();
        createCreateInterface();
        createNavigation();

        rtype::ui::ErrorMessageConfig errorConfig;
        errorConfig.title = "CONNECTION ERROR";
        errorConfig.autoCloseDelay = 0.0f;
        _errorMessageWidget = std::make_shared<ErrorMessageWidget>("", errorConfig);
        _errorMessageWidget->setVisible(false);

        rtype::ui::ErrorMessageCallbacks errorCallbacks;
        errorCallbacks.onClose = [this]() {};
        _errorMessageWidget->setCallbacks(errorCallbacks);
        _errorMessageWidget->initialize();
        addChild(_errorMessageWidget);

        switchTab(MultiplayerTab::JOIN);

        _initialized = true;
    }

    void MultiplayerWidget::createMainPanel() {
        _mainPanel = std::make_shared<PanelWidget>();
        _mainPanel->setPosition(-200.0f, -25.0f);
        auto transform = getTransform();
        _mainPanel->setSize(transform.width, transform.height);
        _mainPanel->setBackgroundColor(UIColor(10, 20, 40, 220));
        _mainPanel->setBorderColor(UIColor(100, 150, 255, 255));
        _mainPanel->setBorderWidth(3.0f);
        _mainPanel->setTitle("MULTIPLAYER");

        addChild(_mainPanel);

        auto contentBounds = _mainPanel->getContentBounds();
        _titleText = std::make_shared<TextWidget>(_config.title, _config.titleFontSize);
        _titleText->setPosition(contentBounds.width / 2.0f - 120, 20.0f);
        _titleText->setSize(240.0f, 60.0f);
        _titleText->setBackgroundColor(UIColor::Transparent());
        _titleText->setTextColor(UIColor(100, 200, 255, 255));
        _mainPanel->addChild(_titleText);
    }

    void MultiplayerWidget::createTabButtons() {
        auto contentBounds = _mainPanel->getContentBounds();
        _joinTabButton = std::make_shared<ButtonWidget>("JOIN SERVER");
        _joinTabButton->setPosition(100.0f, 90.0f);
        _joinTabButton->setSize(200.0f, 50.0f);
        _joinTabButton->setTextColor(UIColor::White());
        _joinTabButton->setOnClick([this]() {
            switchTab(MultiplayerTab::JOIN);
        });
        _mainPanel->addChild(_joinTabButton);

        _createTabButton = std::make_shared<ButtonWidget>("CREATE ROOM");
        _createTabButton->setPosition(320.0f, 90.0f);
        _createTabButton->setSize(200.0f, 50.0f);
        _createTabButton->setTextColor(UIColor::White());
        _createTabButton->setOnClick([this]() {
            switchTab(MultiplayerTab::CREATE);
        });
        _mainPanel->addChild(_createTabButton);
    }

    void MultiplayerWidget::createJoinInterface() {
        auto contentBounds = _mainPanel->getContentBounds();
        _joinPanel = std::make_shared<PanelWidget>();
        _joinPanel->setPosition(30.0f, 160.0f);
        _joinPanel->setSize(contentBounds.width - 60.0f, 340.0f);
        _joinPanel->setBackgroundColor(UIColor(5, 15, 30, 150));
        _joinPanel->setBorderColor(UIColor(50, 100, 150, 255));
        _joinPanel->setBorderWidth(1.0f);
        _mainPanel->addChild(_joinPanel);

        _joinInstructionsText = std::make_shared<TextWidget>("Enter server details or select a room from the list:", _config.textFontSize);
        _joinInstructionsText->setPosition(20.0f, 20.0f);
        _joinInstructionsText->setSize(500.0f, 30.0f);
        _joinInstructionsText->setBackgroundColor(UIColor::Transparent());
        _joinInstructionsText->setTextColor(UIColor(200, 220, 255, 255));
        _joinPanel->addChild(_joinInstructionsText);

        _ipLabel = std::make_shared<TextWidget>("Server IP:", _config.textFontSize);
        _ipLabel->setPosition(20.0f, 70.0f);
        _ipLabel->setSize(120.0f, 30.0f);
        _ipLabel->setBackgroundColor(UIColor::Transparent());
        _ipLabel->setTextColor(UIColor(180, 200, 255, 255));
        _joinPanel->addChild(_ipLabel);

        _ipInput = std::make_shared<InputFieldWidget>("127.0.0.1");
        _ipInput->setPosition(150.0f, 65.0f);
        _ipInput->setSize(250.0f, 40.0f);
        _ipInput->setText(_config.defaultIP);
        _ipInput->setVisible(true);
        _joinPanel->addChild(_ipInput);

        _portLabel = std::make_shared<TextWidget>("Port:", _config.textFontSize);
        _portLabel->setPosition(20.0f, 130.0f);
        _portLabel->setSize(120.0f, 30.0f);
        _portLabel->setBackgroundColor(UIColor::Transparent());
        _portLabel->setTextColor(UIColor(180, 200, 255, 255));
        _joinPanel->addChild(_portLabel);

        _portInput = std::make_shared<InputFieldWidget>("4242");
        _portInput->setPosition(150.0f, 125.0f);
        _portInput->setSize(150.0f, 40.0f);
        _portInput->setText(std::to_string(_config.defaultPort));
        _portInput->setVisible(true);
        _joinPanel->addChild(_portInput);

        _joinServerButton = std::make_shared<ButtonWidget>("JOIN SERVER");
        _joinServerButton->setPosition(150.0f, 200.0f);
        _joinServerButton->setSize(200.0f, 50.0f);
        _joinServerButton->setBackgroundColor(UIColor(0, 120, 180, 200));
        _joinServerButton->setBorderColor(UIColor(0, 150, 220, 255));
        _joinServerButton->setBorderWidth(2.0f);
        _joinServerButton->setTextColor(UIColor::White());
        _joinServerButton->setOnClick([this]() {
            std::string ip = _ipInput->getText();
            std::string portStr = _portInput->getText();
            
            try {
                int port = std::stoi(portStr);
                std::cout << "Joining server: " << ip << ":" << port << std::endl;
                if (_callbacks.onJoinServer) {
                    _callbacks.onJoinServer(ip, port);
                }
            } catch (const std::exception& e) {
                std::cout << "Invalid port number: " << portStr << std::endl;
            }
        });
        _joinPanel->addChild(_joinServerButton);

        // Room list panel (right side)
        _roomListPanel = std::make_shared<PanelWidget>();
        _roomListPanel->setPosition(480.0f, 60.0f);
        _roomListPanel->setSize(420.0f, 260.0f);
        _roomListPanel->setBackgroundColor(UIColor(2, 8, 15, 180));
        _roomListPanel->setBorderColor(UIColor(80, 120, 180, 255));
        _roomListPanel->setBorderWidth(1.0f);
        _joinPanel->addChild(_roomListPanel);

        // Room list title
        _roomListTitle = std::make_shared<TextWidget>("Available Rooms", _config.textFontSize);
        _roomListTitle->setPosition(20.0f, 10.0f);
        _roomListTitle->setSize(200.0f, 30.0f);
        _roomListTitle->setBackgroundColor(UIColor::Transparent());
        _roomListTitle->setTextColor(UIColor(150, 180, 255, 255));
        _roomListPanel->addChild(_roomListTitle);

        // Room list content - initially empty, will be populated by refreshRoomList()
        _roomListContent = std::make_shared<TextWidget>("Fetching rooms from server...", _config.textFontSize - 4);
        _roomListContent->setPosition(20.0f, 45.0f);
        _roomListContent->setSize(400.0f, 160.0f);
        _roomListContent->setBackgroundColor(UIColor::Transparent());
        _roomListContent->setTextColor(UIColor(180, 200, 240, 255));
        _roomListPanel->addChild(_roomListContent);

        // Refresh button for room list
        auto refreshButton = std::make_shared<ButtonWidget>("REFRESH");
        refreshButton->setPosition(320.0f, 10.0f);
        refreshButton->setSize(100.0f, 30.0f);
        refreshButton->setBackgroundColor(UIColor(0, 80, 160, 200));
        refreshButton->setBorderColor(UIColor(0, 120, 220, 255));
        refreshButton->setBorderWidth(1.0f);
        refreshButton->setTextColor(UIColor::White());
        refreshButton->setOnClick([this]() {
            refreshRoomList();
            return true;
        });
        _roomListPanel->addChild(refreshButton);

        // Initial room list fetch
        refreshRoomList();
    }

    void MultiplayerWidget::createCreateInterface() {
        auto contentBounds = _mainPanel->getContentBounds();

        // Create panel
        _createPanel = std::make_shared<PanelWidget>();
        _createPanel->setPosition(30.0f, 160.0f);
        _createPanel->setSize(contentBounds.width - 60.0f, 420.0f);
        _createPanel->setBackgroundColor(UIColor(5, 15, 30, 150));
        _createPanel->setBorderColor(UIColor(50, 100, 150, 255));
        _createPanel->setBorderWidth(1.0f);
        _mainPanel->addChild(_createPanel);

        // Instructions
        _createInstructionsText = std::make_shared<TextWidget>("Customize and create your room:", _config.textFontSize);
        _createInstructionsText->setPosition(20.0f, 20.0f);
        _createInstructionsText->setSize(600.0f, 30.0f);
        _createInstructionsText->setBackgroundColor(UIColor::Transparent());
        _createInstructionsText->setTextColor(UIColor(200, 220, 255, 255));
        _createPanel->addChild(_createInstructionsText);

        // Room name input
        _roomNameLabel = std::make_shared<TextWidget>("Room Name:", _config.textFontSize - 4);
        _roomNameLabel->setPosition(20.0f, 60.0f);
        _roomNameLabel->setSize(120.0f, 25.0f);
        _roomNameLabel->setBackgroundColor(UIColor::Transparent());
        _roomNameLabel->setTextColor(UIColor(180, 200, 255, 255));
        _createPanel->addChild(_roomNameLabel);

        _roomNameInput = std::make_shared<InputFieldWidget>(_currentRoomSettings.name);
        _roomNameInput->setPosition(150.0f, 55.0f);
        _roomNameInput->setSize(250.0f, 35.0f);
        _createPanel->addChild(_roomNameInput);

        // Password input
        _passwordLabel = std::make_shared<TextWidget>("Password:", _config.textFontSize - 4);
        _passwordLabel->setPosition(450.0f, 60.0f);
        _passwordLabel->setSize(100.0f, 25.0f);
        _passwordLabel->setBackgroundColor(UIColor::Transparent());
        _passwordLabel->setTextColor(UIColor(180, 200, 255, 255));
        _createPanel->addChild(_passwordLabel);

        _passwordInput = std::make_shared<InputFieldWidget>(_currentRoomSettings.password);
        _passwordInput->setPosition(560.0f, 55.0f);
        _passwordInput->setSize(200.0f, 35.0f);
        _createPanel->addChild(_passwordInput);

        // Background color selection
        _colorLabel = std::make_shared<TextWidget>("Room Background Color:", _config.textFontSize - 4);
        _colorLabel->setPosition(20.0f, 110.0f);
        _colorLabel->setSize(200.0f, 25.0f);
        _colorLabel->setBackgroundColor(UIColor::Transparent());
        _colorLabel->setTextColor(UIColor(180, 200, 255, 255));
        _createPanel->addChild(_colorLabel);

        // Color preview
        _colorPreview = std::make_shared<PanelWidget>();
        _colorPreview->setPosition(250.0f, 105.0f);
        _colorPreview->setSize(100.0f, 35.0f);
        _colorPreview->setBackgroundColor(_currentRoomSettings.backgroundColor);
        _colorPreview->setBorderColor(UIColor::White());
        _colorPreview->setBorderWidth(2.0f);
        _createPanel->addChild(_colorPreview);

        // Color selection buttons
        std::vector<UIColor> colorOptions = {
            UIColor(10, 20, 40, 220),    // Dark blue (default)
            UIColor(40, 10, 20, 220),    // Dark red
            UIColor(20, 40, 10, 220),    // Dark green
            UIColor(40, 30, 10, 220),    // Dark orange
            UIColor(30, 10, 40, 220),    // Dark purple
            UIColor(10, 30, 30, 220),    // Dark teal
            UIColor(30, 30, 10, 220),    // Dark yellow
            UIColor(20, 20, 20, 220)     // Dark gray
        };

        float buttonX = 20.0f;
        float buttonY = 150.0f;
        for (size_t i = 0; i < colorOptions.size(); ++i) {
            auto colorButton = std::make_shared<ButtonWidget>("");
            colorButton->setPosition(buttonX + (i % 4) * 80.0f, buttonY + (i / 4) * 50.0f);
            colorButton->setSize(60.0f, 40.0f);
            colorButton->setBackgroundColor(colorOptions[i]);
            colorButton->setBorderColor(UIColor::White());
            colorButton->setBorderWidth(2.0f);

            UIColor selectedColor = colorOptions[i];
            colorButton->setOnClick([this, selectedColor]() {
                _currentRoomSettings.backgroundColor = selectedColor;
                if (_colorPreview) {
                    _colorPreview->setBackgroundColor(selectedColor);
                }
            });

            _createPanel->addChild(colorButton);
            _colorButtons.push_back(colorButton);
        }

        _createRoomButton = std::make_shared<ButtonWidget>("CREATE ROOM");
        _createRoomButton->setPosition(contentBounds.width - 275.0f, 355.0f);
        _createRoomButton->setSize(200.0f, 50.0f);
        _createRoomButton->setBackgroundColor(UIColor(0, 100, 200, 200));
        _createRoomButton->setBorderColor(UIColor(0, 150, 255, 255));
        _createRoomButton->setBorderWidth(2.0f);
        _createRoomButton->setTextColor(UIColor::White());
        _createRoomButton->setOnClick([this]() {
            // Update room settings from input fields
            if (_roomNameInput) {
                _currentRoomSettings.name = _roomNameInput->getText();
            }
            if (_passwordInput) {
                _currentRoomSettings.password = _passwordInput->getText();
            }
            
            // Validate room name
            if (_currentRoomSettings.name.empty()) {
                showError("Room name cannot be empty!");
                return;
            }
            
            // Check network connectivity before creating room
            if (_callbacks.checkNetworkConnection && !_callbacks.checkNetworkConnection()) {
                showError("Cannot create room: Not connected to server.\n\nPlease ensure that:\n• A server is running\n• Network connection is available\n• Server address is correct");
                return;
            }
            
            std::cout << "Creating room '" << _currentRoomSettings.name << "' with background color " 
                    << _currentRoomSettings.backgroundColor.getRed() << "," << _currentRoomSettings.backgroundColor.getGreen() 
                    << "," << _currentRoomSettings.backgroundColor.getBlue() << std::endl;
            
            if (_callbacks.onCreateRoom) {
                _callbacks.onCreateRoom(_currentRoomSettings);
            }
        });
        _createPanel->addChild(_createRoomButton);
    }

    void MultiplayerWidget::createNavigation() {
        auto contentBounds = _mainPanel->getContentBounds();

        _joinBackButton = std::make_shared<ButtonWidget>("BACK TO MENU");
        _joinBackButton->setPosition(contentBounds.width - 270.0f, 270.0f);
        _joinBackButton->setSize(200.0f, 50.0f);
        _joinBackButton->setBackgroundColor(UIColor(100, 100, 100, 200));
        _joinBackButton->setBorderColor(UIColor(150, 150, 150, 255));
        _joinBackButton->setBorderWidth(2.0f);
        _joinBackButton->setTextColor(UIColor::White());
        _joinBackButton->setOnClick([this]() {
            std::cout << "Back to main menu from multiplayer..." << std::endl;
            if (_callbacks.onBack) {
                _callbacks.onBack();
            }
        });
        _joinPanel->addChild(_joinBackButton);

        _createBackButton = std::make_shared<ButtonWidget>("BACK TO MENU");
        _createBackButton->setPosition(15.0f, 355.0f);
        _createBackButton->setSize(200.0f, 50.0f);
        _createBackButton->setBackgroundColor(UIColor(100, 100, 100, 200));
        _createBackButton->setBorderColor(UIColor(150, 150, 150, 255));
        _createBackButton->setBorderWidth(2.0f);
        _createBackButton->setTextColor(UIColor::White());
        _createBackButton->setOnClick([this]() {
            std::cout << "Back to main menu from multiplayer..." << std::endl;
            if (_callbacks.onBack) {
                _callbacks.onBack();
            }
        });
        _createPanel->addChild(_createBackButton);
    }

    void MultiplayerWidget::switchTab(MultiplayerTab tab) {
        _currentTab = tab;
        updateTabButtons();
        updateTabContent();
    }

    void MultiplayerWidget::updateTabButtons() {
        if (_currentTab == MultiplayerTab::JOIN) {
            _joinTabButton->setBackgroundColor(UIColor(0, 100, 200, 255));
            _joinTabButton->setBorderColor(UIColor(0, 150, 255, 255));
        } else {
            _joinTabButton->setBackgroundColor(UIColor(60, 60, 80, 200));
            _joinTabButton->setBorderColor(UIColor(100, 100, 120, 255));
        }
        _joinTabButton->setBorderWidth(2.0f);

        if (_currentTab == MultiplayerTab::CREATE) {
            _createTabButton->setBackgroundColor(UIColor(0, 100, 200, 255));
            _createTabButton->setBorderColor(UIColor(0, 150, 255, 255));
        } else {
            _createTabButton->setBackgroundColor(UIColor(60, 60, 80, 200));
            _createTabButton->setBorderColor(UIColor(100, 100, 120, 255));
        }
        _createTabButton->setBorderWidth(2.0f);
    }

    void MultiplayerWidget::updateTabContent() {
        _joinPanel->setVisible(_currentTab == MultiplayerTab::JOIN);
        _createPanel->setVisible(_currentTab == MultiplayerTab::CREATE);
    }

    void MultiplayerWidget::show() {
        setVisible(true);
        if (_mainPanel) {
            _mainPanel->setVisible(true);
            updateTabContent();
        }
    }

    void MultiplayerWidget::hide() {
        setVisible(false);
        if (_mainPanel) {
            _mainPanel->setVisible(false);
        }
    }

    bool MultiplayerWidget::isVisible() const {
        return Widget::isVisible();
    }

    void MultiplayerWidget::renderSelf() const {
        // The main panel and its children handle their own rendering
    }

    void MultiplayerWidget::showError(const std::string& message) {
        if (_errorMessageWidget) {
            _errorMessageWidget->setMessage(message);
            _errorMessageWidget->show();
        }
    }

    void MultiplayerWidget::refreshRoomList() {
        if (!_callbacks.onGetRoomList) {
            if (_roomListContent) {
                _roomListContent->setText("No room list service available.\n\nClick to retry...");
            }
            return;
        }

        try {
            // Fetch room list from server
            std::vector<RoomInfo> rooms = _callbacks.onGetRoomList();
            
            std::string roomListText;
            
            if (rooms.empty()) {
                roomListText = "No rooms available.\n\nCreate a new room or\ncheck server connection.\n\nClick to refresh...";
            } else {
                for (const auto& room : rooms) {
                    roomListText += "• " + room.name + " (" + std::to_string(room.currentPlayers) + "/" + std::to_string(room.maxPlayers) + " players)";
                    
                    if (room.isPasswordProtected) {
                        roomListText += " [🔒]";
                    }
                    
                    if (room.isFull()) {
                        roomListText += " [FULL]";
                    }
                    
                    roomListText += "\n";
                }
                roomListText += "\nClick to refresh list...";
            }
            
            if (_roomListContent) {
                _roomListContent->setText(roomListText);
            }
            
            std::cout << "Room list updated - found " << rooms.size() << " rooms" << std::endl;
            
        } catch (const std::exception& e) {
            if (_roomListContent) {
                _roomListContent->setText("Failed to fetch rooms.\n\nServer may be offline.\n\nClick to retry...");
            }
            std::cout << "Error fetching room list: " << e.what() << std::endl;
        }
    }
} // namespace rtype::ui