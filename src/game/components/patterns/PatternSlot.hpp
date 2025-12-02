/*
** R-Type ECS - PatternSlot
** Individual pattern slot with timing for spawners
*/

#pragma once

#include "BulletPatternComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Individual pattern slot with its own timing
     */
    struct PatternSlot {
        BulletPatternComponent pattern;
        float startDelay = 0.0f;       // Delay before pattern starts
        float priority = 0.0f;          // Higher = executes first when simultaneous
        bool enabled = true;            // Can be toggled at runtime

        PatternSlot() = default;
        
        explicit PatternSlot(const BulletPatternComponent& p, float delay = 0.0f)
            : pattern(p), startDelay(delay) {}

        PatternSlot& setDelay(float delay) { startDelay = delay; return *this; }
        PatternSlot& setPriority(float p) { priority = p; return *this; }
        PatternSlot& setEnabled(bool e) { enabled = e; return *this; }
    };

} // namespace rtype::ecs
