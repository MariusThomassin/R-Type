/*
** R-Type ECS - Spawner Factory
** Factory methods for creating common spawner configurations
*/

#pragma once

#include "PatternSpawnerComponent.hpp"
#include "BulletPatternComponent.hpp"
#include <vector>

namespace rtype::ecs {

    /**
     * @brief Factory for creating PatternSpawnerComponent instances
     */
    struct SpawnerFactory {
        /**
         * @brief Create a simple single-pattern spawner
         */
        static PatternSpawnerComponent createSimple(const BulletPatternComponent& pattern) {
            PatternSpawnerComponent spawner;
            spawner.addPattern(pattern);
            spawner.loopPatterns = true;
            return spawner;
        }

        /**
         * @brief Create a boss with multiple phase patterns
         */
        static PatternSpawnerComponent createBoss(const std::vector<BulletPatternComponent>& phasePatterns) {
            PatternSpawnerComponent spawner;
            for (size_t i = 0; i < phasePatterns.size(); ++i) {
                spawner.addPattern(phasePatterns[i]);
                spawner.phasePatterns.push_back(static_cast<int>(i));
            }
            spawner.loopPatterns = false;
            return spawner;
        }

        /**
         * @brief Create a turret that cycles through patterns
         */
        static PatternSpawnerComponent createTurret(const std::vector<BulletPatternComponent>& patterns,
                                                      float cycleDelay = 2.0f) {
            PatternSpawnerComponent spawner;
            for (const auto& p : patterns) {
                spawner.addPattern(p, cycleDelay);
            }
            spawner.loopPatterns = true;
            return spawner;
        }

        /**
         * @brief Create a spawner with random pattern selection
         */
        static PatternSpawnerComponent createRandom(const std::vector<BulletPatternComponent>& patterns) {
            PatternSpawnerComponent spawner;
            for (const auto& p : patterns) {
                spawner.addPattern(p);
            }
            spawner.loopPatterns = true;
            return spawner;
        }

        /**
         * @brief Create a delayed spawner (for stage scripting)
         */
        static PatternSpawnerComponent createDelayed(const BulletPatternComponent& pattern, float delay) {
            PatternSpawnerComponent spawner;
            spawner.addPattern(pattern, delay);
            spawner.autoStart = true;
            spawner.loopPatterns = false;
            return spawner;
        }
    };

} // namespace rtype::ecs
