/*
** R-Type Client
** Main game client with ECS architecture
** Uses fixed timestep for game logic, uncapped rendering
** Event-driven input via EventBus
*/

#include <raylib.h>

#include <iostream>
#include <string>
#include "../engine/ecs/ECS.hpp"
#include "../engine/ecs/core/EventBus.hpp"
#include "../engine/ecs/events/Events.hpp"
#include "../engine/ecs/core/SystemManager.hpp"
#include "../game/Components.hpp"
#include "../game/Systems.hpp"
#include "../game/systems/DebugSystem.hpp"
#include "../engine/ui/UIManager.hpp"
#include "../engine/ui/widgets/ButtonWidget.hpp"
#include "../engine/ui/widgets/TextWidget.hpp"
#include "../engine/ui/widgets/PanelWidget.hpp"
#include "../engine/ui/widgets/SettingsWidget.hpp"
#include "../engine/ui/widgets/MainMenuWidget.hpp"
#include "../engine/graphics/RenderUtils.hpp"
#include "NetworkClient.hpp"

using namespace rtype::ecs;
using rtype::ecs::BulletType;
using rtype::ecs::BulletColor;
using rtype::ui::UIColor;

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;  // 60 Hz game logic

// Music system variables
static Music backgroundMusic = {};
static bool musicLoaded = false;
static bool musicEnabled = true;
static float musicVolume = 0.75f;
static std::string currentMusicPath = "assets/sound/music/Sketchbook 2024-10-13.ogg";

/**
 * @brief Initialize and load background music
 */
void initializeMusic() {
    if (!IsAudioDeviceReady()) {
        std::cout << "Audio device not ready, cannot load music" << std::endl;
        return;
    }
    
    backgroundMusic = LoadMusicStream(currentMusicPath.c_str());
    if (backgroundMusic.frameCount > 0) {
        musicLoaded = true;
        SetMusicVolume(backgroundMusic, musicVolume);
        std::cout << "Background music loaded: " << currentMusicPath << std::endl;
        
        if (musicEnabled) {
            PlayMusicStream(backgroundMusic);
            std::cout << "Background music started" << std::endl;
        }
    } else {
        std::cout << "Failed to load background music: " << currentMusicPath << std::endl;
    }
}

/**
 * @brief Update music playback (call every frame)
 */
void updateMusic() {
    if (musicLoaded && musicEnabled) {
        UpdateMusicStream(backgroundMusic);
        
        // Loop the music when it finishes
        if (!IsMusicStreamPlaying(backgroundMusic)) {
            PlayMusicStream(backgroundMusic);
        }
    }
}

/**
 * @brief Set music enabled/disabled state
 */
void setMusicEnabled(bool enabled) {
    musicEnabled = enabled;
    if (musicLoaded) {
        if (enabled) {
            PlayMusicStream(backgroundMusic);
            std::cout << "Background music resumed" << std::endl;
        } else {
            PauseMusicStream(backgroundMusic);
            std::cout << "Background music paused" << std::endl;
        }
    }
}

/**
 * @brief Set music volume (0.0 to 1.0)
 */
void setMusicVolume(float volume) {
    musicVolume = volume;
    if (musicLoaded) {
        SetMusicVolume(backgroundMusic, musicVolume);
    }
}

/**
 * @brief Clean up music resources
 */
