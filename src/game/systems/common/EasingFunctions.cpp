/*
** R-Type ECS - EasingFunctions
** Implementation of easing curves for smooth trajectory transitions
*/

#include "EasingFunctions.hpp"

namespace rtype::ecs {

    // ============== Linear ==============
    float Easing::linear(float t) {
        return t;
    }

    // ============== Quadratic ==============
    float Easing::easeInQuad(float t) {
        return t * t;
    }

    float Easing::easeOutQuad(float t) {
        return t * (2.0f - t);
    }

    float Easing::easeInOutQuad(float t) {
        return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
    }

    // ============== Cubic ==============
    float Easing::easeInCubic(float t) {
        return t * t * t;
    }

    float Easing::easeOutCubic(float t) {
        float f = t - 1.0f;
        return f * f * f + 1.0f;
    }

    float Easing::easeInOutCubic(float t) {
        return t < 0.5f ? 4.0f * t * t * t : (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f) + 1.0f;
    }

    // ============== Quartic ==============
    float Easing::easeInQuart(float t) {
        return t * t * t * t;
    }

    float Easing::easeOutQuart(float t) {
        float f = t - 1.0f;
        return 1.0f - f * f * f * f;
    }

    float Easing::easeInOutQuart(float t) {
        if (t < 0.5f) return 8.0f * t * t * t * t;
        float f = t - 1.0f;
        return 1.0f - 8.0f * f * f * f * f;
    }

    // ============== Sine ==============
    float Easing::easeInSine(float t) {
        return 1.0f - std::cos(t * M_PI_F * 0.5f);
    }

    float Easing::easeOutSine(float t) {
        return std::sin(t * M_PI_F * 0.5f);
    }

    float Easing::easeInOutSine(float t) {
        return 0.5f * (1.0f - std::cos(M_PI_F * t));
    }

    // ============== Exponential ==============
    float Easing::easeInExpo(float t) {
        return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * (t - 1.0f));
    }

    float Easing::easeOutExpo(float t) {
        return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t);
    }

    float Easing::easeInOutExpo(float t) {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        if (t < 0.5f) return 0.5f * std::pow(2.0f, 20.0f * t - 10.0f);
        return 1.0f - 0.5f * std::pow(2.0f, -20.0f * t + 10.0f);
    }

    // ============== Circular ==============
    float Easing::easeInCirc(float t) {
        return 1.0f - std::sqrt(1.0f - t * t);
    }

    float Easing::easeOutCirc(float t) {
        float f = t - 1.0f;
        return std::sqrt(1.0f - f * f);
    }

    float Easing::easeInOutCirc(float t) {
        if (t < 0.5f) return 0.5f * (1.0f - std::sqrt(1.0f - 4.0f * t * t));
        float f = 2.0f * t - 2.0f;
        return 0.5f * (std::sqrt(1.0f - f * f) + 1.0f);
    }

    // ============== Back (overshoot) ==============
    float Easing::easeInBack(float t) {
        constexpr float c = 1.70158f;
        return t * t * ((c + 1.0f) * t - c);
    }

    float Easing::easeOutBack(float t) {
        constexpr float c = 1.70158f;
        float f = t - 1.0f;
        return f * f * ((c + 1.0f) * f + c) + 1.0f;
    }

    float Easing::easeInOutBack(float t) {
        constexpr float c = 1.70158f * 1.525f;
        if (t < 0.5f) {
            return 0.5f * (4.0f * t * t * ((c + 1.0f) * 2.0f * t - c));
        }
        float f = 2.0f * t - 2.0f;
        return 0.5f * (f * f * ((c + 1.0f) * f + c) + 2.0f);
    }

    // ============== Elastic ==============
    float Easing::easeInElastic(float t) {
        if (t == 0.0f || t == 1.0f) return t;
        return -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * (2.0f * M_PI_F / 3.0f));
    }

    float Easing::easeOutElastic(float t) {
        if (t == 0.0f || t == 1.0f) return t;
        return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * (2.0f * M_PI_F / 3.0f)) + 1.0f;
    }

    float Easing::easeInOutElastic(float t) {
        if (t == 0.0f || t == 1.0f) return t;
        if (t < 0.5f) {
            return -0.5f * std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * (2.0f * M_PI_F / 4.5f));
        }
        return 0.5f * std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * (2.0f * M_PI_F / 4.5f)) + 1.0f;
    }

    // ============== Bounce ==============
    float Easing::easeOutBounce(float t) {
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

    float Easing::easeInBounce(float t) {
        return 1.0f - easeOutBounce(1.0f - t);
    }

    float Easing::easeInOutBounce(float t) {
        if (t < 0.5f) return 0.5f * easeInBounce(t * 2.0f);
        return 0.5f * easeOutBounce(t * 2.0f - 1.0f) + 0.5f;
    }

    // ============== Utility ==============
    float Easing::clamp01(float t) {
        return t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
    }

} // namespace rtype::ecs
