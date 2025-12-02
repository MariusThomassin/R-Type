/*
** R-Type ECS - Pattern State Manager
** Handles spawner state transitions and wave progression
*/

#pragma once

#include "../../components/patterns/PatternSpawnerComponent.hpp"
#include "../../components/patterns/BulletPatternComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Manages pattern spawner state and wave progression
     */
    struct PatternStateManager {

        /**
         * @brief Advance to the next wave in pattern
         */
        static void advanceWave(PatternSpawnerComponent& spawner, BulletPatternComponent& pattern) {
            spawner.currentWaveIndex++;
            spawner.waveTimer = 0.0f;
            spawner.burstBulletsSpawned = 0;
            spawner.burstTimer = 0.0f;

            if (pattern.parallelWaves) {
                spawner.currentWaveIndex = static_cast<int>(pattern.waves.size());
            }
        }

        /**
         * @brief Handle pattern completion
         */
        static void handlePatternComplete(PatternSpawnerComponent& spawner) {
            spawner.waveRepeatCount++;

            BulletPatternComponent* pattern = spawner.getActivePattern();
            if (!pattern) return;

            if (pattern->repeatCount > 0 && spawner.waveRepeatCount >= pattern->repeatCount) {
                if (spawner.loopPatterns && spawner.patterns.size() > 1) {
                    spawner.nextPattern();
                    spawner.state = SpawnerState::Cooldown;
                    spawner.stateTimer = pattern->repeatDelay;
                } else {
                    spawner.state = SpawnerState::Finished;
                }
            } else {
                spawner.resetPattern();
                spawner.state = SpawnerState::Cooldown;
                spawner.stateTimer = 0.0f;
            }
        }

        /**
         * @brief Check if spawner should start
         */
        static bool shouldStart(const PatternSpawnerComponent& spawner) {
            return spawner.autoStart && 
                   spawner.state == SpawnerState::Idle && 
                   !spawner.patterns.empty();
        }

        /**
         * @brief Check if spawner is ready to process waves
         */
        static bool isReadyToProcess(const PatternSpawnerComponent& spawner) {
            return spawner.state == SpawnerState::Active;
        }
    };

} // namespace rtype::ecs
