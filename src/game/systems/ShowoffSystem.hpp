/*
** R-Type ECS - ShowoffSystem
** Demonstration mode showcasing bullet patterns and trajectories
** Press P to start/stop showoff mode
*/

#pragma once

#include "../../engine/ecs/core/ISystem.hpp"
#include "../../engine/ecs/core/Registry.hpp"
#include "../../engine/ecs/core/EventBus.hpp"
#include "../../engine/ecs/events/InputEvents.hpp"
#include "../../engine/ecs/components/TransformComponent.hpp"
#include "../components/patterns/PatternSpawnerComponent.hpp"
#include "../components/patterns/PatternFactory.hpp"

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

        ~ShowoffSystem() override {
            m_eventBus.unsubscribe<events::ShowoffStartEvent>(m_startSubId);
            m_eventBus.unsubscribe<events::ShowoffEndEvent>(m_endSubId);
        }

        void update(float dt) override {
            if (!m_registry || !m_active) return;

            m_phaseTimer += dt;
            m_totalTime += dt;

            // Check if current phase is complete
            if (m_phaseTimer >= m_phaseDuration) {
                advancePhase();
            }
        }

        SystemPhase getPhase() const override {
            return SystemPhase::GameLogic;
        }

        bool isActive() const { return m_active; }
        const std::string& getCurrentPatternName() const { return m_currentPatternName; }
        int getCurrentPhase() const { return m_currentPhase; }
        int getTotalPhases() const { return static_cast<int>(m_patternSequence.size()); }
        float getPhaseProgress() const { return m_phaseTimer / m_phaseDuration; }

    private:
        struct PatternDemo {
            std::string name;
            float duration;
            std::function<void(ShowoffSystem*)> spawn;
        };

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

    private:
        EventBus& m_eventBus;
        int m_screenWidth;
        int m_screenHeight;
        
        bool m_active = false;
        int m_currentPhase = 0;
        float m_phaseTimer = 0.0f;
        float m_phaseDuration = 4.0f;
        float m_totalTime = 0.0f;
        std::string m_currentPatternName;
        
        std::vector<PatternDemo> m_patternSequence;
        std::vector<EntityId> m_spawnerEntities;
        
        EventBus::SubscriberId m_startSubId;
        EventBus::SubscriberId m_endSubId;
    };

} // namespace rtype::ecs
