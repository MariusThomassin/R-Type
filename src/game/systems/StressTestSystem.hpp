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

#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <ctime>
#include <algorithm>
#include <numeric>

namespace rtype::ecs {

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

        StressTestSystem(EventBus& eventBus, int screenWidth, int screenHeight)
            : m_eventBus(eventBus), m_screenWidth(screenWidth), m_screenHeight(screenHeight) {
            
            m_toggleSubId = m_eventBus.subscribe<events::StressTestToggleEvent>(
                [this](const events::StressTestToggleEvent&) {
                    toggle();
                }
            );
        }

        ~StressTestSystem() override {
            m_eventBus.unsubscribe<events::StressTestToggleEvent>(m_toggleSubId);
        }

        void update(float dt) override {
            if (!m_registry || !m_active) return;

            m_timer += dt;
            m_totalTime += dt;
            m_phaseTime += dt;
            m_frameCount++;

            // Sample FPS periodically
            if (m_frameCount % FPS_SAMPLE_RATE == 0) {
                samplePerformance();
            }

            // Check if current phase is complete
            if (m_phaseTime >= PHASE_DURATION) {
                finalizePhaseStats();
                advancePhase();
            }

            // Spawn new wave periodically
            if (m_timer >= m_spawnInterval) {
                spawnStressWave();
                m_timer = 0.0f;
                m_waveCount++;
                m_currentPhaseStats.totalWaves++;
            }

            // Update stats
            updateStats();
        }

        SystemPhase getPhase() const override {
            return SystemPhase::GameLogic;
        }

        bool isActive() const { return m_active; }
        bool isComplete() const { return m_testComplete; }
        int getBulletCount() const { return m_bulletCount; }
        int getWaveCount() const { return m_waveCount; }
        int getSpawnerCount() const { return static_cast<int>(m_spawnerEntities.size()); }
        float getTotalTime() const { return m_totalTime; }
        float getPhaseTime() const { return m_phaseTime; }
        float getPhaseProgress() const { return m_phaseTime / PHASE_DURATION; }
        int getCurrentPhase() const { return m_intensity; }
        int getTotalPhases() const { return MAX_INTENSITY; }

        // Configuration
        void setIntensity(int level) { 
            m_intensity = std::max(1, std::min(MAX_INTENSITY, level));
            updateIntensitySettings();
        }
        int getIntensity() const { return m_intensity; }

    private:
        void toggle() {
            if (m_active) {
                stop();
            } else {
                start();
            }
        }

        void start() {
            m_active = true;
            m_testComplete = false;
            m_timer = 0.0f;
            m_totalTime = 0.0f;
            m_phaseTime = 0.0f;
            m_waveCount = 0;
            m_bulletCount = 0;
            m_frameCount = 0;
            m_intensity = 1;  // Start at intensity 1
            
            // Clear previous results
            m_phaseResults.clear();
            m_currentPhaseStats = PhaseStats{};
            m_currentPhaseStats.intensity = m_intensity;
            
            updateIntensitySettings();
            
            // Initial spawn burst
            spawnStressWave();
        }

        void stop() {
            m_active = false;
            
            // Clean up all spawners
            for (EntityId entity : m_spawnerEntities) {
                if (m_registry->entityExists(entity)) {
                    m_registry->destroyEntity(entity);
                }
            }
            m_spawnerEntities.clear();
        }

        void samplePerformance() {
            int currentFps = static_cast<int>(1.0f / GetFrameTime());
            m_currentPhaseStats.fpsSamples.push_back(currentFps);
            
            if (currentFps < m_currentPhaseStats.minFps) {
                m_currentPhaseStats.minFps = currentFps;
            }
            if (currentFps > m_currentPhaseStats.maxFps) {
                m_currentPhaseStats.maxFps = currentFps;
            }
            if (m_bulletCount > m_currentPhaseStats.maxEntities) {
                m_currentPhaseStats.maxEntities = m_bulletCount;
            }
        }

        void finalizePhaseStats() {
            m_currentPhaseStats.duration = m_phaseTime;
            
            // Calculate average FPS
            if (!m_currentPhaseStats.fpsSamples.empty()) {
                float sum = std::accumulate(m_currentPhaseStats.fpsSamples.begin(), 
                                           m_currentPhaseStats.fpsSamples.end(), 0.0f);
                m_currentPhaseStats.avgFps = sum / m_currentPhaseStats.fpsSamples.size();
            }
            
            m_phaseResults.push_back(m_currentPhaseStats);
        }

        void advancePhase() {
            // Clean up current spawners before next phase
            for (EntityId entity : m_spawnerEntities) {
                if (m_registry->entityExists(entity)) {
                    m_registry->destroyEntity(entity);
                }
            }
            m_spawnerEntities.clear();

            m_intensity++;
            m_phaseTime = 0.0f;

            if (m_intensity > MAX_INTENSITY) {
                // Test complete - generate report
                generateReport();
                m_testComplete = true;
                stop();
                return;
            }

            // Start next phase
            m_currentPhaseStats = PhaseStats{};
            m_currentPhaseStats.intensity = m_intensity;
            updateIntensitySettings();
            
            // Initial spawn for new phase
            spawnStressWave();
        }

