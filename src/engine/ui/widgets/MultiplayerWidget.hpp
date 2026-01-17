#pragma once

#include "../Widget.hpp"
#include "PanelWidget.hpp"
#include "ButtonWidget.hpp"
#include "TextWidget.hpp"
#include "InputFieldWidget.hpp"
#include "ErrorMessageWidget.hpp"
#include <memory>
#include <functional>
#include <string>

namespace rtype::ui {

/**
 * @brief Configuration structure for multiplayer
 */
struct MultiplayerConfig {
    std::string title = "MULTIPLAYER";
    int titleFontSize = 48;
    int textFontSize = 20;
    std::string defaultIP = "127.0.0.1";
    int defaultPort = 4242;
};

/**
 * @brief Tab types for multiplayer interface
 */
enum class MultiplayerTab {
    JOIN,
    CREATE
};

/**
 * @brief Callback functions for multiplayer events
 */
/**
 * @brief Room customization settings
 */
struct RoomSettings {
    std::string name = "My Room";
    std::string password = "";
    UIColor backgroundColor = UIColor(10, 20, 40, 220);
};

/**
 * @brief Information about an available room
 */
struct RoomInfo {
    std::string name;
    int currentPlayers;
    int maxPlayers;
    bool isPasswordProtected;
    bool isFull() const { return currentPlayers >= maxPlayers; }
};

/**
 * @brief Callback functions for multiplayer events
 */
struct MultiplayerCallbacks {
    std::function<void()> onBack;
    std::function<void(const std::string& ip, int port)> onJoinServer;
    std::function<void(const RoomSettings& settings)> onCreateRoom;
    std::function<bool()> checkNetworkConnection;  // Returns true if connected to server
    std::function<void(const std::string&)> onNetworkError;  // Called when network error occurs
    std::function<std::vector<RoomInfo>()> onGetRoomList;  // Fetch available rooms from server
};

/**
 * @brief Multiplayer widget with tabbed interface for Join/Create functionality
 * 
 * This widget provides:
 * - Tab selection between Join Server and Create Room
 * - IP address and port input for joining servers
 * - Room creation interface
 * - Navigation back to main menu
 */
class MultiplayerWidget : public Widget {
public:
    /**
     * @brief Constructor
     * @param config Initial configuration for the multiplayer interface
     */
    explicit MultiplayerWidget(const MultiplayerConfig& config = MultiplayerConfig{});

    /**
     * @brief Set callback functions for multiplayer events
     * @param callbacks Structure containing callback functions
     */
    void setCallbacks(const MultiplayerCallbacks& callbacks);

    /**
     * @brief Initialize the widget after adding to UI manager
     */
    void initialize();

    /**
     * @brief Show the multiplayer interface
     */
    void show();

    /**
     * @brief Hide the multiplayer interface
     */
    void hide();

    /**
     * @brief Check if the interface is currently visible
     */
    bool isVisible() const;

    /**
     * @brief Render the widget
     */
    void renderSelf() const override;

    /**
     * @brief Switch to a specific tab
     * @param tab The tab to switch to
     */
    void switchTab(MultiplayerTab tab);

    /**
     * @brief Refresh the room list with current server data
     */
    void refreshRoomList();

private:
    /**
     * @brief Configuration for the multiplayer interface
     */
    MultiplayerConfig _config;
    /**
     * @brief Callback functions for multiplayer events
     */
    MultiplayerCallbacks _callbacks;

    /**
     * @brief Main panel containing all UI elements
     */
    std::shared_ptr<PanelWidget> _mainPanel;
    /**
     * @brief Title text widget
     */
    std::shared_ptr<TextWidget> _titleText;
    
    /**
     * @brief Tab buttons
     */
    std::shared_ptr<ButtonWidget> _joinTabButton;
    /**
     * @brief Create tab button
     */
    std::shared_ptr<ButtonWidget> _createTabButton;
    
    /**
     * @brief Join tab components
     */
    std::shared_ptr<PanelWidget> _joinPanel;
    /**
     * @brief Instructions text for joining
     */
    std::shared_ptr<TextWidget> _joinInstructionsText;
    /**
     * @brief IP address and port input fields
     */
    std::shared_ptr<TextWidget> _ipLabel;
    /**
     * @brief IP address input field
     */
    std::shared_ptr<InputFieldWidget> _ipInput;
    /**
     * @brief Port label
     */
    std::shared_ptr<TextWidget> _portLabel;
    /**
     * @brief Port input field
     */
    std::shared_ptr<InputFieldWidget> _portInput;
    /**
     * @brief Join Server button
     */
    std::shared_ptr<ButtonWidget> _joinServerButton;
    /**
     * @brief Room list panel
     */
    std::shared_ptr<PanelWidget> _roomListPanel;
    /**
     * @brief Room list title text
     */
    std::shared_ptr<TextWidget> _roomListTitle;
    /**
     * @brief Room list content text
     */
    std::shared_ptr<TextWidget> _roomListContent;
    
    /**
     * @brief Create tab components
     */
    std::shared_ptr<PanelWidget> _createPanel;
    /**
     * @brief Instructions text for creating a room
     */
    std::shared_ptr<TextWidget> _createInstructionsText;
    
    /**
     * @brief Room creation fields
     */
    std::shared_ptr<TextWidget> _roomNameLabel;
    /**
     * @brief Room name input field for creation
     */
    std::shared_ptr<InputFieldWidget> _roomNameInput;
    /**
     * @brief Password label
     */
    std::shared_ptr<TextWidget> _passwordLabel;
    /**
     * @brief Password input field
     */
    std::shared_ptr<InputFieldWidget> _passwordInput;
    /**
     * @brief Background color selection
     */
    std::shared_ptr<TextWidget> _colorLabel;
    /**
     * @brief Color preview panel
     */
    std::shared_ptr<PanelWidget> _colorPreview;
    /**
     * @brief Color selection buttons
     */
    std::vector<std::shared_ptr<ButtonWidget>> _colorButtons;
    /**
     * @brief Create Room button
     */
    std::shared_ptr<ButtonWidget> _createRoomButton;
    
    /**
     * @brief Join back button for navigation
     */
    std::shared_ptr<ButtonWidget> _joinBackButton;
    /**
     * @brief Create back button for navigation
     */
    std::shared_ptr<ButtonWidget> _createBackButton;
    
    /**
     * @brief Error message dialog
     */
    std::shared_ptr<ErrorMessageWidget> _errorMessageWidget;

    /**
     * @brief Whether the widget has been initialized
     */
    bool _initialized;
    /**
     * @brief Currently selected tab
     */
    MultiplayerTab _currentTab;
    /**
     * @brief Current room settings for creation
     */
    RoomSettings _currentRoomSettings;
    /**
     * @brief Create the main panel and title
     */
    void createMainPanel();

    /**
     * @brief Create tab buttons
     */
    void createTabButtons();

    /**
     * @brief Create join server interface
     */
    void createJoinInterface();

    /**
     * @brief Create room creation interface
     */
    void createCreateInterface();

    /**
     * @brief Create navigation buttons
     */
    void createNavigation();

    /**
     * @brief Update tab button appearances
     */
    void updateTabButtons();

    /**
     * @brief Update content visibility based on current tab
     */
    void updateTabContent();

    /**
     * @brief Show error message dialog
     * @param message The error message to display
     */
    void showError(const std::string& message);
};

} // namespace rtype::ui