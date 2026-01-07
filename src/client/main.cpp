/*
** R-Type Client
** Main game client with ECS architecture
** Uses fixed timestep for game logic, uncapped rendering
** Event-driven input via EventBus
*/

#include <raylib.h>

#include <iostream>
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
#include "../engine/graphics/RenderUtils.hpp"
#include "NetworkClient.hpp"

using namespace rtype::ecs;
using rtype::ecs::BulletType;
using rtype::ecs::BulletColor;
using rtype::ui::UIColor;

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;  // 60 Hz game logic

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

    // ==================== Event Bus ====================
    rtype::ecs::EventBus eventBus;

    // ==================== Input Manager ====================
    rtype::ecs::events::InputManager inputManager(eventBus);

    // ==================== UI Manager ====================
    rtype::ui::UIManager uiManager(eventBus);

    // UIManager automatically subscribes to events via its constructor // Optional for testing focus/text

    // ==================== Game State Management ====================
    GameState gameState = GameState::MENU;

    // ==================== ECS Setup ====================
    Registry registry;
    SystemManager systems(&registry);
    bool shouldExit = false;

    // ==================== Network Setup ====================
    rtype::client::NetworkClient networkClient(registry);

    // Connect to server with command-line arguments
    if (!networkClient.connect(serverIp, serverPort)) {
        std::cerr << "[Main] Failed to connect to " << serverIp << ":" << serverPort
                  << ", continuing in offline mode" << std::endl;
    }

    // ==================== Menu Panel Setup ====================
    
    // -- Create main menu panel --
    auto menuPanel = std::make_shared<rtype::ui::PanelWidget>();
    menuPanel->setPosition(SCREEN_WIDTH / 2.0f - 250, 60.0f);
    menuPanel->setSize(500.0f, 500.0f);
    menuPanel->setBackgroundColor(UIColor(0, 0, 0, 180)); // Semi-transparent black background
    menuPanel->setBorderColor(UIColor(100, 100, 255, 200)); // Blue border with transparency
    menuPanel->setBorderWidth(2.0f);
    uiManager.addWidget(menuPanel);

    // Get content bounds for proper positioning within panel
    auto contentBounds = menuPanel->getContentBounds();

    // -- Create title glow layers for effect (back to front) --
    auto titleGlow2 = std::make_shared<rtype::ui::TextWidget>("R-TYPE", 64);
    titleGlow2->setPosition(contentBounds.width / 2.0f - 118, 44.0f); // Properly centered with glow offset
    titleGlow2->setSize(200.0f, 80.0f);
    titleGlow2->setBackgroundColor(UIColor::Transparent());
    titleGlow2->setTextColor(UIColor(255, 30, 30, 60)); // Very dim red glow
    menuPanel->addChild(titleGlow2);

    auto titleGlow1 = std::make_shared<rtype::ui::TextWidget>("R-TYPE", 64);
    titleGlow1->setPosition(contentBounds.width / 2.0f - 119, 46.0f); // Slightly offset for glow effect
    titleGlow1->setSize(200.0f, 80.0f);
    titleGlow1->setBackgroundColor(UIColor::Transparent());
    titleGlow1->setTextColor(UIColor(255, 50, 50, 100)); // Dim red glow
    menuPanel->addChild(titleGlow1);

    // -- Create glowing game title (main title on top) --
    auto gameTitle = std::make_shared<rtype::ui::TextWidget>("R-TYPE", 64);
    gameTitle->setPosition(contentBounds.width / 2.0f - 120, 48.0f); // Centered in panel
    gameTitle->setSize(200.0f, 80.0f);
    gameTitle->setBackgroundColor(UIColor::Transparent());
    gameTitle->setTextColor(UIColor(255, 100, 100, 255)); // Bright red for main text
    menuPanel->addChild(gameTitle);

    // -- Create subtitle --
    auto subtitle = std::make_shared<rtype::ui::TextWidget>("The classic side-scrolling shooter", 24);
    subtitle->setPosition(contentBounds.width / 2.0f - 200, 140.0f); // Centered below title
    subtitle->setSize(400.0f, 40.0f);
    subtitle->setBackgroundColor(UIColor::Transparent());
    subtitle->setTextColor(UIColor(200, 200, 255, 255)); // Light blue
    menuPanel->addChild(subtitle);

    // -- Create Play button with enhanced styling --
    auto playButton = std::make_shared<rtype::ui::ButtonWidget>("PLAY");
    playButton->setPosition(contentBounds.width / 2.0f - 120, 200.0f); // Centered in panel
    playButton->setSize(240.0f, 60.0f);
    playButton->setBackgroundColor(UIColor(0, 150, 0, 200)); // Semi-transparent green
    playButton->setBorderColor(UIColor(0, 255, 0, 255)); // Bright green border
    playButton->setBorderWidth(2.0f);
    playButton->setTextColor(UIColor::White());
    playButton->setOnClick([&gameState, &networkClient]() {
        std::cout << "Play button clicked! Starting game..." << std::endl;
        gameState = GameState::PLAYING;

        // Send PLAYER_READY to server
        if (networkClient.isConnected()) {
            networkClient.sendPlayerReady();
        }
    });
    menuPanel->addChild(playButton);

    // -- Create Settings button --
    auto settingsButton = std::make_shared<rtype::ui::ButtonWidget>("SETTINGS");
    settingsButton->setPosition(contentBounds.width / 2.0f - 120, 280.0f); // Centered in panel
    settingsButton->setSize(240.0f, 50.0f);
    settingsButton->setBackgroundColor(UIColor(100, 100, 100, 200)); // Semi-transparent gray
    settingsButton->setBorderColor(UIColor(150, 150, 150, 255)); // Light gray border
    settingsButton->setBorderWidth(2.0f);
    settingsButton->setTextColor(UIColor::White());
    settingsButton->setOnClick([]() {
        std::cout << "Settings button clicked!" << std::endl;
        // TODO: Implement settings menu
    });
    menuPanel->addChild(settingsButton);

    // -- Create Exit button with enhanced styling --
    auto exitButton = std::make_shared<rtype::ui::ButtonWidget>("EXIT");
    exitButton->setPosition(contentBounds.width / 2.0f - 120, 350.0f); // Centered in panel
    exitButton->setSize(240.0f, 50.0f);
    exitButton->setBackgroundColor(UIColor(150, 0, 0, 200)); // Semi-transparent red
    exitButton->setBorderColor(UIColor(255, 0, 0, 255)); // Bright red border
    exitButton->setBorderWidth(2.0f);
    exitButton->setTextColor(UIColor::White());
    exitButton->setOnClick([&shouldExit]() {
        std::cout << "Exit button clicked! Closing window..." << std::endl;
        shouldExit = true;
    });
    menuPanel->addChild(exitButton);

    // ==================== Menu Animation Variables ====================
    float glowTime = 0.0f;
    float titlePulse = 1.0f;

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
    
    renderSystem->setOverlayCallback([showoffSystem, stressTestSystem, debugSystem, &gameState, &uiManager, &glowTime]() {
        // Render UI and menu background only in menu state
        if (gameState == GameState::MENU) {
            // Draw animated star field background
            for (int i = 0; i < 100; i++) {
                int x = static_cast<int>(fmodf(i * 137.5f, static_cast<float>(SCREEN_WIDTH))); // Pseudo-random positions
                int y = static_cast<int>(fmodf(i * 273.7f, static_cast<float>(SCREEN_HEIGHT)));
                float twinkle = sinf(glowTime + i * 0.1f) * 0.5f + 0.5f;
                int alpha = static_cast<int>(50 + 100 * twinkle);
                DrawCircle(x, y, 1.0f, Color{255, 255, 255, static_cast<unsigned char>(alpha)});
            }
            
            // Draw subtle gradient overlay
            for (int y = 0; y < SCREEN_HEIGHT; y += 4) {
                float intensity = static_cast<float>(y) / SCREEN_HEIGHT;
                int alpha = static_cast<int>(20 * intensity);
                DrawRectangle(0, y, SCREEN_WIDTH, 4, Color{0, 0, 50, static_cast<unsigned char>(alpha)});
            }
            
            // Render UI elements
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

        // Update UI only in menu state
        if (gameState == GameState::MENU) {
            uiManager.update(frameTime);
            
            // Update glowing animation
            glowTime += frameTime * 2.0f; // Speed of glow animation
            titlePulse = 0.7f + 0.3f * (sinf(glowTime) + 1.0f) / 2.0f; // Pulse between 0.7 and 1.0
            
            // Update title glow colors dynamically
            int baseRed = 255;
            int glowRed = static_cast<int>(baseRed * titlePulse);
            int glowAlpha1 = static_cast<int>(150 * titlePulse);
            int glowAlpha2 = static_cast<int>(80 * titlePulse);
            
            gameTitle->setTextColor(UIColor(glowRed, static_cast<int>(100 * titlePulse), static_cast<int>(100 * titlePulse), 255));
            titleGlow1->setTextColor(UIColor(255, 50, 50, glowAlpha1));
            titleGlow2->setTextColor(UIColor(255, 30, 30, glowAlpha2));
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
    CloseWindow();

    return 0;
}
