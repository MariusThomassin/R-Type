/*
** R-Type ECS - PatternSpawnerComponent
** Component for entities that spawn bullet patterns
*/

#pragma once

#include "../../../engine/ecs/core/IComponent.hpp"
#include "PatternTypes.hpp"
#include "PatternSlot.hpp"
#include <vector>

namespace rtype::ecs {

    /**
     * @brief Component for entities that spawn bullet patterns
     *
     * Attach to enemies, bosses, or dedicated spawner entities.
     * Supports multiple patterns, phase-based switching, and runtime control.
     */
    struct PatternSpawnerComponent : public IComponent {
        std::vector<PatternSlot> patterns;
        int activePatternIndex = 0;

        SpawnerState state = SpawnerState::Idle;
        float stateTimer = 0.0f;
        float patternTimer = 0.0f;

        int currentWaveIndex = 0;
        float waveTimer = 0.0f;
        int waveRepeatCount = 0;
        int burstBulletsSpawned = 0;
        float burstTimer = 0.0f;

        float currentRotation = 0.0f;

        float spawnOffsetX = 0.0f;
        float spawnOffsetY = 0.0f;
        bool useEntityRotation = true;
        bool autoStart = true;

        int currentPhase = 0;
        std::vector<int> phasePatterns;

        bool loopPatterns = true;
        int sequenceIndex = 0;

        PatternSpawnerComponent() = default;

        std::string getTypeName() const override {
            return "PatternSpawnerComponent";
        }

        // === Pattern management ===

        PatternSpawnerComponent& addPattern(const BulletPatternComponent& pattern, float startDelay = 0.0f) {
            patterns.emplace_back(pattern, startDelay);
            return *this;
        }

        BulletPatternComponent* getActivePattern() {
            if (activePatternIndex >= 0 && activePatternIndex < static_cast<int>(patterns.size())) {
                return &patterns[activePatternIndex].pattern;
            }
            return nullptr;
        }

        const BulletPatternComponent* getActivePattern() const {
            if (activePatternIndex >= 0 && activePatternIndex < static_cast<int>(patterns.size())) {
                return &patterns[activePatternIndex].pattern;
            }
            return nullptr;
        }

        PatternSlot* getActiveSlot() {
            if (activePatternIndex >= 0 && activePatternIndex < static_cast<int>(patterns.size())) {
                return &patterns[activePatternIndex];
            }
            return nullptr;
        }

        // === State control ===

        void start() {
            if (patterns.empty()) return;
            state = SpawnerState::Active;
            if (patternTimer == 0.0f) {
                resetPattern();
            }
        }

        void pause() {
            if (state == SpawnerState::Active) {
                state = SpawnerState::Paused;
            }
        }

        void stop() {
            state = SpawnerState::Idle;
            resetPattern();
        }

        void resetPattern() {
            patternTimer = 0.0f;
            currentWaveIndex = 0;
            waveTimer = 0.0f;
            waveRepeatCount = 0;
            burstBulletsSpawned = 0;
            burstTimer = 0.0f;
            currentRotation = 0.0f;
        }

        void nextPattern() {
            if (patterns.empty()) return;
            activePatternIndex = (activePatternIndex + 1) % static_cast<int>(patterns.size());
            resetPattern();
        }

        void setPattern(int index) {
            if (index >= 0 && index < static_cast<int>(patterns.size())) {
                activePatternIndex = index;
                resetPattern();
            }
        }

        PatternSpawnerComponent& setOffset(float x, float y) {
            spawnOffsetX = x;
            spawnOffsetY = y;
            return *this;
        }

        PatternSpawnerComponent& setAutoStart(bool auto_start) {
            autoStart = auto_start;
            return *this;
        }

        PatternSpawnerComponent& setLoop(bool loop) {
            loopPatterns = loop;
            return *this;
        }
    };

} // namespace rtype::ecs
