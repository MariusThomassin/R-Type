#include "MultiplayerWidget.hpp"
#include "../UIColor.hpp"
#include <raylib.h>
#include <iostream>
#include <sstream>

namespace rtype::ui {

MultiplayerWidget::MultiplayerWidget(const MultiplayerConfig& config)
    : Widget()
    , config_(config)
    , initialized_(false)
    , currentTab_(MultiplayerTab::JOIN)
{
    // Set larger size for more spacious panels
    setSize(1200.0f, 650.0f);
}

void MultiplayerWidget::setCallbacks(const MultiplayerCallbacks& callbacks) {
    callbacks_ = callbacks;
}

void MultiplayerWidget::initialize() {
    if (initialized_) return;

    createMainPanel();
    createTabButtons();
    createJoinInterface();
    createCreateInterface();
    createNavigation();

    // Create error message widget
    rtype::ui::ErrorMessageConfig errorConfig;
    errorConfig.title = "CONNECTION ERROR";
    errorConfig.autoCloseDelay = 0.0f; // Don't auto-close
    errorMessageWidget_ = std::make_shared<ErrorMessageWidget>("", errorConfig);
    errorMessageWidget_->setVisible(false);
    
    rtype::ui::ErrorMessageCallbacks errorCallbacks;
    errorCallbacks.onClose = [this]() {
        // Error dialog closed, no additional action needed
    };
    errorMessageWidget_->setCallbacks(errorCallbacks);
    errorMessageWidget_->initialize();
    addChild(errorMessageWidget_);

    // Start with Join tab
    switchTab(MultiplayerTab::JOIN);

    initialized_ = true;
}

void MultiplayerWidget::createMainPanel() {
    mainPanel_ = std::make_shared<PanelWidget>();
    mainPanel_->setPosition(-200.0f, -25.0f);
    auto transform = getTransform();
    mainPanel_->setSize(transform.width, transform.height);
    mainPanel_->setBackgroundColor(UIColor(10, 20, 40, 220));
    mainPanel_->setBorderColor(UIColor(100, 150, 255, 255));
    mainPanel_->setBorderWidth(3.0f);
    mainPanel_->setTitle("MULTIPLAYER");
    
    addChild(mainPanel_);

    // Title
    auto contentBounds = mainPanel_->getContentBounds();
    titleText_ = std::make_shared<TextWidget>(config_.title, config_.titleFontSize);
    titleText_->setPosition(contentBounds.width / 2.0f - 120, 20.0f);
    titleText_->setSize(240.0f, 60.0f);
    titleText_->setBackgroundColor(UIColor::Transparent());
    titleText_->setTextColor(UIColor(100, 200, 255, 255));
    mainPanel_->addChild(titleText_);
}

void MultiplayerWidget::createTabButtons() {
    auto contentBounds = mainPanel_->getContentBounds();

    // Join tab button
    joinTabButton_ = std::make_shared<ButtonWidget>("JOIN SERVER");
    joinTabButton_->setPosition(100.0f, 90.0f);
    joinTabButton_->setSize(200.0f, 50.0f);
    joinTabButton_->setTextColor(UIColor::White());
    joinTabButton_->setOnClick([this]() {
        switchTab(MultiplayerTab::JOIN);
    });
    mainPanel_->addChild(joinTabButton_);

    // Create tab button
    createTabButton_ = std::make_shared<ButtonWidget>("CREATE ROOM");
    createTabButton_->setPosition(320.0f, 90.0f);
    createTabButton_->setSize(200.0f, 50.0f);
    createTabButton_->setTextColor(UIColor::White());
    createTabButton_->setOnClick([this]() {
        switchTab(MultiplayerTab::CREATE);
    });
    mainPanel_->addChild(createTabButton_);
}

void MultiplayerWidget::createJoinInterface() {
    auto contentBounds = mainPanel_->getContentBounds();

    // Join panel
    joinPanel_ = std::make_shared<PanelWidget>();
    joinPanel_->setPosition(30.0f, 160.0f);
    joinPanel_->setSize(contentBounds.width - 60.0f, 340.0f);
    joinPanel_->setBackgroundColor(UIColor(5, 15, 30, 150));
    joinPanel_->setBorderColor(UIColor(50, 100, 150, 255));
    joinPanel_->setBorderWidth(1.0f);
    mainPanel_->addChild(joinPanel_);

    // Instructions
    joinInstructionsText_ = std::make_shared<TextWidget>("Enter server details or select a room from the list:", config_.textFontSize);
    joinInstructionsText_->setPosition(20.0f, 20.0f);
    joinInstructionsText_->setSize(500.0f, 30.0f);
    joinInstructionsText_->setBackgroundColor(UIColor::Transparent());
    joinInstructionsText_->setTextColor(UIColor(200, 220, 255, 255));
    joinPanel_->addChild(joinInstructionsText_);

    // IP Address input
    ipLabel_ = std::make_shared<TextWidget>("Server IP:", config_.textFontSize);
    ipLabel_->setPosition(20.0f, 70.0f);
    ipLabel_->setSize(120.0f, 30.0f);
    ipLabel_->setBackgroundColor(UIColor::Transparent());
    ipLabel_->setTextColor(UIColor(180, 200, 255, 255));
    joinPanel_->addChild(ipLabel_);

    ipInput_ = std::make_shared<InputFieldWidget>("127.0.0.1");
    ipInput_->setPosition(150.0f, 65.0f);
    ipInput_->setSize(250.0f, 40.0f);
    ipInput_->setText(config_.defaultIP);
    ipInput_->setVisible(true);
    joinPanel_->addChild(ipInput_);

    // Port input
    portLabel_ = std::make_shared<TextWidget>("Port:", config_.textFontSize);
    portLabel_->setPosition(20.0f, 130.0f);
    portLabel_->setSize(120.0f, 30.0f);
    portLabel_->setBackgroundColor(UIColor::Transparent());
    portLabel_->setTextColor(UIColor(180, 200, 255, 255));
    joinPanel_->addChild(portLabel_);

    portInput_ = std::make_shared<InputFieldWidget>("4242");
    portInput_->setPosition(150.0f, 125.0f);
    portInput_->setSize(150.0f, 40.0f);
    portInput_->setText(std::to_string(config_.defaultPort));
    portInput_->setVisible(true);
    joinPanel_->addChild(portInput_);

    // Room name input for direct room joining
    roomNameJoinLabel_ = std::make_shared<TextWidget>("Room Name:", config_.textFontSize);
    roomNameJoinLabel_->setPosition(20.0f, 190.0f);
    roomNameJoinLabel_->setSize(120.0f, 30.0f);
    roomNameJoinLabel_->setBackgroundColor(UIColor::Transparent());
    roomNameJoinLabel_->setTextColor(UIColor(180, 200, 255, 255));
    joinPanel_->addChild(roomNameJoinLabel_);

    roomNameJoinInput_ = std::make_shared<InputFieldWidget>("Enter room name");
    roomNameJoinInput_->setPosition(150.0f, 185.0f);
    roomNameJoinInput_->setSize(250.0f, 40.0f);
    roomNameJoinInput_->setText("");
    roomNameJoinInput_->setVisible(true);
    joinPanel_->addChild(roomNameJoinInput_);

    // Join by server button
    joinServerButton_ = std::make_shared<ButtonWidget>("JOIN SERVER");
    joinServerButton_->setPosition(150.0f, 280.0f);
    joinServerButton_->setSize(150.0f, 40.0f);
    joinServerButton_->setBackgroundColor(UIColor(0, 120, 180, 200));
    joinServerButton_->setBorderColor(UIColor(0, 150, 220, 255));
    joinServerButton_->setBorderWidth(2.0f);
    joinServerButton_->setTextColor(UIColor::White());
    joinServerButton_->setOnClick([this]() {
        std::string ip = ipInput_->getText();
        std::string portStr = portInput_->getText();
        
        try {
            int port = std::stoi(portStr);
            std::cout << "Joining server: " << ip << ":" << port << std::endl;
            if (callbacks_.onJoinServer) {
                callbacks_.onJoinServer(ip, port);
            }
        } catch (const std::exception& e) {
            std::cout << "Invalid port number: " << portStr << std::endl;
        }
    });
    joinPanel_->addChild(joinServerButton_);

    // Join by room name button
    joinRoomButton_ = std::make_shared<ButtonWidget>("JOIN ROOM");
    joinRoomButton_->setPosition(320.0f, 280.0f);
    joinRoomButton_->setSize(150.0f, 40.0f);
    joinRoomButton_->setBackgroundColor(UIColor(0, 150, 0, 200));
    joinRoomButton_->setBorderColor(UIColor(0, 255, 0, 255));
    joinRoomButton_->setBorderWidth(2.0f);
    joinRoomButton_->setTextColor(UIColor::White());
    joinRoomButton_->setOnClick([this]() {
        std::string roomName = roomNameJoinInput_->getText();
        
        if (roomName.empty()) {
            std::cout << "Please enter a room name" << std::endl;
            return;
        }
        
        std::cout << "Joining room: " << roomName << std::endl;
        // For now, use default server settings when joining by room name
        if (callbacks_.onJoinServer) {
            callbacks_.onJoinServer(config_.defaultIP, config_.defaultPort);
        }
    });
    joinPanel_->addChild(joinRoomButton_);

    // Room list panel (right side)
    roomListPanel_ = std::make_shared<PanelWidget>();
    roomListPanel_->setPosition(480.0f, 60.0f);
    roomListPanel_->setSize(420.0f, 260.0f);
    roomListPanel_->setBackgroundColor(UIColor(2, 8, 15, 180));
    roomListPanel_->setBorderColor(UIColor(80, 120, 180, 255));
    roomListPanel_->setBorderWidth(1.0f);
    joinPanel_->addChild(roomListPanel_);

    // Room list title
    roomListTitle_ = std::make_shared<TextWidget>("Available Rooms", config_.textFontSize);
    roomListTitle_->setPosition(20.0f, 10.0f);
    roomListTitle_->setSize(200.0f, 30.0f);
    roomListTitle_->setBackgroundColor(UIColor::Transparent());
    roomListTitle_->setTextColor(UIColor(150, 180, 255, 255));
    roomListPanel_->addChild(roomListTitle_);

    // Room list content - initially empty, will be populated by refreshRoomList()
    roomListContent_ = std::make_shared<TextWidget>("Fetching rooms from server...", config_.textFontSize - 4);
    roomListContent_->setPosition(20.0f, 45.0f);
    roomListContent_->setSize(400.0f, 160.0f);
    roomListContent_->setBackgroundColor(UIColor::Transparent());
    roomListContent_->setTextColor(UIColor(180, 200, 240, 255));
    roomListPanel_->addChild(roomListContent_);

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
    roomListPanel_->addChild(refreshButton);

    // Initial room list fetch
    refreshRoomList();
}

void MultiplayerWidget::createCreateInterface() {
    auto contentBounds = mainPanel_->getContentBounds();

    // Create panel
    createPanel_ = std::make_shared<PanelWidget>();
    createPanel_->setPosition(30.0f, 160.0f);
    createPanel_->setSize(contentBounds.width - 60.0f, 420.0f);
    createPanel_->setBackgroundColor(UIColor(5, 15, 30, 150));
    createPanel_->setBorderColor(UIColor(50, 100, 150, 255));
    createPanel_->setBorderWidth(1.0f);
    mainPanel_->addChild(createPanel_);

    // Instructions
    createInstructionsText_ = std::make_shared<TextWidget>("Customize and create your room:", config_.textFontSize);
    createInstructionsText_->setPosition(20.0f, 20.0f);
    createInstructionsText_->setSize(600.0f, 30.0f);
    createInstructionsText_->setBackgroundColor(UIColor::Transparent());
    createInstructionsText_->setTextColor(UIColor(200, 220, 255, 255));
    createPanel_->addChild(createInstructionsText_);

    // Room name input
    roomNameLabel_ = std::make_shared<TextWidget>("Room Name:", config_.textFontSize - 4);
    roomNameLabel_->setPosition(20.0f, 60.0f);
    roomNameLabel_->setSize(120.0f, 25.0f);
    roomNameLabel_->setBackgroundColor(UIColor::Transparent());
    roomNameLabel_->setTextColor(UIColor(180, 200, 255, 255));
    createPanel_->addChild(roomNameLabel_);

    roomNameInput_ = std::make_shared<InputFieldWidget>(currentRoomSettings_.name);
    roomNameInput_->setPosition(150.0f, 55.0f);
    roomNameInput_->setSize(250.0f, 35.0f);
    createPanel_->addChild(roomNameInput_);

    // Password input
    passwordLabel_ = std::make_shared<TextWidget>("Password:", config_.textFontSize - 4);
    passwordLabel_->setPosition(450.0f, 60.0f);
    passwordLabel_->setSize(100.0f, 25.0f);
    passwordLabel_->setBackgroundColor(UIColor::Transparent());
    passwordLabel_->setTextColor(UIColor(180, 200, 255, 255));
    createPanel_->addChild(passwordLabel_);

    passwordInput_ = std::make_shared<InputFieldWidget>(currentRoomSettings_.password);
    passwordInput_->setPosition(560.0f, 55.0f);
    passwordInput_->setSize(200.0f, 35.0f);
    createPanel_->addChild(passwordInput_);

    // Background color selection
    colorLabel_ = std::make_shared<TextWidget>("Room Background Color:", config_.textFontSize - 4);
    colorLabel_->setPosition(20.0f, 110.0f);
    colorLabel_->setSize(200.0f, 25.0f);
    colorLabel_->setBackgroundColor(UIColor::Transparent());
    colorLabel_->setTextColor(UIColor(180, 200, 255, 255));
    createPanel_->addChild(colorLabel_);

    // Color preview
    colorPreview_ = std::make_shared<PanelWidget>();
    colorPreview_->setPosition(250.0f, 105.0f);
    colorPreview_->setSize(100.0f, 35.0f);
    colorPreview_->setBackgroundColor(currentRoomSettings_.backgroundColor);
    colorPreview_->setBorderColor(UIColor::White());
    colorPreview_->setBorderWidth(2.0f);
    createPanel_->addChild(colorPreview_);

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
        
        // Capture color by value in lambda
        UIColor selectedColor = colorOptions[i];
        colorButton->setOnClick([this, selectedColor]() {
            currentRoomSettings_.backgroundColor = selectedColor;
            if (colorPreview_) {
                colorPreview_->setBackgroundColor(selectedColor);
            }
            std::cout << "Selected room color: " << selectedColor.getRed() << "," << selectedColor.getGreen() << "," << selectedColor.getBlue() << std::endl;
        });
        
        createPanel_->addChild(colorButton);
        colorButtons_.push_back(colorButton);
    }

