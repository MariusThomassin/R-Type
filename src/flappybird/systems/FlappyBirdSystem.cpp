/*
** Flappy Bird - FlappyBirdSystem Implementation
** Main game logic for Flappy Bird mode
*/

#include "FlappyBirdSystem.hpp"

#include <raylib.h>
#include <iostream>
#include <algorithm>

namespace flappy {

    FlappyBirdSystem::FlappyBirdSystem(rtype::ecs::EventBus& eventBus, int screenWidth, int screenHeight)
        : m_eventBus(eventBus)
        , m_screenWidth(screenWidth)
        , m_screenHeight(screenHeight)
        , m_rng(std::random_device{}())
    {
        // Ground takes up bottom portion
        m_groundY = static_cast<float>(screenHeight) - 100.0f;
        
        // Gap distribution: gaps can be between 20% and 70% of playable area
        float minGapY = static_cast<float>(screenHeight) * 0.25f;
        float maxGapY = m_groundY - static_cast<float>(screenHeight) * 0.25f;
        m_gapDistribution = std::uniform_real_distribution<float>(minGapY, maxGapY);
        
        // Subscribe to input events
        m_keyPressedSubId = m_eventBus.subscribe<rtype::ecs::events::KeyPressedEvent>(
            [this](const rtype::ecs::events::KeyPressedEvent& e) {
                if (e.key == rtype::ecs::events::KeyCode::Space ||
                    e.key == rtype::ecs::events::KeyCode::W ||
                    e.key == rtype::ecs::events::KeyCode::Up) {
                    m_flapRequested = true;
                }
                
                // R to restart when game over
                if (e.key == rtype::ecs::events::KeyCode::R && m_gamePhase == GamePhase::GameOver) {
                    resetGame();
                }
            }
        );
        
        m_mouseClickSubId = m_eventBus.subscribe<rtype::ecs::events::MouseButtonPressedEvent>(
            [this](const rtype::ecs::events::MouseButtonPressedEvent& e) {
                if (e.button == rtype::ecs::events::MouseButton::Left) {
                    m_flapRequested = true;
                }
            }
        );
        
        // Also subscribe to continuous key state for edge detection fallback
        m_keyStateSubId = m_eventBus.subscribe<rtype::ecs::events::KeyStateEvent>(
            [this](const rtype::ecs::events::KeyStateEvent& e) {
                // Edge detection: trigger flap on key down transition
                bool flapKeyDown = e.state.space || e.state.w || e.state.up;
                if (flapKeyDown && !m_flapKeyWasDown) {
                    m_flapRequested = true;
                }
                m_flapKeyWasDown = flapKeyDown;
            }
        );
    }

    FlappyBirdSystem::~FlappyBirdSystem() {
        m_eventBus.unsubscribe<rtype::ecs::events::KeyPressedEvent>(m_keyPressedSubId);
        m_eventBus.unsubscribe<rtype::ecs::events::MouseButtonPressedEvent>(m_mouseClickSubId);
        m_eventBus.unsubscribe<rtype::ecs::events::KeyStateEvent>(m_keyStateSubId);
    }

    void FlappyBirdSystem::initializeGame() {
        if (!m_registry) {
            std::cerr << "[FlappyBirdSystem] No registry set!" << std::endl;
            return;
        }
        
        // Create bird entity
        m_birdEntity = m_registry->createEntity();
        
        float birdStartX = static_cast<float>(m_screenWidth) * 0.3f;
        float birdStartY = static_cast<float>(m_screenHeight) * 0.4f;
        
        m_registry->addComponent(m_birdEntity, rtype::ecs::TransformComponent(birdStartX, birdStartY));
        m_registry->addComponent(m_birdEntity, rtype::ecs::VelocityComponent(0.0f, 0.0f));
        m_registry->addComponent(m_birdEntity, BirdComponent());
        m_registry->addComponent(m_birdEntity, rtype::ecs::ColliderComponent(-15.0f, -12.0f, 30.0f, 24.0f));
        
        // Create ground entity
        m_groundEntity = m_registry->createEntity();
        m_registry->addComponent(m_groundEntity, rtype::ecs::TransformComponent(0.0f, m_groundY));
        m_registry->addComponent(m_groundEntity, GroundComponent(m_screenWidth, 100.0f, m_pipeSpeed));
        
        // Create score entity
        m_scoreEntity = m_registry->createEntity();
        m_registry->addComponent(m_scoreEntity, FlappyScoreComponent());
        
        m_gamePhase = GamePhase::Waiting;
        m_score = 0;
        
        std::cout << "[FlappyBirdSystem] Game initialized!" << std::endl;
    }