        void generateReport() {
            // Get current time for filename
            std::time_t now = std::time(nullptr);
            char timeStr[64];
            std::strftime(timeStr, sizeof(timeStr), "%Y%m%d_%H%M%S", std::localtime(&now));
            
            std::string filename = "stress_test_report_" + std::string(timeStr) + ".txt";
            std::ofstream report(filename);
            
            if (!report.is_open()) {
                return;  // Failed to create file
            }

            report << "===============================================\n";
            report << "       R-Type ECS Stress Test Report\n";
            report << "===============================================\n\n";
            
            std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
            report << "Generated: " << timeStr << "\n";
            report << "Total Test Duration: " << m_totalTime << " seconds\n";
            report << "Total Waves Spawned: " << m_waveCount << "\n\n";

            report << "-----------------------------------------------\n";
            report << "              Per-Phase Results\n";
            report << "-----------------------------------------------\n\n";

            int overallMinFps = 9999;
            int overallMaxFps = 0;
            float overallAvgFps = 0.0f;
            int overallMaxEntities = 0;

            for (const auto& phase : m_phaseResults) {
                report << "INTENSITY LEVEL " << phase.intensity << "\n";
                report << "  Duration:     " << phase.duration << "s\n";
                report << "  FPS (min):    " << phase.minFps << "\n";
                report << "  FPS (max):    " << phase.maxFps << "\n";
                report << "  FPS (avg):    " << static_cast<int>(phase.avgFps) << "\n";
                report << "  Max Entities: " << phase.maxEntities << "\n";
                report << "  Waves:        " << phase.totalWaves << "\n";
                report << "\n";

                if (phase.minFps < overallMinFps) overallMinFps = phase.minFps;
                if (phase.maxFps > overallMaxFps) overallMaxFps = phase.maxFps;
                overallAvgFps += phase.avgFps;
                if (phase.maxEntities > overallMaxEntities) overallMaxEntities = phase.maxEntities;
            }

            if (!m_phaseResults.empty()) {
                overallAvgFps /= m_phaseResults.size();
            }

            report << "-----------------------------------------------\n";
            report << "              Overall Summary\n";
            report << "-----------------------------------------------\n\n";
            report << "  Overall FPS (min): " << overallMinFps << "\n";
            report << "  Overall FPS (max): " << overallMaxFps << "\n";
            report << "  Overall FPS (avg): " << static_cast<int>(overallAvgFps) << "\n";
            report << "  Peak Entity Count: " << overallMaxEntities << "\n\n";

            // Performance grade
            report << "-----------------------------------------------\n";
            report << "              Performance Grade\n";
            report << "-----------------------------------------------\n\n";
            
            std::string grade;
            std::string comment;
            
            if (overallMinFps >= 55) {
                grade = "A+ (Excellent)";
                comment = "Exceptional performance! ECS handles stress with ease.";
            } else if (overallMinFps >= 45) {
                grade = "A (Great)";
                comment = "Great performance throughout all intensity levels.";
            } else if (overallMinFps >= 35) {
                grade = "B (Good)";
                comment = "Solid performance with minor dips at high intensity.";
            } else if (overallMinFps >= 25) {
                grade = "C (Acceptable)";
                comment = "Acceptable performance, may need optimization for high entity counts.";
            } else if (overallMinFps >= 15) {
                grade = "D (Poor)";
                comment = "Performance issues detected. Consider optimization.";
            } else {
                grade = "F (Critical)";
                comment = "Severe performance problems. Immediate optimization needed.";
            }

            report << "  Grade: " << grade << "\n";
            report << "  " << comment << "\n\n";

            report << "===============================================\n";
            report << "                 End of Report\n";
            report << "===============================================\n";

            report.close();
            
            m_reportFilename = filename;
        }

        void updateIntensitySettings() {
            // Scale parameters based on intensity (1-5)
            m_spawnInterval = 0.5f / m_intensity;
            m_bulletsPerWave = 48 * m_intensity;
            m_spawnerGridSize = 2 + m_intensity;
        }

        void spawnStressWave() {
            float centerX = m_screenWidth / 2.0f;
            float centerY = m_screenHeight / 3.0f;
            
            // Rotate spawn positions each wave for variety
            float waveAngle = m_waveCount * 15.0f * GameMath::DEG_TO_RAD;
            
            int gridSize = m_spawnerGridSize;
            
            for (int i = 0; i < gridSize; ++i) {
                float angle = (2.0f * GameMath::M_PI_F * i) / gridSize + waveAngle;
                float radius = 100.0f + (m_waveCount % 3) * 50.0f;
                
                float x = centerX + std::cos(angle) * radius;
                float y = centerY + std::sin(angle) * radius * 0.5f; // Elliptical
                
                // Clamp to screen
                x = std::max(50.0f, std::min(static_cast<float>(m_screenWidth - 50), x));
                y = std::max(50.0f, std::min(static_cast<float>(m_screenHeight / 2), y));
                
                spawnStressPattern(x, y, i);
            }
        }