    // Create room button
    createRoomButton_ = std::make_shared<ButtonWidget>("CREATE ROOM");
    createRoomButton_->setPosition(contentBounds.width - 275.0f, 355.0f);
    createRoomButton_->setSize(200.0f, 50.0f);
    createRoomButton_->setBackgroundColor(UIColor(0, 100, 200, 200));
    createRoomButton_->setBorderColor(UIColor(0, 150, 255, 255));
    createRoomButton_->setBorderWidth(2.0f);
    createRoomButton_->setTextColor(UIColor::White());
    createRoomButton_->setOnClick([this]() {
        // Update room settings from input fields
        if (roomNameInput_) {
            currentRoomSettings_.name = roomNameInput_->getText();
        }
        if (passwordInput_) {
            currentRoomSettings_.password = passwordInput_->getText();
        }
        
        // Validate room name
        if (currentRoomSettings_.name.empty()) {
            showError("Room name cannot be empty!");
            return;
        }
        
        // Check network connectivity before creating room
        if (callbacks_.checkNetworkConnection && !callbacks_.checkNetworkConnection()) {
            showError("Cannot create room: Not connected to server.\n\nPlease ensure that:\n• A server is running\n• Network connection is available\n• Server address is correct");
            return;
        }
        
        std::cout << "Creating room '" << currentRoomSettings_.name << "' with background color " 
                  << currentRoomSettings_.backgroundColor.getRed() << "," << currentRoomSettings_.backgroundColor.getGreen() 
                  << "," << currentRoomSettings_.backgroundColor.getBlue() << std::endl;
        
        if (callbacks_.onCreateRoom) {
            callbacks_.onCreateRoom(currentRoomSettings_);
        }
    });
    createPanel_->addChild(createRoomButton_);
}