    void FlappyBirdSystem::resetGame() {
        if (!m_registry) return;
        
        // Destroy all pipes
        for (auto& pipePair : m_pipePairs) {
            m_registry->destroyEntityDeferred(pipePair.first);
            m_registry->destroyEntityDeferred(pipePair.second);
        }
        m_pipePairs.clear();
        
        // Reset bird position and state
        if (m_registry->entityExists(m_birdEntity)) {
            auto& transform = m_registry->getComponent<rtype::ecs::TransformComponent>(m_birdEntity);
            auto& velocity = m_registry->getComponent<rtype::ecs::VelocityComponent>(m_birdEntity);
            auto& bird = m_registry->getComponent<BirdComponent>(m_birdEntity);
            
            transform.x = static_cast<float>(m_screenWidth) * 0.3f;
            transform.y = static_cast<float>(m_screenHeight) * 0.4f;
            transform.rotation = 0.0f;
            
            velocity.vx = 0.0f;
            velocity.vy = 0.0f;
            
            bird.isAlive = true;
            bird.hasStarted = false;
        }
        
        // Reset score
        if (m_registry->entityExists(m_scoreEntity)) {
            auto& scoreComp = m_registry->getComponent<FlappyScoreComponent>(m_scoreEntity);
            scoreComp.reset();
        }
        
        // Reset ground scroll
        if (m_registry->entityExists(m_groundEntity)) {
            auto& ground = m_registry->getComponent<GroundComponent>(m_groundEntity);
            ground.scrollOffset = 0.0f;
        }
        
        m_gamePhase = GamePhase::Waiting;
        m_score = 0;
        m_pipeSpawnTimer = 0.0f;
        m_flapRequested = false;
        
        std::cout << "[FlappyBirdSystem] Game reset!" << std::endl;
    }

    void FlappyBirdSystem::update(float dt) {
        if (!m_registry) return;
        
        // Handle flap input
        if (m_flapRequested) {
            handleFlap();
            m_flapRequested = false;
        }
        
        switch (m_gamePhase) {
            case GamePhase::Waiting:
                // Bird bobs up and down waiting for input
                if (m_registry->entityExists(m_birdEntity)) {
                    auto& transform = m_registry->getComponent<rtype::ecs::TransformComponent>(m_birdEntity);
                    auto& bird = m_registry->getComponent<BirdComponent>(m_birdEntity);
                    
                    // Gentle bobbing animation
                    static float bobTime = 0.0f;
                    bobTime += dt;
                    transform.y = static_cast<float>(m_screenHeight) * 0.4f + std::sin(bobTime * 3.0f) * 10.0f;
                    
                    bird.updateAnimation(dt, 0.0f);
                }
                break;
                
            case GamePhase::Playing:
                applyGravity(dt);
                updateBirdRotation();
                updatePipes(dt);
                updateGround(dt);
                checkCollisions();
                checkScoring();
                
                // Spawn new pipes
                m_pipeSpawnTimer += dt;
                if (m_pipeSpawnTimer >= m_pipeSpawnInterval) {
                    spawnPipePair();
                    m_pipeSpawnTimer = 0.0f;
                }
                
                // Update bird animation
                if (m_registry->entityExists(m_birdEntity)) {
                    auto& velocity = m_registry->getComponent<rtype::ecs::VelocityComponent>(m_birdEntity);
                    auto& bird = m_registry->getComponent<BirdComponent>(m_birdEntity);
                    bird.updateAnimation(dt, velocity.vy);
                }
                break;
                
            case GamePhase::GameOver:
                // Bird falls to ground
                applyGravity(dt);
                
                // Clamp bird to ground
                if (m_registry->entityExists(m_birdEntity)) {
                    auto& transform = m_registry->getComponent<rtype::ecs::TransformComponent>(m_birdEntity);
                    auto& velocity = m_registry->getComponent<rtype::ecs::VelocityComponent>(m_birdEntity);
                    
                    if (transform.y >= m_groundY - 15.0f) {
                        transform.y = m_groundY - 15.0f;
                        velocity.vy = 0.0f;
                    }
                }
                break;
        }
    }

