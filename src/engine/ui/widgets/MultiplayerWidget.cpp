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
    // Set default size and position
    setSize(800.0f, 600.0f);
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
    mainPanel_->setPosition(0.0f, 0.0f);
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
    joinPanel_->setPosition(50.0f, 160.0f);
    joinPanel_->setSize(contentBounds.width - 100.0f, 300.0f);
    joinPanel_->setBackgroundColor(UIColor(5, 15, 30, 150));
    joinPanel_->setBorderColor(UIColor(50, 100, 150, 255));
    joinPanel_->setBorderWidth(1.0f);
    mainPanel_->addChild(joinPanel_);

    // Instructions
    joinInstructionsText_ = std::make_shared<TextWidget>("Enter server details to join an existing game:", config_.textFontSize);
    joinInstructionsText_->setPosition(20.0f, 20.0f);
    joinInstructionsText_->setSize(600.0f, 30.0f);
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
    ipInput_->setSize(300.0f, 40.0f);
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

    // Join button
    joinServerButton_ = std::make_shared<ButtonWidget>("JOIN SERVER");
    joinServerButton_->setPosition(250.0f, 200.0f);
    joinServerButton_->setSize(200.0f, 50.0f);
    joinServerButton_->setBackgroundColor(UIColor(0, 150, 0, 200));
    joinServerButton_->setBorderColor(UIColor(0, 255, 0, 255));
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
}

void MultiplayerWidget::createCreateInterface() {
    auto contentBounds = mainPanel_->getContentBounds();

    // Create panel
    createPanel_ = std::make_shared<PanelWidget>();
    createPanel_->setPosition(50.0f, 160.0f);
    createPanel_->setSize(contentBounds.width - 100.0f, 300.0f);
    createPanel_->setBackgroundColor(UIColor(5, 15, 30, 150));
    createPanel_->setBorderColor(UIColor(50, 100, 150, 255));
    createPanel_->setBorderWidth(1.0f);
    mainPanel_->addChild(createPanel_);

    // Instructions
    createInstructionsText_ = std::make_shared<TextWidget>("Create a new game room and wait for players to join:", config_.textFontSize);
    createInstructionsText_->setPosition(20.0f, 20.0f);
    createInstructionsText_->setSize(600.0f, 30.0f);
    createInstructionsText_->setBackgroundColor(UIColor::Transparent());
    createInstructionsText_->setTextColor(UIColor(200, 220, 255, 255));
    createPanel_->addChild(createInstructionsText_);

    // Create room button
    createRoomButton_ = std::make_shared<ButtonWidget>("CREATE ROOM");
    createRoomButton_->setPosition(250.0f, 120.0f);
    createRoomButton_->setSize(200.0f, 60.0f);
    createRoomButton_->setBackgroundColor(UIColor(0, 100, 200, 200));
    createRoomButton_->setBorderColor(UIColor(0, 150, 255, 255));
    createRoomButton_->setBorderWidth(2.0f);
    createRoomButton_->setTextColor(UIColor::White());
    createRoomButton_->setOnClick([this]() {
        // Check network connectivity before creating room
        if (callbacks_.checkNetworkConnection && !callbacks_.checkNetworkConnection()) {
            showError("Cannot create room: Not connected to server.\n\nPlease ensure that:\n• A server is running\n• Network connection is available\n• Server address is correct");
            return;
        }
        
        std::cout << "Creating new room..." << std::endl;
        if (callbacks_.onCreateRoom) {
            callbacks_.onCreateRoom();
        }
    });
    createPanel_->addChild(createRoomButton_);
}

void MultiplayerWidget::createNavigation() {
    auto contentBounds = mainPanel_->getContentBounds();

    // Back button
    backButton_ = std::make_shared<ButtonWidget>("BACK TO MENU");
    backButton_->setPosition(50.0f, contentBounds.height - 80.0f);
    backButton_->setSize(200.0f, 50.0f);
    backButton_->setBackgroundColor(UIColor(100, 100, 100, 200));
    backButton_->setBorderColor(UIColor(150, 150, 150, 255));
    backButton_->setBorderWidth(2.0f);
    backButton_->setTextColor(UIColor::White());
    backButton_->setOnClick([this]() {
        std::cout << "Back to main menu from multiplayer..." << std::endl;
        if (callbacks_.onBack) {
            callbacks_.onBack();
        }
    });
    mainPanel_->addChild(backButton_);
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

} // namespace rtype::ui