void MultiplayerWidget::createNavigation() {
    auto contentBounds = mainPanel_->getContentBounds();

    // Back button
    joinBackButton_ = std::make_shared<ButtonWidget>("BACK TO MENU");
    joinBackButton_->setPosition(contentBounds.width - 270.0f, 270.0f);
    joinBackButton_->setSize(200.0f, 50.0f);
    joinBackButton_->setBackgroundColor(UIColor(100, 100, 100, 200));
    joinBackButton_->setBorderColor(UIColor(150, 150, 150, 255));
    joinBackButton_->setBorderWidth(2.0f);
    joinBackButton_->setTextColor(UIColor::White());
    joinBackButton_->setOnClick([this]() {
        std::cout << "Back to main menu from multiplayer..." << std::endl;
        if (callbacks_.onBack) {
            callbacks_.onBack();
        }
    });
    joinPanel_->addChild(joinBackButton_);

    // Back button
    createBackButton_ = std::make_shared<ButtonWidget>("BACK TO MENU");
    createBackButton_->setPosition(15.0f, 355.0f);
    createBackButton_->setSize(200.0f, 50.0f);
    createBackButton_->setBackgroundColor(UIColor(100, 100, 100, 200));
    createBackButton_->setBorderColor(UIColor(150, 150, 150, 255));
    createBackButton_->setBorderWidth(2.0f);
    createBackButton_->setTextColor(UIColor::White());
    createBackButton_->setOnClick([this]() {
        std::cout << "Back to main menu from multiplayer..." << std::endl;
        if (callbacks_.onBack) {
            callbacks_.onBack();
        }
    });
    createPanel_->addChild(createBackButton_);
}

