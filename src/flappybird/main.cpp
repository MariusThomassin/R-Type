/*
** Flappy Bird Client
** Standalone game using the R-Type ECS engine
** Simple arcade game demonstrating engine reusability
*/

#include <raylib.h>

#include <iostream>
#include <string>

// Engine includes
#include "../engine/ecs/ECS.hpp"
#include "../engine/ecs/core/EventBus.hpp"
#include "../engine/ecs/events/InputEvents.hpp"
#include "../engine/ecs/events/InputManager.hpp"
#include "../engine/ecs/core/SystemManager.hpp"
#include "../engine/ecs/systems/MusicSystem.hpp"

// Flappy Bird includes
#include "FlappyComponents.hpp"
#include "FlappySystems.hpp"

using namespace rtype::ecs;

constexpr int SCREEN_WIDTH = 480;
constexpr int SCREEN_HEIGHT = 640;
constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

// Sky gradient colors (light blue to darker blue)
const Color SKY_TOP = {135, 206, 235, 255};     // Light sky blue
const Color SKY_BOTTOM = {70, 130, 180, 255};   // Steel blue

/**
 * @brief Draw a vertical gradient background (sky)
 */
void drawSkyBackground(int width, int height) {
    for (int y = 0; y < height; y++) {
        float t = static_cast<float>(y) / static_cast<float>(height);
        Color color = {
            static_cast<unsigned char>(SKY_TOP.r + (SKY_BOTTOM.r - SKY_TOP.r) * t),
            static_cast<unsigned char>(SKY_TOP.g + (SKY_BOTTOM.g - SKY_TOP.g) * t),
            static_cast<unsigned char>(SKY_TOP.b + (SKY_BOTTOM.b - SKY_TOP.b) * t),
            255
        };
        DrawLine(0, y, width, y, color);
    }
}

/**
 * @brief Draw some simple cloud decorations
 */
void drawClouds(float animTime) {
    Color cloudColor = {255, 255, 255, 200};
    
    // A few clouds at different positions
    float cloudSpeed = 30.0f;
    
    // Cloud 1
    float cloud1X = fmodf(100 + animTime * cloudSpeed, SCREEN_WIDTH + 200) - 100;
    DrawCircle(static_cast<int>(cloud1X), 80, 30, cloudColor);
    DrawCircle(static_cast<int>(cloud1X + 25), 75, 25, cloudColor);
    DrawCircle(static_cast<int>(cloud1X + 50), 80, 30, cloudColor);
    DrawCircle(static_cast<int>(cloud1X + 20), 90, 20, cloudColor);
    DrawCircle(static_cast<int>(cloud1X + 35), 92, 22, cloudColor);
    
    // Cloud 2
    float cloud2X = fmodf(300 + animTime * cloudSpeed * 0.7f, SCREEN_WIDTH + 200) - 100;
    DrawCircle(static_cast<int>(cloud2X), 150, 25, cloudColor);
    DrawCircle(static_cast<int>(cloud2X + 20), 145, 20, cloudColor);
    DrawCircle(static_cast<int>(cloud2X + 40), 150, 25, cloudColor);
    
    // Cloud 3
    float cloud3X = fmodf(500 + animTime * cloudSpeed * 1.2f, SCREEN_WIDTH + 200) - 100;
    DrawCircle(static_cast<int>(cloud3X), 200, 20, cloudColor);
    DrawCircle(static_cast<int>(cloud3X + 18), 195, 18, cloudColor);
    DrawCircle(static_cast<int>(cloud3X + 35), 200, 20, cloudColor);
}

