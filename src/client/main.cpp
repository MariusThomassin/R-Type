/*
** R-Type Client
** Main game client with ECS architecture
** Uses fixed timestep for game logic, uncapped rendering
** Event-driven input via EventBus
*/

// Windows: Must include winsock2.h before windows.h (required by ASIO)
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#endif

#include <raylib.h>

#include <iostream>
#include <string>
#include "../engine/ecs/ECS.hpp"
#include "../engine/ecs/core/EventBus.hpp"
#include "../engine/ecs/events/Events.hpp"
#include "../engine/ecs/core/SystemManager.hpp"
#include "../game/Components.hpp"
#include "../game/Systems.hpp"
#include "../game/systems/WindowedDebugSystem.hpp"
#include "../game/systems/DebugSystem.hpp"
#include "../engine/ecs/systems/MusicSystem.hpp"
#include "../engine/ecs/systems/BackgroundSystem.hpp"
#include "../engine/ecs/components/ImageBackgroundComponent.hpp"
#include "../engine/ui/UIManager.hpp"
#include "../engine/ui/widgets/ButtonWidget.hpp"
#include "../engine/ui/widgets/TextWidget.hpp"
#include "../engine/ui/widgets/PanelWidget.hpp"
#include "../engine/ui/widgets/SettingsWidget.hpp"
#include "../engine/ui/widgets/MainMenuWidget.hpp"
#include "../engine/ui/widgets/LobbyWidget.hpp"
#include "../engine/ui/widgets/MultiplayerWidget.hpp"
#include "../engine/graphics/RenderUtils.hpp"
#include "../shared/SettingsManager.hpp"
#include "../engine/ecs/events/InputUtils.hpp"
#include "NetworkClient.hpp"
#include "LocalServer.hpp"
#include "ProfileManager.hpp"
#include "ScoreManager.hpp"
#include "../game/components/FloatingTextComponent.hpp"
#include "../shared/PathUtils.hpp"

using namespace rtype::ecs;
using rtype::ecs::BulletType;
using rtype::ecs::BulletColor;
using rtype::ui::UIColor;

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;  // 60 Hz game logic

// MusicSystem pointer for callbacks (set in main)
static rtype::ecs::MusicSystem* g_musicSystem = nullptr;

/**
 * @brief Initialize music system (called after MusicSystem is created)
 */
void initializeMusic() {
    if (g_musicSystem) {
        std::cout << "MusicSystem initialized and ready" << std::endl;
    }
}

/**
 * @brief Update music playback (call every frame)
 */
void updateMusic() {
    // Kept for compatibility - MusicSystem::update() is called via SystemManager
}

/**
 * @brief Set music enabled/disabled state
 */
void setMusicEnabled(bool enabled) {
    if (g_musicSystem) {
        if (enabled) {
            g_musicSystem->resume();
        } else {
            g_musicSystem->pause();
        }
        std::cout << "Music " << (enabled ? "enabled" : "disabled") << std::endl;
    }
}

/**
 * @brief Set music volume (0.0 to 1.0)
 */
void setMusicVolume(float volume) {
    if (g_musicSystem) {
        g_musicSystem->setMasterVolume(volume);
    }
}

/**
 * @brief Clean up music resources
 */
void cleanupMusic() {
    std::cout << "Music cleanup handled by MusicSystem" << std::endl;
}

/**
 * @brief Create the player entity with all necessary components
 */
Entity createPlayer(Registry& registry, int playerId) { 
    Entity player = registry.createEntity();

    registry.addComponent(player, TransformComponent(
        150.0f,                           // x - left side
        SCREEN_HEIGHT / 2.0f,             // y - center
        0.0f,                             // rotation
        1.5f, 1.5f                        // scale
    ));

    registry.addComponent(player, VelocityComponent(0.0f, 0.0f, 400.0f));

    PlayerShipComponent ship(PlayerShipComponent::ShipStyle::Classic);
    ship.layer = 10;
    registry.addComponent(player, ship);

    registry.addComponent(player, PlayerComponent(playerId, 3));

    WeaponComponent weapon(0.15f, 10);
    weapon.projectileSpeed = 800.0f;
    registry.addComponent(player, weapon);

    return player;
}

/**
 * @brief Create the scrolling background entity (legacy - use BackgroundSystem instead)
 * @deprecated Use BackgroundSystem::createDefaultBackground() instead
 */
Entity createBackground(Registry& registry, int screenWidth, int screenHeight) {
    Entity bg = registry.createEntity();
    
    registry.addComponent(bg, TransformComponent(0, 0));
    
    BackgroundComponent background(screenWidth, screenHeight, 200, 100.0f);
    background.layer = -100;  // Render behind everything
    registry.addComponent(bg, background);
    
    return bg;
}

/**
 * @brief Clamp player position to screen bounds
 */
void clampPlayerToScreen(Registry& registry) {
    constexpr float MARGIN = 30.0f;

    registry.forEach<PlayerComponent, TransformComponent>(
        [&registry](EntityId entity) {
            auto& transform = registry.getComponent<TransformComponent>(entity);

            if (transform.x < MARGIN) transform.x = MARGIN;
            if (transform.x > SCREEN_WIDTH - MARGIN) transform.x = SCREEN_WIDTH - MARGIN;
            if (transform.y < MARGIN) transform.y = MARGIN;
            if (transform.y > SCREEN_HEIGHT - MARGIN) transform.y = SCREEN_HEIGHT - MARGIN;
        }
    );
}

