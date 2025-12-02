/*
** R-Type ECS - SpinComponent
** Component for bullets that rotate on themselves
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"
#include <cstdlib>
#include <cmath>

namespace rtype::ecs {

    /**
     * @brief Component for continuous rotation animation
     * 
     * Used for bullets (like Ball type) that spin on themselves.
     * Supports variable speed with optional acceleration/deceleration.
     */
    struct SpinComponent : public IComponent {
        float spinSpeed = 180.0f;        // Degrees per second
        float targetSpinSpeed = 180.0f;  // Target speed for interpolation
        float spinAcceleration = 0.0f;   // How fast to reach target speed (0 = instant)
        float currentRotation = 0.0f;    // Accumulated rotation
        
        SpinComponent() = default;
        
        std::string getTypeName() const override { return "SpinComponent"; }
        
        /**
         * @brief Create a spin component with random direction and speed
         * @param minSpeed Minimum spin speed (degrees/sec)
         * @param maxSpeed Maximum spin speed (degrees/sec)
         * @param accelChance Chance of having acceleration (0.0 to 1.0)
         */
        static SpinComponent randomSpin(float minSpeed = 90.0f, float maxSpeed = 360.0f, 
                                        float accelChance = 0.3f) {
            SpinComponent spin;
            
            float direction = (std::rand() % 2 == 0) ? 1.0f : -1.0f;
            
            float speedRange = maxSpeed - minSpeed;
            float randomSpeed = minSpeed + (std::rand() % 1000) / 1000.0f * speedRange;
            spin.spinSpeed = randomSpeed * direction;
            spin.targetSpinSpeed = spin.spinSpeed;
            
            if ((std::rand() % 100) / 100.0f < accelChance) {
                float newTarget = minSpeed + (std::rand() % 1000) / 1000.0f * speedRange;
                if (std::rand() % 3 == 0) {
                    newTarget *= -1.0f; // Sometimes reverse direction
                } else {
                    newTarget *= direction; // Keep same direction
                }
                spin.targetSpinSpeed = newTarget;
                spin.spinAcceleration = 50.0f + (std::rand() % 150);
            }
            
            return spin;
        }
        
        /**
         * @brief Update spin rotation
         * @param dt Delta time
         * @return Current rotation to apply
         */
        float update(float dt) {
            if (spinAcceleration > 0.0f) {
                float diff = targetSpinSpeed - spinSpeed;
                if (std::abs(diff) > 0.1f) {
                    float maxChange = spinAcceleration * dt;
                    if (std::abs(diff) <= maxChange) {
                        spinSpeed = targetSpinSpeed;
                    } else {
                        spinSpeed += (diff > 0 ? maxChange : -maxChange);
                    }
                }
            }
            
            currentRotation += spinSpeed * dt;
            
            while (currentRotation > 360.0f) currentRotation -= 360.0f;
            while (currentRotation < -360.0f) currentRotation += 360.0f;
            
            return currentRotation;
        }
    };

} // namespace rtype::ecs