int main() {
    // ==================== Raylib Initialization ====================
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Flappy Bird - R-Type Engine");
    SetTargetFPS(60);
    
    // ==================== Audio Initialization ====================
    InitAudioDevice();
    std::cout << "Audio device initialized. Ready: " << (IsAudioDeviceReady() ? "YES" : "NO") << std::endl;
    
    // ==================== Event Bus ====================
    EventBus eventBus;
    
    // ==================== Input Manager ====================
    events::InputManager inputManager(eventBus, nullptr);
    
    // ==================== ECS Setup ====================
    Registry registry;
    SystemManager systems(&registry);
    
    // ==================== Music System ====================
    MusicSystem musicSystem(eventBus);
    musicSystem.setMasterVolume(0.5f);
    
    // Try to play R-Type background music if available
    // (Flappy Bird can reuse R-Type assets)
    musicSystem.playTrack("assets/sound/music/Sketchbook 2024-10-13.ogg", 1.0f, true);
    
    // ==================== Game Systems ====================
    auto* flappySystem = systems.addSystem<flappy::FlappyBirdSystem>(eventBus, SCREEN_WIDTH, SCREEN_HEIGHT);
    auto* renderSystem = systems.addSystem<flappy::FlappyRenderSystem>(SCREEN_WIDTH, SCREEN_HEIGHT);
    
    // Initialize the game (creates bird, ground, etc.)
    flappySystem->initializeGame();
    
    // ==================== Game Loop ====================
    float accumulator = 0.0f;
    float animTime = 0.0f;
    
    std::cout << "==================================" << std::endl;
    std::cout << "     FLAPPY BIRD - R-Type Engine  " << std::endl;
    std::cout << "==================================" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  SPACE / LEFT CLICK - Flap" << std::endl;
    std::cout << "  R - Restart (when game over)" << std::endl;
    std::cout << "  ESC - Quit" << std::endl;
    std::cout << "==================================" << std::endl;
    
    while (!WindowShouldClose()) {
        float frameTime = GetFrameTime();
        accumulator += frameTime;
        animTime += frameTime;
        
        // Poll input
        inputManager.pollInput();
        
        // Update music
        musicSystem.update(frameTime);
        
        // Fixed timestep updates
        while (accumulator >= FIXED_TIMESTEP) {
            // Update game logic
            flappySystem->update(FIXED_TIMESTEP);
            
            // Flush deferred entity destructions
            registry.flushDeferred();
            
            accumulator -= FIXED_TIMESTEP;
        }
        
        // ==================== Rendering ====================
        BeginDrawing();
        
        // Draw sky background
        drawSkyBackground(SCREEN_WIDTH, SCREEN_HEIGHT);
        
        // Draw clouds (decorative)
        drawClouds(animTime);
        
        // Render game entities (pipes, bird, ground)
        renderSystem->update(frameTime);
        
        // Draw UI (score, game over screen)
        bool isWaiting = (flappySystem->getGamePhase() == flappy::FlappyBirdSystem::GamePhase::Waiting);
        bool isGameOver = (flappySystem->getGamePhase() == flappy::FlappyBirdSystem::GamePhase::GameOver);
        renderSystem->drawScore(flappySystem->getScore(), flappySystem->getHighScore(), isGameOver, isWaiting);
        
        // Title when waiting
        if (isWaiting) {
            const char* title = "FLAPPY BIRD";
            int titleWidth = MeasureText(title, 48);
            DrawText(title, SCREEN_WIDTH / 2 - titleWidth / 2 + 3, 153, 48, BLACK);
            DrawText(title, SCREEN_WIDTH / 2 - titleWidth / 2, 150, 48, WHITE);
            
            const char* subtitle = "Powered by R-Type Engine";
            int subWidth = MeasureText(subtitle, 16);
            DrawText(subtitle, SCREEN_WIDTH / 2 - subWidth / 2 + 1, 206, 16, BLACK);
            DrawText(subtitle, SCREEN_WIDTH / 2 - subWidth / 2, 205, 16, LIGHTGRAY);
        }
        
        // FPS counter (debug)
        DrawFPS(10, 10);
        
        EndDrawing();
    }
    
    // ==================== Cleanup ====================
    CloseAudioDevice();
    CloseWindow();
    
    return 0;
}
