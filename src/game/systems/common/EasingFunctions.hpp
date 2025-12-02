/*
** R-Type ECS - EasingFunctions
** Common easing curves for smooth trajectory transitions
*/

#pragma once

#include <cmath>

namespace rtype::ecs {

    /**
     * @brief Collection of easing functions for smooth transitions
     * 
     * All functions take t in range [0, 1] and return value in [0, 1]
     * (or slightly beyond for elastic/bounce effects)
     */
    class Easing {
    public:
        // ============== Linear ==============
        static float linear(float t);

        // ============== Quadratic ==============
        static float easeInQuad(float t);
        static float easeOutQuad(float t);
        static float easeInOutQuad(float t);

        // ============== Cubic ==============
        static float easeInCubic(float t);
        static float easeOutCubic(float t);
        static float easeInOutCubic(float t);

        // ============== Quartic ==============
        static float easeInQuart(float t);
        static float easeOutQuart(float t);
        static float easeInOutQuart(float t);

        // ============== Sine ==============
        static float easeInSine(float t);
        static float easeOutSine(float t);
        static float easeInOutSine(float t);

        // ============== Exponential ==============
        static float easeInExpo(float t);
        static float easeOutExpo(float t);
        static float easeInOutExpo(float t);

        // ============== Circular ==============
        static float easeInCirc(float t);
        static float easeOutCirc(float t);
        static float easeInOutCirc(float t);

        // ============== Back (overshoot) ==============
        static float easeInBack(float t);
        static float easeOutBack(float t);
        static float easeInOutBack(float t);

        // ============== Elastic ==============
        static float easeInElastic(float t);
        static float easeOutElastic(float t);
        static float easeInOutElastic(float t);

        // ============== Bounce ==============
        static float easeOutBounce(float t);
        static float easeInBounce(float t);
        static float easeInOutBounce(float t);

        // ============== Utility ==============
        
        /**
         * @brief Apply easing to interpolate between two values
         * @param start Start value
         * @param end End value
         * @param t Progress (0 to 1)
         * @param easingFunc Easing function to use
         */
        template<typename EaseFunc>
        static float lerp(float start, float end, float t, EaseFunc easingFunc) {
            return start + (end - start) * easingFunc(t);
        }

        /**
         * @brief Clamp t to [0, 1] range before applying easing
         */
        static float clamp01(float t);

    private:
        static constexpr float M_PI_F = 3.14159265358979323846f;
    };

} // namespace rtype::ecs

