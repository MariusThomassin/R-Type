/*
** R-Type Client
** Main game client with ECS architecture
** Uses fixed timestep for game logic, uncapped rendering
** Event-driven input via EventBus
*/

#include <raylib.h>

#include <iostream>
#include "engine/ecs/ECS.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/Events.hpp"
#include "engine/ecs/core/SystemManager.hpp"
#include "game/Components.hpp"
#include "game/Systems.hpp"
#include "engine/ui/UIManager.hpp"
#include "engine/ui/widgets/Label.hpp"
#include "engine/ui/widgets/Button.hpp"

using namespace rtype::ecs;
using namespace rtype::ecs::events;
using namespace rtype::ui;

using rtype::ecs::BulletType;
using rtype::ecs::BulletColor;

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
    auto players = registry.getEntitiesWith<PlayerComponent, TransformComponent>();

    constexpr float MARGIN = 30.0f;

    for (EntityId entity : players) {
        auto& transform = registry.getComponent<TransformComponent>(entity);

        if (transform.x < MARGIN) transform.x = MARGIN;
        if (transform.x > SCREEN_WIDTH - MARGIN) transform.x = SCREEN_WIDTH - MARGIN;
        if (transform.y < MARGIN) transform.y = MARGIN;
        if (transform.y > SCREEN_HEIGHT - MARGIN) transform.y = SCREEN_HEIGHT - MARGIN;
    }
}

int main() {
    // ==================== Raylib Initialization ====================
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "R-Type");

    // ==================== Event Bus ====================
    EventBus eventBus;

    // ==================== Input Manager ====================
    rtype::ecs::events::InputManager inputManager(eventBus);

    // ==================== UI Manager ====================
    rtype::ui::UIManager uiManager(eventBus);

    eventBus.subscribe<rtype::ecs::events::MouseButtonPressedEvent>([&uiManager](const rtype::ecs::events::MouseButtonPressedEvent& e) {
        uiManager.handleMouseClick(e);
    });
    eventBus.subscribe<rtype::ecs::events::MouseMoveEvent>([&uiManager](const rtype::ecs::events::MouseMoveEvent& e) {
        uiManager.handleMouseMove(e);
    });
    eventBus.subscribe<rtype::ecs::events::KeyPressedEvent>([&uiManager](const rtype::ecs::events::KeyPressedEvent& e) {
        uiManager.handleKeyPress(e.key);
    }); // Optional for testing focus/text

    // ==================== Game State Management ====================
    GameState gameState = GameState::MENU;
    
    // ==================== ECS Setup ====================
    Registry registry;
    SystemManager systems(&registry);
    bool shouldExit = false;

    // -- Create game title --
    auto gameTitle = std::make_shared<rtype::ui::Label>("R-TYPE", 64.0f);
    gameTitle->setPosition(SCREEN_WIDTH / 2.0f - 150, 100.0f);
    gameTitle->setSize(300.0f, 80.0f);
    gameTitle->setBackgroundColor(rtype::ui::UIColor(0, 0, 0, 0)); // Transparent
    gameTitle->setTextColor(rtype::ui::UIColor(255, 50, 50, 255)); // Rouge vif
    uiManager.addWidget(gameTitle);

    auto subtitle = std::make_shared<rtype::ui::Label>("The classic side-scrolling shooter", 24.0f);
    subtitle->setPosition(SCREEN_WIDTH / 2.0f - 200, 180.0f);
    subtitle->setSize(400.0f, 40.0f);
    subtitle->setBackgroundColor(rtype::ui::UIColor(0, 0, 0, 0)); // Transparent
    subtitle->setTextColor(rtype::ui::UIColor(200, 200, 200, 255)); // Light gray
    uiManager.addWidget(subtitle);

    // -- Create Play button --
    auto playButton = std::make_shared<rtype::ui::Button>("PLAY");
    playButton->setPosition(SCREEN_WIDTH / 2.0f - 100, 250.0f);
    playButton->setSize(200.0f, 50.0f);
    playButton->setBackgroundColor(rtype::ui::UIColor(0, 180, 0, 255)); // Vert
    playButton->setTextColor(rtype::ui::UIColor(255, 255, 255, 255));
    playButton->setOnClick([&gameState]() {
        std::cout << "Play button clicked! Starting game..." << std::endl;
        gameState = GameState::PLAYING;
    });
    uiManager.addWidget(playButton);

    // -- Create Exit button --
    auto exitButton = std::make_shared<rtype::ui::Button>("EXIT");
    exitButton->setPosition(SCREEN_WIDTH / 2.0f - 100, 320.0f);
    exitButton->setSize(200.0f, 50.0f);
    exitButton->setBackgroundColor(rtype::ui::UIColor(180, 0, 0, 255)); // Rouge
    exitButton->setTextColor(rtype::ui::UIColor(255, 255, 255, 255));
    exitButton->setOnClick([&shouldExit]() {
        std::cout << "Exit button clicked! Closing window..." << std::endl;
        shouldExit = true;
    });
    uiManager.addWidget(exitButton);

    auto* inputSystem = systems.addSystem<InputSystem>(eventBus, 350.0f);
    systems.addSystem<MovementSystem>();
    auto* bulletSystem = systems.addSystem<BulletSystem>(eventBus);
    bulletSystem->setScreenSize(SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* renderSystem = systems.addSystem<RenderSystem>(SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* debugSystem = systems.addSystem<DebugSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    debugSystem->setTextures(renderSystem->getTextures());
    debugSystem->init();
    
    renderSystem->setOverlayCallback([debugSystem]() {
        debugSystem->draw();
    });

    renderSystem->setUIManager(&uiManager);
    renderSystem->setGameStatePtr(&gameState);

    // ==================== Create Game Entities ====================
    Entity background = createBackground(registry, SCREEN_WIDTH, SCREEN_HEIGHT);
    (void)background;
    
    Entity player = createPlayer(registry, 1);
    (void)player;

    // ==================== Game Loop (Fixed Timestep) ====================
    float accumulator = 0.0f;

    while (!WindowShouldClose() && !shouldExit) {
        float frameTime = GetFrameTime();
        accumulator += frameTime;

        inputManager.pollInput();

        // Update UI only in menu state
        if (gameState == GameState::MENU) {
            uiManager.update(frameTime);
        }
        
        // Allow ESC to return to menu from game
        if (gameState == GameState::PLAYING && IsKeyPressed(KEY_ESCAPE)) {
            std::cout << "Returning to menu..." << std::endl;
            gameState = GameState::MENU;
        }

        while (accumulator >= FIXED_TIMESTEP) {
            // Update game systems only when playing
            if (gameState == GameState::PLAYING) {
                inputSystem->update(FIXED_TIMESTEP);
                debugSystem->update(FIXED_TIMESTEP);
                systems.getSystem<MovementSystem>()->update(FIXED_TIMESTEP);
                bulletSystem->update(FIXED_TIMESTEP);
                clampPlayerToScreen(registry);
            }
            accumulator -= FIXED_TIMESTEP;
        }

        // Always use RenderSystem for consistent background
        renderSystem->update(frameTime);
    }

    // ==================== Cleanup ====================
    CloseWindow();

    return 0;
}
