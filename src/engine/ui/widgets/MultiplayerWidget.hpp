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
    // Configuration
    MultiplayerConfig config_;
    MultiplayerCallbacks callbacks_;

    // UI Components
    std::shared_ptr<PanelWidget> mainPanel_;
    std::shared_ptr<TextWidget> titleText_;
    
    // Tab buttons
    std::shared_ptr<ButtonWidget> joinTabButton_;
    std::shared_ptr<ButtonWidget> createTabButton_;
    
    // Join tab components
    std::shared_ptr<PanelWidget> joinPanel_;
    std::shared_ptr<TextWidget> joinInstructionsText_;
    std::shared_ptr<TextWidget> ipLabel_;
    std::shared_ptr<InputFieldWidget> ipInput_;
    std::shared_ptr<TextWidget> portLabel_;
    std::shared_ptr<InputFieldWidget> portInput_;
    std::shared_ptr<TextWidget> roomNameJoinLabel_;
    std::shared_ptr<InputFieldWidget> roomNameJoinInput_;
    std::shared_ptr<ButtonWidget> joinServerButton_;
    std::shared_ptr<ButtonWidget> joinRoomButton_;
    
    // Room list components
    std::shared_ptr<PanelWidget> roomListPanel_;
    std::shared_ptr<TextWidget> roomListTitle_;
    std::shared_ptr<TextWidget> roomListContent_;
    
    // Create tab components
    std::shared_ptr<PanelWidget> createPanel_;
    std::shared_ptr<TextWidget> createInstructionsText_;
    
    // Room customization components
    std::shared_ptr<TextWidget> roomNameLabel_;
    std::shared_ptr<InputFieldWidget> roomNameInput_;
    std::shared_ptr<TextWidget> passwordLabel_;
    std::shared_ptr<InputFieldWidget> passwordInput_;
    std::shared_ptr<TextWidget> colorLabel_;
    std::shared_ptr<PanelWidget> colorPreview_;
    std::vector<std::shared_ptr<ButtonWidget>> colorButtons_;
    
    std::shared_ptr<ButtonWidget> createRoomButton_;
    
    // Navigation
    std::shared_ptr<ButtonWidget> joinBackButton_;
    std::shared_ptr<ButtonWidget> createBackButton_;
    
    // Error handling
    std::shared_ptr<ErrorMessageWidget> errorMessageWidget_;

    // State
    bool initialized_;
    MultiplayerTab currentTab_;
    RoomSettings currentRoomSettings_;

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