void cleanupMusic() {
    if (musicLoaded) {
        UnloadMusicStream(backgroundMusic);
        musicLoaded = false;
        std::cout << "Background music unloaded" << std::endl;
    }
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
 * @brief Create the scrolling background entity
 */
Entity createBackground(Registry& registry, int screenWidth, int screenHeight) {
    Entity bg = registry.createEntity();
    
    registry.addComponent(bg, TransformComponent(0, 0));
    
    BackgroundComponent background(screenWidth, screenHeight, 200, 100.0f);
    background.layer = 0;  // Render behind everything
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

int main() {
    // ==================== Raylib Initialization ====================
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "R-Type");
    ClearWindowState(FLAG_WINDOW_RESIZABLE);  // Fixed window size to prevent FPS drops on resize
    
    // ==================== Audio Initialization ====================
    InitAudioDevice();  // Explicitly initialize audio device
    std::cout << "Audio device initialized. Ready: " << (IsAudioDeviceReady() ? "YES" : "NO") << std::endl;

    // ==================== Event Bus ====================
    rtype::ecs::EventBus eventBus;

    // ==================== Input Manager ====================
    rtype::ecs::events::InputManager inputManager(eventBus);

    // ==================== UI Manager ====================
    rtype::ui::UIManager uiManager(eventBus);

    // UIManager automatically subscribes to events via its constructor // Optional for testing focus/text

    // ==================== Game State Management ====================
    GameState gameState = GameState::MENU;
    
    // ==================== Settings Panel ====================
    bool showingSettingsPanel = false;
    
    // ==================== ECS Setup ====================
    Registry registry;
    SystemManager systems(&registry);
    bool shouldExit = false;

    // ==================== Network Setup ====================
    rtype::client::NetworkClient networkClient(registry);

    // Connect to server (localhost for now)
    if (!networkClient.connect("127.0.0.1", 4242)) {
        std::cerr << "[Main] Failed to connect to server, continuing in offline mode" << std::endl;
    }

    // ==================== Settings Panel Setup ====================
    
    // Create settings widget with initial configuration
    rtype::ui::SettingsConfig initialConfig;
    initialConfig.musicEnabled = true;
    initialConfig.musicVolume = 75.0f;
    initialConfig.effectsVolume = 75.0f;  // Initial effects volume

    // ==================== Button Click Sound Setup ====================
    // Set a default click sound for all buttons
    rtype::ui::ButtonWidget::setDefaultClickSound("assets/sound/mixkit-modern-technology-select-3124.wav");
    // Set initial volume to match effects volume settings (convert percentage to 0.0-1.0 range)
    rtype::ui::ButtonWidget::setSoundVolume(initialConfig.effectsVolume / 100.0f);
    
    std::cout << "Button click sound system initialized!" << std::endl;
    std::cout << "- Default click sound: mixkit-modern-technology-select-3124.wav" << std::endl;
    std::cout << "- All buttons will now play sound on click" << std::endl;
    std::cout << "- Use button->setClickSound(\"path\") to customize individual button sounds" << std::endl;
    std::cout << "- Use button->setClickSoundEnabled(false) to disable sound for specific buttons" << std::endl;

    // ==================== Background Music Setup ====================
    // Initialize music system with initial settings
    ::musicEnabled = initialConfig.musicEnabled;
    ::musicVolume = initialConfig.musicVolume / 100.0f;
    initializeMusic();
    
    auto settingsWidget = std::make_shared<rtype::ui::SettingsWidget>(initialConfig);
    settingsWidget->setPosition(SCREEN_WIDTH / 2.0f - 300, SCREEN_HEIGHT / 2.0f - 250);
    
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
    menuCallbacks.onPlay = [&gameState]() {
        gameState = GameState::PLAYING;
    };

    menuCallbacks.onSettings = [&showingSettingsPanel, &settingsWidget, &mainMenuWidget]() {
        showingSettingsPanel = true;
        settingsWidget->show();
        mainMenuWidget->hide();
    };

    menuCallbacks.onExit = [&shouldExit]() {
        shouldExit = true;
    };

    // Send PLAYER_READY to server
    if (networkClient.isConnected()) {
        networkClient.sendPlayerReady();
    }

    mainMenuWidget->setCallbacks(menuCallbacks);
    uiManager.addWidget(mainMenuWidget);
    mainMenuWidget->initialize(); // Initialize after adding to UI manager

    // Now set the settings callbacks (after mainMenuWidget is created)
    callbacks.onClose = [&showingSettingsPanel, mainMenuWidget]() {
        showingSettingsPanel = false;
        mainMenuWidget->show(); // Show main menu again
        std::cout << "Settings panel closed." << std::endl;
    };
    settingsWidget->setCallbacks(callbacks);

    auto* inputSystem = systems.addSystem<InputSystem>(eventBus, 350.0f, &networkClient);
    systems.addSystem<MovementSystem>();
    auto* bulletSystem = systems.addSystem<BulletSystem>(eventBus);
    bulletSystem->setScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* patternSystem = systems.addSystem<PatternSystem>();
    patternSystem->setScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    systems.addSystem<TrajectorySystem>();
    systems.addSystem<SpinSystem>();
    auto* showoffSystem = systems.addSystem<ShowoffSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* stressTestSystem = systems.addSystem<StressTestSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* renderSystem = systems.addSystem<RenderSystem>(SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* debugSystem = systems.addSystem<DebugSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);

    debugSystem->setTextures(renderSystem->getTextures());
    debugSystem->init();

    renderSystem->setOverlayCallback([showoffSystem, stressTestSystem, debugSystem, &gameState, &uiManager]() {
        // Render UI only in menu state (MainMenuWidget handles its own background)
        if (gameState == GameState::MENU) {
            uiManager.render();
        }
        
        // Game state overlays
        debugSystem->draw();
        debugSystem->draw();
        
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
    Entity background = createBackground(registry, SCREEN_WIDTH, SCREEN_HEIGHT);
    (void)background;

    // NOTE: Player is now created by the server and spawned via network
    // Entity player = createPlayer(registry, 1);
    // (void)player;

    // ==================== Game Loop (Fixed Timestep) ====================
    float accumulator = 0.0f;

    while (!WindowShouldClose() && !shouldExit) {
        float frameTime = GetFrameTime();
        accumulator += frameTime;

        inputManager.pollInput();
        
        // Update background music
        updateMusic();

        // Update UI only in menu state
        if (gameState == GameState::MENU) {
            uiManager.update(frameTime);
        }
        
        // Allow ESC to return to menu from game
        if (gameState == GameState::PLAYING && IsKeyPressed(KEY_ESCAPE)) {
            std::cout << "Returning to menu..." << std::endl;
            gameState = GameState::MENU;
        }

        // Update network (process received messages)
        networkClient.update();

        while (accumulator >= FIXED_TIMESTEP) {
            // Update game systems based on game state
            if (gameState == GameState::PLAYING) {
                inputSystem->update(FIXED_TIMESTEP);
                debugSystem->update(FIXED_TIMESTEP);
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

        debugSystem->updateShowoffState(
            showoffSystem->isActive(),
            showoffSystem->getCurrentPatternName(),
            showoffSystem->getCurrentPhase(),
            showoffSystem->getTotalPhases(),
            showoffSystem->getPhaseProgress()
        );
        debugSystem->updateStressTestState(
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