    void FlappyBirdSystem::handleFlap() {
        if (!m_registry || !m_registry->entityExists(m_birdEntity)) return;
        
        auto& bird = m_registry->getComponent<BirdComponent>(m_birdEntity);
        
        if (!bird.isAlive) {
            // If game over, restart on flap
            if (m_gamePhase == GamePhase::GameOver) {
                resetGame();
            }
            return;
        }
        
        auto& velocity = m_registry->getComponent<rtype::ecs::VelocityComponent>(m_birdEntity);
        
        // Start game on first flap
        if (m_gamePhase == GamePhase::Waiting) {
            m_gamePhase = GamePhase::Playing;
            bird.hasStarted = true;
            
            // Spawn first pipe immediately
            spawnPipePair();
        }
        
        // Apply flap force
        velocity.vy = bird.flapForce;
    }

    void FlappyBirdSystem::applyGravity(float dt) {
        if (!m_registry || !m_registry->entityExists(m_birdEntity)) return;
        
        auto& bird = m_registry->getComponent<BirdComponent>(m_birdEntity);
        auto& velocity = m_registry->getComponent<rtype::ecs::VelocityComponent>(m_birdEntity);
        auto& transform = m_registry->getComponent<rtype::ecs::TransformComponent>(m_birdEntity);
        
        // Apply gravity
        velocity.vy += bird.gravity * dt;
        
        // Clamp fall speed
        if (velocity.vy > bird.maxFallSpeed) {
            velocity.vy = bird.maxFallSpeed;
        }
        
        // Update position
        transform.y += velocity.vy * dt;
        
        // Prevent going above screen
        if (transform.y < 0) {
            transform.y = 0;
            velocity.vy = 0;
        }
    }

    void FlappyBirdSystem::updateBirdRotation() {
        if (!m_registry || !m_registry->entityExists(m_birdEntity)) return;
        
        auto& velocity = m_registry->getComponent<rtype::ecs::VelocityComponent>(m_birdEntity);
        auto& transform = m_registry->getComponent<rtype::ecs::TransformComponent>(m_birdEntity);
        
        // Rotate bird based on velocity (-30 to +90 degrees)
        float targetRotation = velocity.vy * 0.1f;
        targetRotation = std::clamp(targetRotation, -30.0f, 90.0f);
        
        // Smooth rotation
        transform.rotation = transform.rotation * 0.9f + targetRotation * 0.1f;
    }

