/*
** R-Type ECS - FloatingTextComponent
** Component for floating score/text popups with animations
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include <string>

namespace rtype::ecs {

    /**
     * @brief Component for animated floating text (score popups, etc.)
     * 
     * Used to display points earned, damage numbers, or other
     * floating text with smooth animations.
     */
    struct FloatingTextComponent : public IComponent {
        std::string text;           // The text to display
        float startX = 0.0f;        // Starting X position
        float startY = 0.0f;        // Starting Y position
        float lifetime = 1.2f;      // Total lifetime in seconds
        float elapsed = 0.0f;       // Time elapsed since spawn
        
        // Animation parameters
        float floatSpeed = 40.0f;   // Upward float speed
        float fadeStart = 0.6f;     // When to start fading (percentage of lifetime)
        float scaleStart = 0.8f;    // Initial scale (for pop-in effect)
        float scaleMax = 1.2f;      // Max scale during pop
        float scalePeak = 0.15f;    // When scale peaks (percentage of lifetime)
        
        // Color
        uint8_t r = 255, g = 255, b = 100, a = 255;  // Default: yellow-ish
        
        int fontSize = 20;          // Base font size

        FloatingTextComponent() = default;

        FloatingTextComponent(const std::string& txt, float x, float y, int points = 0)
            : text(txt), startX(x), startY(y) {
            // Color based on point value
            if (points >= 1000) {
                // Gold for big points
                r = 255; g = 215; b = 0;
                fontSize = 24;
            } else if (points >= 500) {
                // Orange for medium
                r = 255; g = 180; b = 50;
                fontSize = 22;
            } else if (points >= 200) {
                // Yellow for small
                r = 255; g = 255; b = 100;
                fontSize = 20;
            } else {
                // White for tiny
                r = 220; g = 220; b = 220;
                fontSize = 18;
            }
        }

        /**
         * @brief Get current Y position (floats upward)
         */
        float getCurrentY() const {
            return startY - (elapsed * floatSpeed);
        }

        /**
         * @brief Get current alpha (fades out near end)
         */
        float getAlpha() const {
            float lifePercent = elapsed / lifetime;
            if (lifePercent < fadeStart) {
                return 1.0f;
            }
            // Smooth fade out
            float fadeProgress = (lifePercent - fadeStart) / (1.0f - fadeStart);
            return 1.0f - (fadeProgress * fadeProgress);  // Ease out
        }

        /**
         * @brief Get current scale (pop-in effect)
         */
        float getScale() const {
            float lifePercent = elapsed / lifetime;
            if (lifePercent < scalePeak) {
                // Growing phase
                float t = lifePercent / scalePeak;
                return scaleStart + (scaleMax - scaleStart) * t;
            } else if (lifePercent < scalePeak * 2) {
                // Shrinking back to 1.0
                float t = (lifePercent - scalePeak) / scalePeak;
                return scaleMax - (scaleMax - 1.0f) * t;
            }
            return 1.0f;
        }

        /**
         * @brief Check if text should be removed
         */
        bool isExpired() const {
            return elapsed >= lifetime;
        }

        std::string getTypeName() const override {
            return "FloatingTextComponent";
        }
    };

} // namespace rtype::ecs