void MultiplayerWidget::switchTab(MultiplayerTab tab) {
    currentTab_ = tab;
    updateTabButtons();
    updateTabContent();
}

void MultiplayerWidget::updateTabButtons() {
    // Update join tab button
    if (currentTab_ == MultiplayerTab::JOIN) {
        joinTabButton_->setBackgroundColor(UIColor(0, 100, 200, 255)); // Active blue
        joinTabButton_->setBorderColor(UIColor(0, 150, 255, 255));
    } else {
        joinTabButton_->setBackgroundColor(UIColor(60, 60, 80, 200)); // Inactive gray
        joinTabButton_->setBorderColor(UIColor(100, 100, 120, 255));
    }
    joinTabButton_->setBorderWidth(2.0f);

    // Update create tab button
    if (currentTab_ == MultiplayerTab::CREATE) {
        createTabButton_->setBackgroundColor(UIColor(0, 100, 200, 255)); // Active blue
        createTabButton_->setBorderColor(UIColor(0, 150, 255, 255));
    } else {
        createTabButton_->setBackgroundColor(UIColor(60, 60, 80, 200)); // Inactive gray
        createTabButton_->setBorderColor(UIColor(100, 100, 120, 255));
    }
    createTabButton_->setBorderWidth(2.0f);
}

