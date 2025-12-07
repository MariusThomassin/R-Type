/*
** R-Type ECS - ShowoffSystem
** Demonstration mode showcasing bullet patterns and trajectories
** Press P to start/stop showoff mode
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/InputEvents.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "game/components/patterns/PatternSpawnerComponent.hpp"
#include "game/components/patterns/PatternFactory.hpp"

#include <vector>
#include <string>

namespace rtype::ecs {

    /**
     * @brief System that runs a demonstration of bullet patterns
     *
     * When activated via ShowoffStartEvent (P key):
     * - Spawns pattern spawners in sequence
     * - Cycles through different pattern types
     * - Shows off all available trajectories and shapes
     * - Displays current pattern name on screen
     */
    class ShowoffSystem : public ISystem {
        public:
            /**
             * @brief Construct a new Showoff System object
             * @param eventBus Reference to EventBus for input handling
             * @param screenWidth Width of the game screen
             * @param screenHeight Height of the game screen
             */
            ShowoffSystem(EventBus& eventBus, int screenWidth, int screenHeight)
                : m_eventBus(eventBus), m_screenWidth(screenWidth), m_screenHeight(screenHeight) {
                
                m_startSubId = m_eventBus.subscribe<events::ShowoffStartEvent>(
                    [this](const events::ShowoffStartEvent&) {
                        startShowoff();
                    }
                );
                
                m_endSubId = m_eventBus.subscribe<events::ShowoffEndEvent>(
                    [this](const events::ShowoffEndEvent&) {
                        stopShowoff();
                    }
                );
                
                initializePatternSequence();
            }

            /**
             * @brief Destroy the Showoff System object
             */
            ~ShowoffSystem() override {
                m_eventBus.unsubscribe<events::ShowoffStartEvent>(m_startSubId);
                m_eventBus.unsubscribe<events::ShowoffEndEvent>(m_endSubId);
            }

            /**
             * @brief Update the showoff system
             * @param dt Delta time since last update
             */
            void update(float dt) override {
                if (!m_registry || !m_active) return;

                m_phaseTimer += dt;
                m_totalTime += dt;

                // Check if current phase is complete
                if (m_phaseTimer >= m_phaseDuration) {
                    advancePhase();
                }
            }

            /**
             * @brief Get the system phase
             * @return SystemPhase
             */
            SystemPhase getPhase() const override {
                return SystemPhase::GameLogic;
            }

            /**
             * @brief Check if showoff mode is active
             * @return true if active
             */
            bool isActive() const { return m_active; }
            /**
             * @brief Get the name of the current pattern being shown
             * @return Pattern name
             */
            const std::string& getCurrentPatternName() const { return m_currentPatternName; }
            /**
             * @brief Get the current phase index
             * @return Phase index
             */
            int getCurrentPhase() const { return m_currentPhase; }
            /**
             * @brief Get total number of phases
             * @return Total phases
             */
            int getTotalPhases() const { return static_cast<int>(m_patternSequence.size()); }
            /**
             * @brief Get progress through current phase (0.0 to 1.0)
             * @return Phase progress
             */
            float getPhaseProgress() const { return m_phaseTimer / m_phaseDuration; }

        private:
            /**
             * @brief Start the showoff mode
             */
            struct PatternDemo {
                std::string name;
                float duration;
                std::function<void(ShowoffSystem*)> spawn;
            };

            /**
             * @brief EventBus reference for input handling
             */
            EventBus& m_eventBus;
            /**
             * @brief Screen width
             */
            int m_screenWidth;
            /**
             * @brief Screen height
             */
            int m_screenHeight;

            /**
             * @brief Whether showoff mode is active
             */
            bool m_active = false;
            /**
             * @brief Current phase index
             */
            int m_currentPhase = 0;
            /**
             * @brief Timer for current phase
             */
            float m_phaseTimer = 0.0f;
            /**
             * @brief Duration of current phase
             */
            float m_phaseDuration = 4.0f;
            /**
             * @brief Total elapsed time
             */
            float m_totalTime = 0.0f;
            /**
             * @brief Name of the current pattern
             */
            std::string m_currentPatternName;

            /**
             * @brief Sequence of patterns to demonstrate
             */
            std::vector<PatternDemo> m_patternSequence;
            /**
             * @brief Active spawner entities
             */
            std::vector<EntityId> m_spawnerEntities;

            /**
             * @brief Subscriber IDs for events
             */
            EventBus::SubscriberId m_startSubId;
            /**
             * @brief Subscriber ID for showoff end event
             */
            EventBus::SubscriberId m_endSubId;

            /**
             * @brief Initialize the sequence of patterns to demonstrate
             */
            void initializePatternSequence() {
                float centerX = m_screenWidth / 2.0f;
                float topY = 100.0f;

                // Phase 1: Simple Circle
                m_patternSequence.push_back({
                    "Circle Pattern",
                    4.0f,
                    [centerX, topY](ShowoffSystem* self) {
                        self->spawnPattern(centerX, topY,
                            PatternFactory::createCircle(24, 150.0f, BulletType::Pellet, BulletColor::Cyan));
                    }
                });

                // Phase 2: Rotating Spiral
                m_patternSequence.push_back({
                    "Rotating Spiral",
                    5.0f,
                    [centerX, topY](ShowoffSystem* self) {
                        self->spawnPattern(centerX, topY,
                            PatternFactory::createSpiral(4, 20, 180.0f, 60.0f, BulletType::Rice, BulletColor::Purple));
                    }
                });

                // Phase 3: Aimed Fan
                m_patternSequence.push_back({
                    "Aimed Fan (tracks player)",
                    4.0f,
                    [centerX, topY](ShowoffSystem* self) {
                        auto pattern = PatternFactory::createAimedFan(7, 45.0f, 200.0f, BulletType::Rice, BulletColor::Red);
                        pattern.repeatCount = 8;
                        pattern.repeatDelay = 0.4f;
                        self->spawnPattern(centerX, topY, pattern);
                    }
                });

                // Phase 4: Homing Bullets
                m_patternSequence.push_back({
                    "Homing Bullets",
                    5.0f,
                    [centerX, topY](ShowoffSystem* self) {
                        auto pattern = PatternFactory::createHoming(12, 100.0f, 3.0f, BulletType::Pellet, BulletColor::Magenta);
                        pattern.repeatCount = 3;
                        pattern.repeatDelay = 1.0f;
                        self->spawnPattern(centerX, topY, pattern);
                    }
                });

                // Phase 5: Double Helix
                m_patternSequence.push_back({
                    "Double Helix",
                    5.0f,
                    [centerX, topY](ShowoffSystem* self) {
                        self->spawnPattern(centerX, topY,
                            PatternFactory::createHelix(30, 200.0f, 120.0f, BulletType::Rice, BulletColor::Cyan, BulletColor::Orange));
                    }
                });

                // Phase 6: Sinusoidal Wave
                m_patternSequence.push_back({
                    "Sinusoidal Wave",
                    6.0f,
                    [centerX, topY](ShowoffSystem* self) {
                        auto pattern = PatternFactory::createWave(25, 140.0f, 200.0f, 0.8f, BulletType::Rice, BulletColor::Green);
                        pattern.repeatCount = 4;
                        pattern.repeatDelay = 0.3f;
                        self->spawnPattern(centerX, topY, pattern);
                    }
                });

                // Phase 7: Layered Rings
                m_patternSequence.push_back({
                    "Layered Rings",
                    4.0f,
                    [centerX, topY](ShowoffSystem* self) {
                        auto pattern = PatternFactory::createRings(5, 16, 80.0f, 40.0f, BulletType::Pellet, BulletColor::Yellow);
                        pattern.repeatCount = 3;
                        pattern.repeatDelay = 0.8f;
                        self->spawnPattern(centerX, topY, pattern);
                    }
                });

                // Phase 8: Cross Pattern
                m_patternSequence.push_back({
                    "Rotating Cross",
                    4.0f,
                    [centerX, topY](ShowoffSystem* self) {
                        auto pattern = PatternFactory::createCross(8, 200.0f, BulletType::Rice, BulletColor::Orange);
                        pattern.rotationSpeed = 45.0f;
                        pattern.repeatCount = 5;
                        pattern.repeatDelay = 0.3f;
                        self->spawnPattern(centerX, topY, pattern);
                    }
                });

                // Phase 9: Rose Pattern
                m_patternSequence.push_back({
                    "Rose/Flower Pattern",
                    4.0f,
                    [centerX, topY](ShowoffSystem* self) {
                        auto pattern = PatternFactory::createRose(6, 5, 180.0f, BulletType::Pellet, BulletColor::White);
                        pattern.repeatCount = 2;
                        pattern.repeatDelay = 1.0f;
                        self->spawnPattern(centerX, topY, pattern);
                    }
                });

                // Phase 10: Multi-spawner chaos
                m_patternSequence.push_back({
                    "Multi-Spawner Finale",
                    6.0f,
                    [this](ShowoffSystem* self) {
                        float leftX = self->m_screenWidth * 0.25f;
                        float rightX = self->m_screenWidth * 0.75f;
                        float topY = 80.0f;

                        auto spiral1 = PatternFactory::createSpiral(3, 15, 150.0f, 90.0f, BulletType::Rice, BulletColor::Cyan);
                        spiral1.repeatCount = 4;
                        spiral1.repeatDelay = 0.3f;
                        self->spawnPattern(leftX, topY, spiral1);

                        auto spiral2 = PatternFactory::createSpiral(3, 15, 150.0f, -90.0f, BulletType::Rice, BulletColor::Red);
                        spiral2.repeatCount = 4;
                        spiral2.repeatDelay = 0.3f;
                        self->spawnPattern(rightX, topY, spiral2);

                        auto center = PatternFactory::createCircle(16, 120.0f, BulletType::Pellet, BulletColor::Yellow);
                        center.repeatCount = 6;
                        center.repeatDelay = 0.5f;
                        self->spawnPattern(self->m_screenWidth / 2.0f, topY + 50.0f, center);
                    }
                });
            }

            /**
             * @brief Start the showoff mode
             */
            void startShowoff() {
                if (m_active) return;
                
                m_active = true;
                m_currentPhase = 0;
                m_phaseTimer = 0.0f;
                m_totalTime = 0.0f;
                m_spawnerEntities.clear();

                // Start first phase
                if (!m_patternSequence.empty()) {
                    const auto& demo = m_patternSequence[m_currentPhase];
                    m_currentPatternName = demo.name;
                    m_phaseDuration = demo.duration;
                    demo.spawn(this);
                }
            }

            /**
             * @brief Stop the showoff mode
             */
            void stopShowoff() {
                if (!m_active) return;
                
                m_active = false;
                m_currentPatternName = "";
                
                // Clean up all spawner entities
                for (EntityId entity : m_spawnerEntities) {
                    if (m_registry->entityExists(entity)) {
                        m_registry->destroyEntity(entity);
                    }
                }
                m_spawnerEntities.clear();
            }

            /**
             * @brief Advance to the next phase in the showoff sequence
             */
            void advancePhase() {
                // Clean up current spawners
                for (EntityId entity : m_spawnerEntities) {
                    if (m_registry->entityExists(entity)) {
                        m_registry->destroyEntity(entity);
                    }
                }
                m_spawnerEntities.clear();

                m_currentPhase++;
                m_phaseTimer = 0.0f;

                if (m_currentPhase >= static_cast<int>(m_patternSequence.size())) {
                    // Showoff complete - stop and notify
                    m_active = false;
                    m_currentPatternName = "Showoff Complete!";
                    m_eventBus.emit(events::ShowoffEndEvent{});
                    return;
                }

                // Start next phase
                const auto& demo = m_patternSequence[m_currentPhase];
                m_currentPatternName = demo.name;
                m_phaseDuration = demo.duration;
                demo.spawn(this);
            }

            /**
             * @brief Spawn a pattern at given position
             * @param x X position
             * @param y Y position 
             * @param pattern Bullet pattern to spawn
             */
            void spawnPattern(float x, float y, BulletPatternComponent pattern) {
                Entity spawner = m_registry->createEntity();
                
                m_registry->addComponent(spawner, TransformComponent(x, y));
                
                PatternSpawnerComponent spawnerComp;
                spawnerComp.addPattern(pattern);
                spawnerComp.autoStart = true;
                spawnerComp.loopPatterns = true;
                m_registry->addComponent(spawner, spawnerComp);
                
                m_spawnerEntities.push_back(spawner);
            }
        };
} // namespace rtype::ecs