    void FlappyBirdSystem::spawnPipePair() {
        if (!m_registry) return;
        
        // Random gap position
        float gapCenterY = m_gapDistribution(m_rng);
        
        float pipeX = static_cast<float>(m_screenWidth) + m_pipeWidth;
        
        // Calculate pipe heights
        float topPipeHeight = gapCenterY - m_pipeGapSize / 2.0f;
        float bottomPipeY = gapCenterY + m_pipeGapSize / 2.0f;
        float bottomPipeHeight = m_groundY - bottomPipeY;
        
        // Create top pipe
        rtype::ecs::EntityId topPipe = m_registry->createEntity();
        m_registry->addComponent(topPipe, rtype::ecs::TransformComponent(pipeX, 0.0f));
        m_registry->addComponent(topPipe, rtype::ecs::VelocityComponent(-m_pipeSpeed, 0.0f));
        
        PipeComponent topPipeComp(true, m_pipeWidth, topPipeHeight);
        topPipeComp.gapCenterY = gapCenterY;
        m_registry->addComponent(topPipe, topPipeComp);
        
        m_registry->addComponent(topPipe, rtype::ecs::ColliderComponent(
            0.0f, 0.0f, m_pipeWidth, topPipeHeight
        ));
        
        // Create bottom pipe
        rtype::ecs::EntityId bottomPipe = m_registry->createEntity();
        m_registry->addComponent(bottomPipe, rtype::ecs::TransformComponent(pipeX, bottomPipeY));
        m_registry->addComponent(bottomPipe, rtype::ecs::VelocityComponent(-m_pipeSpeed, 0.0f));
        
        PipeComponent bottomPipeComp(false, m_pipeWidth, bottomPipeHeight);
        bottomPipeComp.gapCenterY = gapCenterY;
        m_registry->addComponent(bottomPipe, bottomPipeComp);
        
        m_registry->addComponent(bottomPipe, rtype::ecs::ColliderComponent(
            0.0f, 0.0f, m_pipeWidth, bottomPipeHeight
        ));
        
        m_pipePairs.push_back({topPipe, bottomPipe});
    }

    void FlappyBirdSystem::updatePipes(float dt) {
        if (!m_registry) return;
        
        std::vector<std::pair<rtype::ecs::EntityId, rtype::ecs::EntityId>> pipesToRemove;
        
        for (auto& pipePair : m_pipePairs) {
            // Update positions (VelocityComponent handles this via MovementSystem)
            // Just check for cleanup here
            
            if (m_registry->entityExists(pipePair.first)) {
                auto& transform = m_registry->getComponent<rtype::ecs::TransformComponent>(pipePair.first);
                auto& velocity = m_registry->getComponent<rtype::ecs::VelocityComponent>(pipePair.first);
                
                // Manual position update (in case MovementSystem isn't running)
                transform.x += velocity.vx * dt;
                
                // Remove pipes that have gone off screen
                if (transform.x < -m_pipeWidth - 50.0f) {
                    pipesToRemove.push_back(pipePair);
                }
            }
            
            // Update bottom pipe position too
            if (m_registry->entityExists(pipePair.second)) {
                auto& transform = m_registry->getComponent<rtype::ecs::TransformComponent>(pipePair.second);
                auto& velocity = m_registry->getComponent<rtype::ecs::VelocityComponent>(pipePair.second);
                transform.x += velocity.vx * dt;
            }
        }
        
        // Clean up off-screen pipes
        for (auto& pipePair : pipesToRemove) {
            m_registry->destroyEntityDeferred(pipePair.first);
            m_registry->destroyEntityDeferred(pipePair.second);
            
            m_pipePairs.erase(
                std::remove(m_pipePairs.begin(), m_pipePairs.end(), pipePair),
                m_pipePairs.end()
            );
        }
    }

    void FlappyBirdSystem::updateGround(float dt) {
        if (!m_registry || !m_registry->entityExists(m_groundEntity)) return;
        
        auto& ground = m_registry->getComponent<GroundComponent>(m_groundEntity);
        ground.updateScroll(dt);
    }