void MultiplayerWidget::updateTabContent() {
    // Show/hide panels based on current tab
    joinPanel_->setVisible(currentTab_ == MultiplayerTab::JOIN);
    createPanel_->setVisible(currentTab_ == MultiplayerTab::CREATE);
}

void MultiplayerWidget::show() {
    setVisible(true);
    if (mainPanel_) {
        mainPanel_->setVisible(true);
        updateTabContent(); // Ensure correct tab content is visible
    }
}

void MultiplayerWidget::hide() {
    setVisible(false);
    if (mainPanel_) {
        mainPanel_->setVisible(false);
    }
}

bool MultiplayerWidget::isVisible() const {
    return Widget::isVisible();
}

void MultiplayerWidget::renderSelf() const {
    // The main panel and its children handle their own rendering
}

void MultiplayerWidget::showError(const std::string& message) {
    if (errorMessageWidget_) {
        errorMessageWidget_->setMessage(message);
        errorMessageWidget_->show();
    }
}

void MultiplayerWidget::refreshRoomList() {
    if (!callbacks_.onGetRoomList) {
        if (roomListContent_) {
            roomListContent_->setText("No room list service available.\n\nClick to retry...");
        }
        return;
    }

    try {
        // Fetch room list from server
        std::vector<RoomInfo> rooms = callbacks_.onGetRoomList();
        
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
        
        if (roomListContent_) {
            roomListContent_->setText(roomListText);
        }
        
        std::cout << "Room list updated - found " << rooms.size() << " rooms" << std::endl;
        
    } catch (const std::exception& e) {
        if (roomListContent_) {
            roomListContent_->setText("Failed to fetch rooms.\n\nServer may be offline.\n\nClick to retry...");
        }
        std::cout << "Error fetching room list: " << e.what() << std::endl;
    }
}

} // namespace rtype::ui