/*
** R-Type ECS - BulletPatternComponent
** Complete bullet pattern definition
*/

#pragma once

#include "../../../engine/ecs/core/IComponent.hpp"
#include "PatternTypes.hpp"
#include "PatternWave.hpp"
#include <vector>
#include <string>

namespace rtype::ecs {

    /**
     * @brief Complete bullet pattern definition
     *
     * A pattern consists of one or more waves that execute in sequence or parallel.
     * Patterns can loop, have varying timing, and combine different bullet types.
     */
    struct BulletPatternComponent : public IComponent {
        std::string patternName;                // For debugging/identification
        std::vector<PatternWave> waves;         // Waves in this pattern

        // Timing
        float patternDuration = 0.0f;           // 0 = auto-calculate from waves
        float repeatDelay = 1.0f;               // Delay between pattern loops
        int repeatCount = 0;                    // 0 = infinite, >0 = limited repeats
        bool parallelWaves = false;             // Execute waves simultaneously

        // Global modifiers
        float globalAngleOffset = 0.0f;         // Added to all wave angles
        float rotationSpeed = 0.0f;             // Pattern rotation (degrees/sec)
        float globalSpeedMultiplier = 1.0f;     // Applied to all bullet speeds

        // Spawn offset from emitter
        float offsetX = 0.0f;
        float offsetY = 0.0f;

        BulletPatternComponent() = default;

        explicit BulletPatternComponent(const std::string& name)
            : patternName(name) {}

        std::string getTypeName() const override {
            return "BulletPatternComponent";
        }

        /**
         * @brief Add a wave to the pattern
         */
        BulletPatternComponent& addWave(const PatternWave& wave) {
            waves.push_back(wave);
            return *this;
        }

        BulletPatternComponent& setTiming(float duration, float delay, int repeats = 0) {
            patternDuration = duration;
            repeatDelay = delay;
            repeatCount = repeats;
            return *this;
        }

        BulletPatternComponent& setRotation(float speed) {
            rotationSpeed = speed;
            return *this;
        }

        BulletPatternComponent& setOffset(float x, float y) {
            offsetX = x;
            offsetY = y;
            return *this;
        }

        BulletPatternComponent& setParallel(bool parallel) {
            parallelWaves = parallel;
            return *this;
        }

        BulletPatternComponent& setSpeedMultiplier(float mult) {
            globalSpeedMultiplier = mult;
            return *this;
        }
    };

} // namespace rtype::ecs
