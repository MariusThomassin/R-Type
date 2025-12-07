/*
** R-Type ECS - StressTestSystem
** Performance stress test mode (toggle with Shift+P)
** Spawns massive amounts of bullets to test ECS performance
** Automatically progresses through intensity levels (20s each)
** Generates performance report on completion
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/core/EventBus.hpp"
#include "engine/ecs/events/InputEvents.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "game/components/patterns/PatternSpawnerComponent.hpp"
#include "game/components/patterns/PatternFactory.hpp"
#include "game/components/ProjectileComponent.hpp"
#include "engine/ecs/components/LifetimeComponent.hpp"
#include "common/GameMath.hpp"

#include <algorithm>
#include <numeric>
#include <fstream>
#include <ctime>
#include <cmath>
#include <raylib.h>
#include <vector>
#include <string>

namespace rtype::ecs {
    
    // Forward declarations
    class BulletPatternComponent;

    /**
     * @brief System for stress testing bullet performance
     *
     * Toggle with Shift+P to run automated stress test:
     * - Spends 20 seconds on each intensity level (1-5)
     * - Records FPS and entity counts throughout
     * - Generates performance report on completion
     */
    class StressTestSystem : public ISystem {
        public:
            static constexpr float PHASE_DURATION = 20.0f;  // 20 seconds per intensity level
            static constexpr int MAX_INTENSITY = 5;
            static constexpr int FPS_SAMPLE_RATE = 10;  // Sample FPS every N frames

            /**
             * @brief Struct to hold performance stats for each phase
             */
            struct PhaseStats {
                int intensity = 0;
                float duration = 0.0f;
                int minFps = 9999;
                int maxFps = 0;
                float avgFps = 0.0f;
                int maxEntities = 0;
                int totalWaves = 0;
                std::vector<int> fpsSamples;
            };

            /**
             * @brief Construct a new Stress Test System object
             * @param eventBus Reference to EventBus for input handling
             * @param screenWidth Width of the game screen
             * @param screenHeight Height of the game screen
             */
            StressTestSystem(EventBus& eventBus, int screenWidth, int screenHeight)
                : m_eventBus(eventBus), m_screenWidth(screenWidth), m_screenHeight(screenHeight) {
                
                m_toggleSubId = m_eventBus.subscribe<events::StressTestToggleEvent>(
                    [this](const events::StressTestToggleEvent&) {
                        toggle();
                    }
                );
            }

            /**
             * @brief Destroy the Stress Test System object
             */
            ~StressTestSystem() override {
                m_eventBus.unsubscribe<events::StressTestToggleEvent>(m_toggleSubId);
            }

            /**
             * @brief Update the stress test system
             * @param dt Delta time since last update
             */
            void update(float dt) override;

            /**
             * @brief Get the system phase
             * @return SystemPhase
             */
            SystemPhase getPhase() const override {
                return SystemPhase::GameLogic;
            }

            /**
             * @brief Check if the stress test is currently active
             * @return true if active
             */
            bool isActive() const { return m_active; }
            /**
             * @brief Check if the stress test has completed all phases
             * @return true if complete
             */
            bool isComplete() const { return m_testComplete; }
            /**
             * @brief Get current bullet count
             * @return Bullet count
             */
            int getBulletCount() const { return m_bulletCount; }
            /**
             * @brief Get current wave count
             * @return Wave count
             */
            int getWaveCount() const { return m_waveCount; }
            /**
             * @brief Get current spawner count
             * @return Spawner count
             */
            int getSpawnerCount() const { return static_cast<int>(m_spawnerEntities.size()); }
            /**
             * @brief Get total elapsed time
             * @return Total time
             */
            float getTotalTime() const { return m_totalTime; }
            /**
             * @brief Get elapsed time in current phase
             * @return Phase time
             */
            float getPhaseTime() const { return m_phaseTime; }
            /**
             * @brief Get progress of current phase as a fraction
             * @return Phase progress (0.0 to 1.0)
             */
            float getPhaseProgress() const { return m_phaseTime / PHASE_DURATION; }
            /**
             * @brief Get current intensity level
             * @return Intensity level (1 to MAX_INTENSITY)
             */
            int getCurrentPhase() const { return m_intensity; }
            /**
             * @brief Get total number of intensity phases
             * @return Total phases
             */
            int getTotalPhases() const { return MAX_INTENSITY; }

            /**
             * @brief Set the intensity level manually (for testing)
             * @param level Intensity level (1 to MAX_INTENSITY)
             */
            void setIntensity(int level) { 
                m_intensity = std::max(1, std::min(MAX_INTENSITY, level));
                updateIntensitySettings();
            }
            /**
             * @brief Get the current intensity level
             * @return Intensity level
             */
            int getIntensity() const { return m_intensity; }

            /**
             * @brief Get the filename of the generated report
             * @return Report filename
             */
            const std::string& getReportFilename() const { return m_reportFilename; }

        private:
            /**
             * @brief Reference to EventBus for input handling
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
             * @brief Current state variables
             */
            bool m_active = false;
            /**
             * @brief Whether the stress test has completed all phases
             */
            bool m_testComplete = false;
            /**
             * @brief Timers and counters
             */
            float m_timer = 0.0f;
            /**
             * @brief Total elapsed time since start
             */
            float m_totalTime = 0.0f;
            /**
             * @brief Elapsed time in current intensity phase
             */
            float m_phaseTime = 0.0f;
            /**
             * @brief Current wave count
             */
            int m_waveCount = 0;
            /**
             * @brief Current bullet count
             */
            int m_bulletCount = 0;
            /**
             * @brief Current frame count
             */
            int m_frameCount = 0;
            
            /**
             * @brief Current intensity level
             */
            int m_intensity = 1;
            /**
             * @brief Bullet spawn settings
             */
            float m_spawnInterval = 0.5f;
            /**
             * @brief Bullets per wave
             */
            int m_bulletsPerWave = 48;
            /**
             * @brief Spawner grid size
             */
            int m_spawnerGridSize = 3;
            /**
             * @brief Active spawner entities
             */
            std::vector<EntityId> m_spawnerEntities;

            /**
             * @brief Current phase performance stats
             */
            PhaseStats m_currentPhaseStats;
            /**
             * @brief All phase results
             */
            std::vector<PhaseStats> m_phaseResults;
            /**
             * @brief Generated report filename
             */
            std::string m_reportFilename;

            /**
             * @brief Subscriber ID for stress test toggle event
             */
            EventBus::SubscriberId m_toggleSubId;

            /**
             * @brief Toggle the stress test mode on/off
             */
            void toggle() {
                if (m_active) {
                    stop();
                } else {
                    start();
                }
            }

            /**
             * @brief Start the stress test mode
             */
            void start();

            /**
             * @brief Stop the stress test mode
             */
            void stop();

            /**
             * @brief Sample current performance metrics
             */
            void samplePerformance();

            /**
             * @brief Finalize statistics for current phase
             */
            void finalizePhaseStats();

            /**
             * @brief Advance to the next intensity phase
             */
            void advancePhase();

            /**
             * @brief Generate performance report and save to file
             */
            void generateReport();

            /**
             * @brief Update bullet spawn settings based on intensity level
             */
            void updateIntensitySettings();

            /**
             * @brief Spawn a wave of stress test bullets
             */
            void spawnStressWave();

            /**
             * @brief Spawn a stress test pattern spawner at given position
             * @param x X position
             * @param y Y position
             * @param patternIndex Index to select pattern type
             */
            void spawnStressPattern(float x, float y, int patternIndex);

            /**
             * @brief Create a stress test bullet pattern based on index
             * @param index Pattern index to determine type
             * @return BulletPatternComponent
             */
            BulletPatternComponent createStressPattern(int index);

            /**
             * @brief Update current bullet count from registry
             */
            void updateStats();
        };
} // namespace rtype::ecs