        void spawnStressPattern(float x, float y, int patternIndex) {
            Entity spawner = m_registry->createEntity();
            
            m_registry->addComponent(spawner, TransformComponent(x, y));
            
            PatternSpawnerComponent spawnerComp;
            
            // Cycle through different pattern types for variety
            BulletPatternComponent pattern = createStressPattern(patternIndex);
            spawnerComp.addPattern(pattern);
            spawnerComp.autoStart = true;
            spawnerComp.loopPatterns = true;
            
            m_registry->addComponent(spawner, spawnerComp);
            m_spawnerEntities.push_back(spawner);
        }

        BulletPatternComponent createStressPattern(int index) {
            int bulletCount = m_bulletsPerWave / m_spawnerGridSize;
            float speed = 120.0f + (index * 20.0f);
            
            // Cycle through colors
            BulletColor colors[] = {
                BulletColor::Cyan, BulletColor::Magenta, BulletColor::Yellow,
                BulletColor::Green, BulletColor::Orange, BulletColor::Purple,
                BulletColor::Red, BulletColor::Blue
            };
            BulletColor color = colors[index % 8];
            
            // Different pattern types based on index
            switch (index % 6) {
                case 0: {
                    // Dense circle
                    auto pattern = PatternFactory::createCircle(bulletCount, speed, BulletType::Pellet, color);
                    pattern.rotationSpeed = 30.0f + index * 10.0f;
                    pattern.repeatCount = 0; // Infinite
                    pattern.repeatDelay = 0.3f / m_intensity;
                    return pattern;
                }
                case 1: {
                    // Fast spiral
                    auto pattern = PatternFactory::createSpiral(6, bulletCount / 6, speed * 1.2f, 90.0f, BulletType::Rice, color);
                    pattern.repeatCount = 0;
                    pattern.repeatDelay = 0.4f / m_intensity;
                    return pattern;
                }
                case 2: {
                    // Multiple rings
                    auto pattern = PatternFactory::createRings(4, bulletCount / 4, speed, 30.0f, BulletType::Pellet, color);
                    pattern.repeatCount = 0;
                    pattern.repeatDelay = 0.5f / m_intensity;
                    return pattern;
                }
                case 3: {
                    // Rose pattern
                    auto pattern = PatternFactory::createRose(8, bulletCount / 8, speed, BulletType::Rice, color);
                    pattern.rotationSpeed = 45.0f;
                    pattern.repeatCount = 0;
                    pattern.repeatDelay = 0.6f / m_intensity;
                    return pattern;
                }
                case 4: {
                    // Cross pattern
                    auto pattern = PatternFactory::createCross(bulletCount / 4, speed * 0.8f, BulletType::Pellet, color);
                    pattern.rotationSpeed = 60.0f;
                    pattern.repeatCount = 0;
                    pattern.repeatDelay = 0.25f / m_intensity;
                    return pattern;
                }
                default: {
                    // Dense aimed fan
                    auto pattern = PatternFactory::createAimedFan(bulletCount / 2, 60.0f, speed, BulletType::Rice, color);
                    pattern.repeatCount = 0;
                    pattern.repeatDelay = 0.35f / m_intensity;
                    return pattern;
                }
            }
        }

        void updateStats() {
            // Count bullets (entities with ProjectileComponent)
            m_bulletCount = 0;
            m_registry->forEach<TransformComponent>(
                [this](EntityId) {
                    m_bulletCount++;
                }
            );
            // Subtract non-bullet entities (rough estimate: spawners + player + background)
            m_bulletCount = std::max(0, m_bulletCount - static_cast<int>(m_spawnerEntities.size()) - 5);
        }

    public:
        const std::string& getReportFilename() const { return m_reportFilename; }

    private:
        EventBus& m_eventBus;
        int m_screenWidth;
        int m_screenHeight;
        
        bool m_active = false;
        bool m_testComplete = false;
        float m_timer = 0.0f;
        float m_totalTime = 0.0f;
        float m_phaseTime = 0.0f;
        int m_waveCount = 0;
        int m_bulletCount = 0;
        int m_frameCount = 0;
        
        // Intensity settings (1-5)
        int m_intensity = 1;
        float m_spawnInterval = 0.5f;
        int m_bulletsPerWave = 48;
        int m_spawnerGridSize = 3;
        
        std::vector<EntityId> m_spawnerEntities;
        
        // Performance tracking
        PhaseStats m_currentPhaseStats;
        std::vector<PhaseStats> m_phaseResults;
        std::string m_reportFilename;
        
        EventBus::SubscriberId m_toggleSubId;
    };

} // namespace rtype::ecs