    void FlappyBirdSystem::checkCollisions() {
        if (!m_registry || !m_registry->entityExists(m_birdEntity)) return;
        
        auto& birdTransform = m_registry->getComponent<rtype::ecs::TransformComponent>(m_birdEntity);
        auto& birdCollider = m_registry->getComponent<rtype::ecs::ColliderComponent>(m_birdEntity);
        
        // Bird AABB
        float birdLeft = birdTransform.x + birdCollider.offsetX;
        float birdRight = birdLeft + birdCollider.width;
        float birdTop = birdTransform.y + birdCollider.offsetY;
        float birdBottom = birdTop + birdCollider.height;
        
        // Check ground collision
        if (birdBottom >= m_groundY) {
            triggerGameOver();
            return;
        }
        
        // Check pipe collisions
        for (auto& pipePair : m_pipePairs) {
            for (auto pipeId : {pipePair.first, pipePair.second}) {
                if (!m_registry->entityExists(pipeId)) continue;
                
                auto& pipeTransform = m_registry->getComponent<rtype::ecs::TransformComponent>(pipeId);
                auto& pipeCollider = m_registry->getComponent<rtype::ecs::ColliderComponent>(pipeId);
                
                // Pipe AABB
                float pipeLeft = pipeTransform.x + pipeCollider.offsetX;
                float pipeRight = pipeLeft + pipeCollider.width;
                float pipeTop = pipeTransform.y + pipeCollider.offsetY;
                float pipeBottom = pipeTop + pipeCollider.height;
                
                // AABB collision check
                if (birdRight > pipeLeft && birdLeft < pipeRight &&
                    birdBottom > pipeTop && birdTop < pipeBottom) {
                    triggerGameOver();
                    return;
                }
            }
        }
    }

    void FlappyBirdSystem::checkScoring() {
        if (!m_registry || !m_registry->entityExists(m_birdEntity)) return;
        
        auto& birdTransform = m_registry->getComponent<rtype::ecs::TransformComponent>(m_birdEntity);
        
        for (auto& pipePair : m_pipePairs) {
            // Use top pipe for scoring
            if (!m_registry->entityExists(pipePair.first)) continue;
            
            auto& pipeTransform = m_registry->getComponent<rtype::ecs::TransformComponent>(pipePair.first);
            auto& pipeComp = m_registry->getComponent<PipeComponent>(pipePair.first);
            
            // Check if bird has passed the pipe (bird center > pipe right edge)
            float pipeCenterX = pipeTransform.x + m_pipeWidth / 2.0f;
            
            if (!pipeComp.scored && birdTransform.x > pipeCenterX) {
                pipeComp.scored = true;
                
                // Also mark bottom pipe as scored
                if (m_registry->entityExists(pipePair.second)) {
                    auto& bottomPipeComp = m_registry->getComponent<PipeComponent>(pipePair.second);
                    bottomPipeComp.scored = true;
                }
                
                m_score++;
                
                // Update high score
                if (m_score > m_highScore) {
                    m_highScore = m_score;
                }
                
                // Update score component
                if (m_registry->entityExists(m_scoreEntity)) {
                    auto& scoreComp = m_registry->getComponent<FlappyScoreComponent>(m_scoreEntity);
                    scoreComp.addPoint();
                }
                
                // Emit score event
                m_eventBus.emit(FlappyScoreEvent{m_score});
                
                std::cout << "[FlappyBird] Score: " << m_score << std::endl;
            }
        }
    }

    void FlappyBirdSystem::triggerGameOver() {
        if (m_gamePhase == GamePhase::GameOver) return;
        
        m_gamePhase = GamePhase::GameOver;
        
        if (m_registry && m_registry->entityExists(m_birdEntity)) {
            auto& bird = m_registry->getComponent<BirdComponent>(m_birdEntity);
            bird.isAlive = false;
        }
        
        // Emit game over event
        m_eventBus.emit(FlappyGameOverEvent{m_score, m_highScore});
        
        std::cout << "[FlappyBird] Game Over! Score: " << m_score << ", High Score: " << m_highScore << std::endl;
    }

} // namespace flappy