int main(int argc, char* argv[]) {
    // ==================== Parse Command-Line Arguments ====================
    std::string serverIp = "127.0.0.1";  // Default: localhost
    int serverPort = 4242;                // Default: 4242

    if (argc >= 2) {
        serverIp = argv[1];
    }
    if (argc >= 3) {
        try {
            serverPort = std::stoi(argv[2]);
        } catch (const std::exception& e) {
            std::cerr << "[Main] Invalid port number: " << argv[2] << ", using default 4242" << std::endl;
            serverPort = 4242;
        }
    }

    std::cout << "[Main] Connecting to server: " << serverIp << ":" << serverPort << std::endl;

    // ==================== Raylib Initialization ====================
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "R-Type");
    ClearWindowState(FLAG_WINDOW_RESIZABLE);  // Fixed window size to prevent FPS drops on resize
    SetExitKey(KEY_NULL); // Disable default ESC key behavior - we handle ESC ourselves
    
    // ==================== Audio Initialization ====================
    InitAudioDevice();  // Explicitly initialize audio device
    std::cout << "Audio device initialized. Ready: " << (IsAudioDeviceReady() ? "YES" : "NO") << std::endl;

    // ==================== Event Bus ====================
    rtype::ecs::EventBus eventBus;

    // ==================== Settings Manager ====================
    rtype::SettingsManager settingsManager;
    // Try to load settings from file, otherwise use defaults
    if (!settingsManager.load("config/settings.json")) {
        std::cout << "[Main] No settings file found, using defaults" << std::endl;
    } else {
        std::cout << "[Main] Settings loaded from config/settings.json" << std::endl;
    }

    // ==================== Input Manager ====================
    rtype::ecs::events::InputManager inputManager(eventBus, &settingsManager);

    // ==================== UI Manager ====================
    rtype::ui::UIManager uiManager(eventBus);

    // UIManager automatically subscribes to events via its constructor // Optional for testing focus/text

    // ==================== Game State Management ====================
    GameState gameState = GameState::MENU;
    GameState previousState = GameState::MENU; // Track where we came from for settings
    
    // ==================== Settings Panel ====================
    bool showingLobby = false;
    bool showingMultiplayer = false;
    
    // ==================== ECS Setup ====================
    Registry registry;
    SystemManager systems(&registry);
    bool shouldExit = false;

    // ==================== Network Setup ====================
    // NetworkClient is created but NOT connected at startup
    // Connection happens through the multiplayer menu when user selects a server
    rtype::client::NetworkClient networkClient(registry);

    // ==================== Local Server (for Solo Mode) ====================
    // LocalServer runs a full GameServer in a background thread
    // This ensures solo mode uses the same authoritative game logic as multiplayer
    rtype::client::LocalServer localServer;
    bool isLocalGame = false;  // Track if we're running a local server

    // ==================== Profile & Score Management ====================
    rtype::client::ProfileManager profileManager;
    rtype::client::ScoreManager scoreManager;
    
    // Set up network callbacks for score tracking
    rtype::client::NetworkCallbacks netCallbacks;
    netCallbacks.onScoreUpdate = [&scoreManager, &networkClient, &registry](const rtype::network::ScoreUpdateMessage& msg) {
        if (msg.clientId == networkClient.getClientId()) {
            scoreManager.updateSessionScore(msg.newScore);
        }
        
        // Spawn floating text for score popups (if position is provided)
        if (msg.delta > 0 && (msg.scoreX != 0.0f || msg.scoreY != 0.0f)) {
            Entity floatText = registry.createEntity();
            std::string scoreStr = "+" + std::to_string(msg.delta);
            registry.addComponent(floatText, FloatingTextComponent(scoreStr, msg.scoreX, msg.scoreY, msg.delta));
        }
    };
    netCallbacks.onLevelComplete = [&scoreManager](const rtype::network::LevelCompleteMessage& msg) {
        scoreManager.setSessionProgress(msg.levelIndex + 1, msg.nextLevelIndex < msg.levelIndex);
    };
    
    netCallbacks.onLevelInfo = [&eventBus](const rtype::network::LevelInfoMessage& msg) {
        std::cout << "[Main] Level info received, loading assets: bg=" << msg.backgroundPath 
                  << ", music=" << msg.stageMusicPath << std::endl;
        
        rtype::ecs::events::LevelAssetsLoaded assetsEvent;
        
        // Use PathUtils to resolve paths - works in both build/ and dist/ layouts
        if (msg.backgroundPath[0] != '\0') {
            assetsEvent.backgroundPath = rtype::resolveAssetPath(msg.backgroundPath);
        }
        if (msg.stageMusicPath[0] != '\0') {
            assetsEvent.stageMusicPath = rtype::resolveAssetPath(msg.stageMusicPath);
        }
        if (msg.bossMusicPath[0] != '\0') {
            assetsEvent.bossMusicPath = rtype::resolveAssetPath(msg.bossMusicPath);
        }
        
        assetsEvent.hasBackground = (msg.backgroundPath[0] != '\0');
        assetsEvent.hasStageMusic = (msg.stageMusicPath[0] != '\0');
        assetsEvent.hasBossMusic = (msg.bossMusicPath[0] != '\0');
        
        eventBus.emit(assetsEvent);
    };
    
    // Room/Lobby callbacks - will be set up after lobbyWidget is created
    // (see below after lobbyWidget initialization)
    networkClient.setCallbacks(netCallbacks);

    // ==================== Settings Panel Setup ====================
    
    // Create settings widget with initial configuration
    rtype::ui::SettingsConfig initialConfig;
    initialConfig.musicEnabled = true;
    initialConfig.musicVolume = 75.0f;
    initialConfig.effectsVolume = 75.0f;  // Initial effects volume
    initialConfig.settingsManager = &settingsManager; // Pass the settings manager for key bindings

    // ==================== Button Click Sound Setup ====================
    // Set a default click sound for all buttons
    rtype::ui::ButtonWidget::setDefaultClickSound(rtype::resolveAssetPath("assets/sound/mixkit-modern-technology-select-3124.wav"));
    // Set initial volume to match effects volume settings (convert percentage to 0.0-1.0 range)
    rtype::ui::ButtonWidget::setSoundVolume(initialConfig.effectsVolume / 100.0f);
    
    std::cout << "Button click sound system initialized!" << std::endl;
    std::cout << "- Default click sound: mixkit-modern-technology-select-3124.wav" << std::endl;
    std::cout << "- All buttons will now play sound on click" << std::endl;
    std::cout << "- Use button->setClickSound(\"path\") to customize individual button sounds" << std::endl;
    std::cout << "- Use button->setClickSoundEnabled(false) to disable sound for specific buttons" << std::endl;

    // ==================== Music System Setup ====================
    rtype::ecs::MusicSystem musicSystem(eventBus);
    g_musicSystem = &musicSystem;
    musicSystem.setMasterVolume(initialConfig.musicVolume / 100.0f);
    
    // Play default music for menu
    musicSystem.playTrack(rtype::resolveAssetPath("assets/sound/music/Sketchbook 2024-10-13.ogg"), 1.0f, true);
    std::cout << "MusicSystem initialized with default track" << std::endl;
    
    auto settingsWidget = std::make_shared<rtype::ui::SettingsWidget>(initialConfig);
    settingsWidget->setPosition(SCREEN_WIDTH / 2.0f - 400, SCREEN_HEIGHT / 2.0f - 275); // Match multiplayer positioning
    
    // Set up callbacks for settings events
    rtype::ui::SettingsCallbacks callbacks;
    callbacks.onMusicToggle = [](bool enabled) {
        std::cout << "Music " << (enabled ? "enabled" : "disabled") << "!" << std::endl;
        setMusicEnabled(enabled);
    };
    callbacks.onMusicVolumeChange = [](float volume) {
        std::cout << "Music volume changed to: " << static_cast<int>(volume) << "%" << std::endl;
        setMusicVolume(volume / 100.0f);  // Convert percentage to 0.0-1.0 range
    };
    callbacks.onEffectsVolumeChange = [](float volume) {
        std::cout << "Effects volume changed to: " << static_cast<int>(volume) << "%" << std::endl;
        // Update button click sound volume to match effects volume settings
        rtype::ui::ButtonWidget::setSoundVolume(volume / 100.0f);  // Convert percentage to 0.0-1.0 range
    };
    callbacks.onKeyBindingChange = [&settingsManager](const std::string& action, rtype::ecs::events::KeyCode newKey) {
        std::cout << "Key binding changed for " << action << " to " 
                  << rtype::ecs::events::InputUtils::keyCodeToString(newKey) << std::endl;
        // Save settings when key bindings change
        if (!settingsManager.save()) {
            std::cout << "Warning: Failed to save settings to file" << std::endl;
        } else {
            std::cout << "Settings saved successfully" << std::endl;
        }
    };
    // Create settings widget first without close callback
    uiManager.addWidget(settingsWidget);
    settingsWidget->initialize(); // Initialize after adding to UI manager

    // ==================== Main Menu Widget Setup ====================
    
    // Create main menu widget
    rtype::ui::MainMenuConfig menuConfig;
    menuConfig.title = "R-TYPE";
    menuConfig.subtitle = "The classic side-scrolling shooter";

    auto mainMenuWidget = std::make_shared<rtype::ui::MainMenuWidget>(menuConfig);
    mainMenuWidget->setPosition(SCREEN_WIDTH / 2.0f - 250, 60.0f);

    // Set up callbacks for main menu events
    rtype::ui::MainMenuCallbacks menuCallbacks;
    menuCallbacks.onSoloPlay = [&gameState, &mainMenuWidget, &localServer, &networkClient, &isLocalGame, &scoreManager, &profileManager]() {
        std::cout << "[Main] Starting solo game..." << std::endl;
        
        // Start local server (Quake-style: solo games run on a local server)
        constexpr uint16_t LOCAL_PORT = 4243;
        if (!localServer.start(true, LOCAL_PORT)) {  // allowSinglePlayer = true
            std::cerr << "[Main] Failed to start local server!" << std::endl;
            return;
        }
        
        // Wait for server to be ready
        if (!localServer.waitUntilReady(3000)) {
            std::cerr << "[Main] Local server failed to become ready!" << std::endl;
            localServer.stop();
            return;
        }
        
        std::cout << "[Main] Local server ready on port " << localServer.getPort() << std::endl;
        
        // Connect to local server
        if (!networkClient.connect("127.0.0.1", localServer.getPort())) {
            std::cerr << "[Main] Failed to connect to local server!" << std::endl;
            localServer.stop();
            return;
        }
        
        // Send profile to server
        const auto& profile = profileManager.getProfile();
        networkClient.sendPlayerProfile(profile.name, profile.avatarId, profile.colorScheme);
        
        // Send player ready (starts game immediately in single player)
        networkClient.sendPlayerReady();
        
        isLocalGame = true;
        scoreManager.resetSession();
        
        gameState = GameState::PLAYING;
        mainMenuWidget->hide();
        
        std::cout << "[Main] Solo game started!" << std::endl;
    };

    menuCallbacks.onMultiplayer = [&gameState, &showingMultiplayer]() {
        gameState = GameState::MULTIPLAYER;
        showingMultiplayer = true;
        std::cout << "Opening multiplayer menu..." << std::endl;
    };

    menuCallbacks.onSettings = [&gameState, &previousState, &settingsWidget, &mainMenuWidget]() {
        previousState = gameState; // Remember where we came from
        gameState = GameState::SETTINGS;
        settingsWidget->show();
        mainMenuWidget->hide();
    };

    menuCallbacks.onExit = [&shouldExit, &localServer, &networkClient, &isLocalGame, &scoreManager, &profileManager]() {
        // Save session score as high score if applicable
        if (isLocalGame) {
            const auto& profile = profileManager.getProfile();
            scoreManager.finishSession(profile.name);
        }
        
        // Clean up local server if running
        if (localServer.isRunning()) {
            networkClient.disconnect();
            localServer.stop();
        }
        shouldExit = true;
    };

    // NOTE: sendPlayerReady is now called when connecting via the multiplayer menu
    // We no longer connect at startup, so this is removed

    mainMenuWidget->setCallbacks(menuCallbacks);
    uiManager.addWidget(mainMenuWidget);
    mainMenuWidget->initialize(); // Initialize after adding to UI manager

    // Now set the settings callbacks (after mainMenuWidget is created)
    callbacks.onClose = [&gameState, mainMenuWidget]() {
        gameState = GameState::MENU;
        mainMenuWidget->show(); // Show main menu again
        std::cout << "Settings panel closed." << std::endl;
    };
    settingsWidget->setCallbacks(callbacks);

    // ==================== Multiplayer Widget Setup ====================
    
    // Create multiplayer widget
    // Use command-line arguments as defaults (if provided), otherwise use hardcoded defaults
    rtype::ui::MultiplayerConfig multiplayerConfig;
    multiplayerConfig.title = "CONNECT TO SERVER";
    multiplayerConfig.defaultIP = serverIp;  // Use command-line argument or default (127.0.0.1)
    multiplayerConfig.defaultPort = serverPort;  // Use command-line argument or default (4242)

    auto multiplayerWidget = std::make_shared<rtype::ui::MultiplayerWidget>(multiplayerConfig);
    multiplayerWidget->setPosition(SCREEN_WIDTH / 2.0f - 400, SCREEN_HEIGHT / 2.0f - 300);
    multiplayerWidget->setVisible(false); // Start hidden

    // ==================== Lobby Widget Setup ====================
    
    // Create lobby widget
    rtype::ui::LobbyConfig lobbyConfig;
    lobbyConfig.title = "LOBBY";
    lobbyConfig.roomName = "Room #1";

    auto lobbyWidget = std::make_shared<rtype::ui::LobbyWidget>(lobbyConfig);
    lobbyWidget->setPosition(SCREEN_WIDTH / 2.0f - 400, SCREEN_HEIGHT / 2.0f - 300);
    lobbyWidget->setVisible(false); // Start hidden

    // Set up callbacks for multiplayer events
    rtype::ui::MultiplayerCallbacks multiplayerCallbacks;
    multiplayerCallbacks.onBack = [&gameState, &showingMultiplayer, &mainMenuWidget, &multiplayerWidget]() {
        gameState = GameState::MENU;
        showingMultiplayer = false;
        multiplayerWidget->hide();
        mainMenuWidget->show();
        std::cout << "Returning to main menu from multiplayer..." << std::endl;
    };

    // Pending room name to join after welcome is received
    std::string pendingRoomToJoin;

    multiplayerCallbacks.onJoinServer = [&gameState, &showingMultiplayer, &showingLobby, &multiplayerWidget, &lobbyWidget, &networkClient, &pendingRoomToJoin](const std::string& ip, int port) {
        std::cout << "[Main] Attempting to connect to " << ip << ":" << port << std::endl;
        
        // Disconnect if already connected to a different server
        if (networkClient.isConnected()) {
            networkClient.disconnect();
        }
        
        // Store the room to join after we receive welcome
        pendingRoomToJoin = "DefaultRoom";
        lobbyWidget->setRoomName(pendingRoomToJoin);
        
        // Connect to the specified server
        if (networkClient.connect(ip, static_cast<uint16_t>(port))) {
            std::cout << "[Main] Connected to server " << ip << ":" << port << std::endl;
            
            // Go to lobby state - room join will happen in onWelcome callback
            gameState = GameState::LOBBY;
            showingMultiplayer = false;
            showingLobby = true;
            multiplayerWidget->hide();
            lobbyWidget->show();
            std::cout << "[Main] Waiting for welcome before joining room..." << std::endl;
        } else {
            std::cerr << "[Main] Failed to connect to " << ip << ":" << port << std::endl;
            pendingRoomToJoin.clear();
        }
    };

    multiplayerCallbacks.onCreateRoom = [&gameState, &showingMultiplayer, &showingLobby, &multiplayerWidget, &lobbyWidget, &networkClient](const rtype::ui::RoomSettings& settings) {
        // First, try to create the room on the server
        if (networkClient.isConnected()) {
            networkClient.createRoom(settings.name, 4);
        }
        
        gameState = GameState::LOBBY;
        showingMultiplayer = false;
        showingLobby = true;
        multiplayerWidget->hide();
        
        // Apply room settings to lobby
        lobbyWidget->setRoomName(settings.name);
        lobbyWidget->setBackgroundColor(settings.backgroundColor);
        // TODO: Apply password settings to lobby
        
        lobbyWidget->show();
        std::cout << "Creating room '" << settings.name << "' and entering lobby..." << std::endl;
    };

    // Network connectivity check
    multiplayerCallbacks.checkNetworkConnection = [&networkClient]() -> bool {
        return networkClient.isConnected();
    };

    multiplayerCallbacks.onNetworkError = [](const std::string& errorMessage) {
        std::cerr << "Network Error: " << errorMessage << std::endl;
    };

    // Room list fetcher - uses NetworkClient to communicate with server
    multiplayerCallbacks.onGetRoomList = [&networkClient]() -> std::vector<rtype::ui::RoomInfo> {
        std::vector<rtype::ui::RoomInfo> rooms;
        
        // Only return rooms if connected to server
        if (!networkClient.isConnected()) {
            return rooms; // Empty list if not connected
        }
        
        // Request room list from server (async - will receive via callback)
        networkClient.requestRoomList();
        
        // Return empty for now - the UI will be updated via the onRoomList callback
        // TODO: Cache the last received room list and return it here
        return rooms;
    };

    multiplayerWidget->setCallbacks(multiplayerCallbacks);
    uiManager.addWidget(multiplayerWidget);
    multiplayerWidget->initialize();

    // Set up callbacks for lobby events
    rtype::ui::LobbyCallbacks lobbyCallbacks;
    lobbyCallbacks.onBack = [&gameState, &showingLobby, &showingMultiplayer, &multiplayerWidget, &lobbyWidget]() {
        gameState = GameState::MULTIPLAYER;
        showingLobby = false;
        showingMultiplayer = true;
        lobbyWidget->hide();
        multiplayerWidget->show();
        std::cout << "Returning to multiplayer menu from lobby..." << std::endl;
    };

    lobbyCallbacks.onStartGame = [&gameState, &showingLobby, &lobbyWidget, &networkClient]() {
        // Only host can start the game
        if (!networkClient.isHost()) {
            std::cout << "Cannot start game - you are not the host!" << std::endl;
            return;
        }
        
        // Send host start game message to server
        networkClient.hostStartGame(0);  // Level 0 for now
        
        gameState = GameState::PLAYING;
        showingLobby = false;
        lobbyWidget->hide();
        std::cout << "Host starting game from lobby..." << std::endl;
    };

    lobbyWidget->setCallbacks(lobbyCallbacks);
    uiManager.addWidget(lobbyWidget);
    lobbyWidget->initialize(); // Initialize after adding to UI manager

    // ==================== Room/Lobby Network Callbacks ====================
    // Now that lobbyWidget is created, set up room callbacks
    netCallbacks.onRoomJoined = [&lobbyWidget, &networkClient](const rtype::network::RoomJoinedMessage& msg) {
        std::cout << "[Main] Joined room: " << msg.roomName << " (slot " << (int)msg.yourSlot << ")" << std::endl;
        lobbyWidget->setRoomName(msg.roomName);
        
        // Check if we are the host
        bool isHost = (msg.hostClientId == networkClient.getClientId());
        lobbyWidget->setIsHost(isHost);
    };
    
    netCallbacks.onRoomInfo = [&lobbyWidget, &networkClient, &gameState, &showingLobby](const rtype::network::RoomInfoMessage& msg) {
        std::cout << "[Main] Room info update: " << msg.roomName << " (" << (int)msg.playerCount << "/" << (int)msg.maxPlayers << " players)" << std::endl;
        
        // Convert RoomPlayerInfo array to LobbyPlayerInfo vector
        std::vector<rtype::ui::LobbyPlayerInfo> players;
        for (int i = 0; i < msg.playerCount && i < 4; ++i) {
            const auto& rp = msg.players[i];
            rtype::ui::LobbyPlayerInfo lp;
            lp.name = rp.playerName;
            lp.slot = rp.slot;
            lp.isHost = rp.isHost;
            lp.isReady = rp.isReady;
            lp.isLocal = (rp.clientId == networkClient.getClientId());
            players.push_back(lp);
        }
        
        lobbyWidget->updatePlayersList(players);
        
        // Update host status
        bool isHost = (msg.hostClientId == networkClient.getClientId());
        lobbyWidget->setIsHost(isHost);
        
        // If room state changed to PLAYING, start the game locally
        if (msg.state == rtype::network::RoomState::PLAYING) {
            std::cout << "[Main] Game starting!" << std::endl;
            gameState = GameState::PLAYING;
            showingLobby = false;
            lobbyWidget->hide();
        }
    };
    
    netCallbacks.onHostChanged = [&lobbyWidget, &networkClient](const rtype::network::HostChangedMessage& msg) {
        std::cout << "[Main] Host changed! Reason: " << msg.reason << std::endl;
        bool isHost = (msg.newHostClientId == networkClient.getClientId());
        lobbyWidget->setIsHost(isHost);
        if (isHost) {
            std::cout << "[Main] You are now the host!" << std::endl;
        }
    };
    
    netCallbacks.onRoomCreated = [&lobbyWidget, &networkClient](const rtype::network::RoomCreatedMessage& msg) {
        std::cout << "[Main] Room created: " << msg.roomName << std::endl;
        lobbyWidget->setRoomName(msg.roomName);
        lobbyWidget->setIsHost(true);  // Creator is always host
    };
    
    netCallbacks.onRoomError = [](const rtype::network::RoomErrorMessage& msg) {
        std::cerr << "[Main] Room error: " << msg.message << std::endl;
    };
    
    // Welcome callback - join pending room after receiving client ID
    netCallbacks.onWelcome = [&networkClient, &pendingRoomToJoin](uint32_t clientId) {
        std::cout << "[Main] Welcome received! Client ID: " << clientId << std::endl;
        
        // Send player ready message now that we have a valid client ID
        networkClient.sendPlayerReady();
        
        // Join the pending room if one was set
        if (!pendingRoomToJoin.empty()) {
            std::cout << "[Main] Joining room: " << pendingRoomToJoin << std::endl;
            networkClient.joinRoom(pendingRoomToJoin);
            pendingRoomToJoin.clear();
        }
    };
    
    // Re-set callbacks with room handlers included
    networkClient.setCallbacks(netCallbacks);

    // Update main menu's onMultiplayer callback to show multiplayer widget
    menuCallbacks.onMultiplayer = [&gameState, &showingMultiplayer, &mainMenuWidget, &multiplayerWidget]() {
        gameState = GameState::MULTIPLAYER;
        showingMultiplayer = true;
        mainMenuWidget->hide();
        multiplayerWidget->show();
        std::cout << "Opening multiplayer menu..." << std::endl;
    };
    mainMenuWidget->setCallbacks(menuCallbacks); // Update callbacks

    // Update settings close callback to handle all states
    callbacks.onClose = [&gameState, &previousState, &mainMenuWidget, &multiplayerWidget, &lobbyWidget]() {
        std::cout << "Closing settings panel..." << std::endl;

        // Return to the state we came from
        gameState = previousState;
        
        // Show appropriate widgets based on where we're going back to
        if (gameState == GameState::MENU) {
            mainMenuWidget->show();
        } else if (gameState == GameState::MULTIPLAYER) {
            multiplayerWidget->show();
        } else if (gameState == GameState::LOBBY) {
            lobbyWidget->show();
        } else if (gameState == GameState::PAUSED) {
            // When closing settings from pause menu, return to paused state
            // No widgets need to be shown in pause state
            std::cout << "Returning to pause menu from settings..." << std::endl;
        }
        std::cout << "Settings panel closed." << std::endl;
    };
    settingsWidget->setCallbacks(callbacks);

    auto* inputSystem = systems.addSystem<InputSystem>(eventBus, 350.0f, &networkClient);
    auto* bulletSystem = systems.addSystem<BulletSystem>(eventBus);
    bulletSystem->setScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* patternSystem = systems.addSystem<PatternSystem>();
    patternSystem->setScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    systems.addSystem<TrajectorySystem>();
    systems.addSystem<SpinSystem>();
    systems.addSystem<MovementSystem>();  // Add MovementSystem
    auto* showoffSystem = systems.addSystem<ShowoffSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* stressTestSystem = systems.addSystem<StressTestSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* renderSystem = systems.addSystem<RenderSystem>(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // ==================== Enemy Systems Setup ====================
    auto* enemySpawnerSystem = systems.addSystem<EnemySpawnerSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* enemyAISystem = systems.addSystem<EnemyAISystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    (void)enemySpawnerSystem;  // Event-driven, updates via SystemManager
    (void)enemyAISystem;       // Event-driven, updates via SystemManager
    
    // ==================== Background System Setup ====================
    rtype::ecs::BackgroundSystem backgroundSystem(registry, eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    backgroundSystem.createDefaultBackground();  // Start with default procedural background
    
    // Both debug systems available - can toggle between them
    auto* windowedDebugSystem = systems.addSystem<WindowedDebugSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* classicDebugSystem = systems.addSystem<DebugSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Set up both debug systems
    windowedDebugSystem->setTextures(renderSystem->getTextures());
    windowedDebugSystem->init();
    classicDebugSystem->setTextures(renderSystem->getTextures());
    classicDebugSystem->init();
    
    // Link systems so they can switch between each other
    windowedDebugSystem->setClassicDebugSystem(classicDebugSystem);
    classicDebugSystem->setWindowedDebugSystem(windowedDebugSystem);
    
    renderSystem->setOverlayCallback([showoffSystem, stressTestSystem, windowedDebugSystem, classicDebugSystem, &gameState, &uiManager]() {
        // Render UI in menu, multiplayer, lobby states, and paused when showing settings
        if (gameState == GameState::MENU || gameState == GameState::MULTIPLAYER || gameState == GameState::LOBBY || 
            gameState == GameState::PAUSED || gameState == GameState::SETTINGS) {
            uiManager.render();
        }

        // Game state overlays - draw active debug system
        windowedDebugSystem->draw();
        classicDebugSystem->draw();
        
        // Draw showoff mode indicator
        if (showoffSystem->isActive()) {
            DrawRectangle(0, 0, 400, 60, Fade(BLACK, 0.7f));
            DrawText("SHOWOFF MODE", 10, 10, 20, YELLOW);
            DrawText(showoffSystem->getCurrentPatternName().c_str(), 10, 35, 16, WHITE);
            
            // Progress bar
            float progress = showoffSystem->getPhaseProgress();
            DrawRectangle(200, 12, 180, 16, DARKGRAY);
            DrawRectangle(200, 12, static_cast<int>(180 * progress), 16, YELLOW);
            
            // Phase counter
            char phaseText[32];
            snprintf(phaseText, sizeof(phaseText), "%d/%d", 
                     showoffSystem->getCurrentPhase() + 1, 
                     showoffSystem->getTotalPhases());
            DrawText(phaseText, 200, 35, 16, GRAY);
        }
        
        // Draw stress test mode indicator
        if (stressTestSystem->isActive()) {
            DrawRectangle(0, 0, 450, 100, Fade(RED, 0.85f));
            DrawText("STRESS TEST MODE", 10, 8, 20, WHITE);
            
            // Phase progress bar
            char phaseLabel[64];
            snprintf(phaseLabel, sizeof(phaseLabel), "Phase %d/%d (Intensity %d)", 
                     stressTestSystem->getCurrentPhase(),
                     stressTestSystem->getTotalPhases(),
                     stressTestSystem->getIntensity());
            DrawText(phaseLabel, 10, 32, 14, YELLOW);
            
            float progress = stressTestSystem->getPhaseProgress();
            DrawRectangle(10, 50, 430, 12, DARKGRAY);
            DrawRectangle(10, 50, static_cast<int>(430 * progress), 12, ORANGE);
            
            char statsText[128];
            snprintf(statsText, sizeof(statsText), "Entities: %d | Waves: %d | FPS: %d | Time: %.0fs/100s",
                     stressTestSystem->getBulletCount(),
                     stressTestSystem->getWaveCount(),
                     GetFPS(),
                     stressTestSystem->getTotalTime());
            DrawText(statsText, 10, 68, 14, LIGHTGRAY);
            
            // Phase time remaining
            float timeLeft = 20.0f - stressTestSystem->getPhaseTime();
            snprintf(statsText, sizeof(statsText), "Next phase in: %.1fs", timeLeft > 0 ? timeLeft : 0);
            DrawText(statsText, 10, 84, 12, GRAY);
        }
        
        // Draw completion message
        if (stressTestSystem->isComplete() && !stressTestSystem->getReportFilename().empty()) {
            DrawRectangle(0, 0, 500, 60, Fade(GREEN, 0.9f));
            DrawText("STRESS TEST COMPLETE!", 10, 10, 20, WHITE);
            char reportMsg[256];
            snprintf(reportMsg, sizeof(reportMsg), "Report saved: %s", 
                     stressTestSystem->getReportFilename().c_str());
            DrawText(reportMsg, 10, 35, 14, YELLOW);
        }
    });

    renderSystem->setUIManager(&uiManager);
    renderSystem->setGameStatePtr(&gameState);

    // ==================== Create Game Entities ====================
    // Background is now managed by BackgroundSystem (created above)
    // Legacy: Entity background = createBackground(registry, SCREEN_WIDTH, SCREEN_HEIGHT);

    // NOTE: Player entity is NO LONGER created locally
    // The server creates and synchronizes player entities when connected
    // This ensures the game can only be played with a server connection

    // ==================== Game Loop (Fixed Timestep) ====================
    float accumulator = 0.0f;

    while (!WindowShouldClose() && !shouldExit) {
        float frameTime = GetFrameTime();
        accumulator += frameTime;

        inputManager.pollInput();
        
        // Update music system (handles playback)
        musicSystem.update(frameTime);
        
        // Update background system (handles starfield animation)
        backgroundSystem.update(frameTime);

        // Update UI in menu, multiplayer, lobby states, and paused when showing settings
        if (gameState == GameState::MENU || gameState == GameState::MULTIPLAYER || gameState == GameState::LOBBY || 
            gameState == GameState::PAUSED || gameState == GameState::SETTINGS) {
            uiManager.update(frameTime);
        }
        
        // Poll for key presses and send to UI system
        int key = GetKeyPressed();
        while (key != 0) {
            // Convert raylib key to our KeyCode
            rtype::ecs::events::KeyCode keyCode = rtype::ecs::events::InputUtils::raylibToKeyCode(key);
            
            // Create and dispatch key press event to UI system
            rtype::ecs::events::KeyPressedEvent keyEvent{keyCode};
            eventBus.emit(keyEvent);
            
            // Get next key in queue
            key = GetKeyPressed();
        }
        
        // Handle pause toggle with ESC key - but not when settings panel is capturing input
        if (IsKeyPressed(KEY_ESCAPE)) {
            // Don't handle ESC for pause if settings panel is visible and waiting for key input
            bool settingsWaitingForInput = true ? (gameState == GameState::SETTINGS) && settingsWidget->isVisible() && settingsWidget->isWaitingForKeyInput() : false;

            if (!settingsWaitingForInput && gameState == GameState::PLAYING) {
                std::cout << "Game paused..." << std::endl;
                gameState = GameState::PAUSED;
            } else if (!settingsWaitingForInput && gameState == GameState::PAUSED) {
                std::cout << "Game resumed..." << std::endl;
                gameState = GameState::PLAYING;
            }
        }
        
        // Handle pause menu key shortcuts
        if (gameState == GameState::PAUSED) {
            if (IsKeyPressed(KEY_M)) {
                std::cout << "Returning to menu..." << std::endl;
                
                // Clean up local server if running
                if (isLocalGame && localServer.isRunning()) {
                    // Save score before closing
                    scoreManager.finishSession(profileManager.getProfile().name);
                    
                    networkClient.disconnect();
                    localServer.stop();
                    isLocalGame = false;
                    std::cout << "[Main] Local server stopped" << std::endl;
                }
                
                gameState = GameState::MENU;
                mainMenuWidget->show(); // Fix: Show main menu when returning from pause
            }
            if (IsKeyPressed(KEY_Q)) {
                std::cout << "Quitting game..." << std::endl;
                
                // Clean up local server if running
                if (isLocalGame && localServer.isRunning()) {
                    scoreManager.finishSession(profileManager.getProfile().name);
                    networkClient.disconnect();
                    localServer.stop();
                }
                
                shouldExit = true;
            }
            if (IsKeyPressed(KEY_S)) {
                std::cout << "Opening settings..." << std::endl;
                previousState = gameState; // Remember we came from pause
                gameState = GameState::SETTINGS;
                settingsWidget->show(); // Fix: Show settings widget when opening from pause menu
            }
        }

        // Update network (process received messages)
        networkClient.update();
        
        // Update respawn animations (slide-in from left)
        networkClient.updateAnimations(frameTime);

        while (accumulator >= FIXED_TIMESTEP) {
            // Update game systems based on game state
            if (gameState == GameState::PLAYING) {
                inputSystem->update(FIXED_TIMESTEP);
                windowedDebugSystem->update(FIXED_TIMESTEP);
                classicDebugSystem->update(FIXED_TIMESTEP);
                showoffSystem->update(FIXED_TIMESTEP);
                stressTestSystem->update(FIXED_TIMESTEP);
                patternSystem->update(FIXED_TIMESTEP);
                systems.getSystem<TrajectorySystem>()->update(FIXED_TIMESTEP);
                systems.getSystem<SpinSystem>()->update(FIXED_TIMESTEP);
                systems.getSystem<MovementSystem>()->update(FIXED_TIMESTEP);
                bulletSystem->update(FIXED_TIMESTEP);
                clampPlayerToScreen(registry);
            }
            accumulator -= FIXED_TIMESTEP;
        }

        // Update both debug systems with showoff/stress test state
        windowedDebugSystem->updateShowoffState(
            showoffSystem->isActive(),
            showoffSystem->getCurrentPatternName(),
            showoffSystem->getCurrentPhase(),
            showoffSystem->getTotalPhases(),
            showoffSystem->getPhaseProgress()
        );
        windowedDebugSystem->updateStressTestState(
            stressTestSystem->isActive(),
            stressTestSystem->isComplete(),
            stressTestSystem->getIntensity(),
            stressTestSystem->getPhaseProgress(),
            stressTestSystem->getReportFilename()
        );
        classicDebugSystem->updateShowoffState(
            showoffSystem->isActive(),
            showoffSystem->getCurrentPatternName(),
            showoffSystem->getCurrentPhase(),
            showoffSystem->getTotalPhases(),
            showoffSystem->getPhaseProgress()
        );
        classicDebugSystem->updateStressTestState(
            stressTestSystem->isActive(),
            stressTestSystem->isComplete(),
            stressTestSystem->getIntensity(),
            stressTestSystem->getPhaseProgress(),
            stressTestSystem->getReportFilename()
        );
        renderSystem->update(frameTime);
    }

    // ==================== Cleanup ====================
    cleanupMusic();      // Clean up music resources
    CloseAudioDevice();  // Clean up audio device
    CloseWindow();

    return 0;
}
