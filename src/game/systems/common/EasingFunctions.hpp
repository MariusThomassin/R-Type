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
        static float linear(float t) {
            return t;
        }

        // ============== Quadratic ==============
        static float easeInQuad(float t) {
            return t * t;
        }

        static float easeOutQuad(float t) {
            return t * (2.0f - t);
        }

        static float easeInOutQuad(float t) {
            return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
        }

        // ============== Cubic ==============
        static float easeInCubic(float t) {
            return t * t * t;
        }

        static float easeOutCubic(float t) {
            float f = t - 1.0f;
            return f * f * f + 1.0f;
        }

        static float easeInOutCubic(float t) {
            return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
        }

        // ============== Quartic ==============
        static float easeInQuart(float t) {
            return t * t * t * t;
        }

        static float easeOutQuart(float t) {
            float f = t - 1.0f;
            return 1.0f - f * f * f * f;
        }

        static float easeInOutQuart(float t) {
            if (t < 0.5f) return 8.0f * t * t * t * t;
            float f = t - 1.0f;
            return 1.0f - 8.0f * f * f * f * f;
        }

        // ============== Sine ==============
        static float easeInSine(float t) {
            return 1.0f - std::cos(t * M_PI_F * 0.5f);
        }

        static float easeOutSine(float t) {
            return std::sin(t * M_PI_F * 0.5f);
        }

        static float easeInOutSine(float t) {
            return 0.5f * (1.0f - std::cos(M_PI_F * t));
        }

        // ============== Exponential ==============
        static float easeInExpo(float t) {
            return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f));
        }

        static float easeOutExpo(float t) {
            return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
        }

        static float easeInOutExpo(float t) {
            if (t == 0.0f) return 0.0f;
            if (t == 1.0f) return 1.0f;
            if (t < 0.5f) return 0.5f * std::pow(2.0f, 20.0f * t - 10.0f);
            return 1.0f - 0.5f * std::pow(2.0f, -20.0f * t + 10.0f);
        }

        // ============== Circular ==============
        static float easeInCirc(float t) {
            return 1.0f - std::sqrt(1.0f - t * t);
        }

        static float easeOutCirc(float t) {
            float f = t - 1.0f;
            return std::sqrt(1.0f - f * f);
        }

        static float easeInOutCirc(float t) {
            if (t < 0.5f) return 0.5f * (1.0f - std::sqrt(1.0f - 4.0f * t * t));
            float f = 2.0f * t - 2.0f;
            return 0.5f * (std::sqrt(1.0f - f * f) + 1.0f);
        }

        // ============== Back (overshoot) ==============
        static float easeInBack(float t) {
            constexpr float c = 1.70158f;
            return t * t * ((c + 1.0f) * t - c);
        }

        static float easeOutBack(float t) {
            constexpr float c = 1.70158f;
            float f = t - 1.0f;
            return f * f * ((c + 1.0f) * f + c) + 1.0f;
        }

        static float easeInOutBack(float t) {
            constexpr float c = 1.70158f * 1.525f;
            if (t < 0.5f) {
                return 0.5f * (4.0f * t * t * ((c + 1.0f) * 2.0f * t - c));
            }
            float f = 2.0f * t - 2.0f;
            return 0.5f * (f * f * ((c + 1.0f) * f + c) + 2.0f);
        }

        // ============== Elastic ==============
        static float easeInElastic(float t) {
            if (t == 0.0f || t == 1.0f) return t;
            return -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * (2.0f * M_PI_F / 3.0f));
        }

        static float easeOutElastic(float t) {
            if (t == 0.0f || t == 1.0f) return t;
            return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * (2.0f * M_PI_F / 3.0f)) + 1.0f;
        }

        static float easeInOutElastic(float t) {
            if (t == 0.0f || t == 1.0f) return t;
            if (t < 0.5f) {
                return -0.5f * std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * (2.0f * M_PI_F / 4.5f));
            }
            return 0.5f * std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * (2.0f * M_PI_F / 4.5f)) + 1.0f;
        }

        // ============== Bounce ==============
        static float easeOutBounce(float t) {
            if (t < 1.0f / 2.75f) {
                return 7.5625f * t * t;
            } else if (t < 2.0f / 2.75f) {
                float f = t - 1.5f / 2.75f;
                return 7.5625f * f * f + 0.75f;
            } else if (t < 2.5f / 2.75f) {
                float f = t - 2.25f / 2.75f;
                return 7.5625f * f * f + 0.9375f;
            } else {
                float f = t - 2.625f / 2.75f;
                return 7.5625f * f * f + 0.984375f;
            }
        }

        static float easeInBounce(float t) {
            return 1.0f - easeOutBounce(1.0f - t);
        }

        static float easeInOutBounce(float t) {
            if (t < 0.5f) return 0.5f * easeInBounce(t * 2.0f);
            return 0.5f * easeOutBounce(t * 2.0f - 1.0f) + 0.5f;
        }

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
        static float clamp01(float t) {
            return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
        }

    private:
        static constexpr float M_PI_F = 3.14159265358979323846f;
    };

} // namespace rtype::ecs
