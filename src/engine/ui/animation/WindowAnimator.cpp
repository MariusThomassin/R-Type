/*
** R-Type Engine - WindowAnimator Implementation
** Provides smooth animations for window operations
*/

#include "WindowAnimator.hpp"
#include <algorithm>
#include <cmath>

namespace rtype::ui {

    void WindowAnimator::animate(
        std::shared_ptr<WindowWidget> window,
        AnimationType type,
        float duration,
        EasingType easing,
        std::function<void()> onComplete
    ) {
        if (!window) return;
        
        // Cancel any existing animation of the same type
        cancelAnimations(window);
        
        WindowAnimation anim;
        anim.window = window;
        anim.type = type;
        anim.duration = duration;
        anim.easing = easing;
        anim.onComplete = onComplete;
        
        auto transform = window->getAbsoluteTransform();
        anim.startX = transform.x;
        anim.startY = transform.y;
        anim.startW = transform.width;
        anim.startH = transform.height;
        anim.endX = transform.x;
        anim.endY = transform.y;
        anim.endW = transform.width;
        anim.endH = transform.height;
        
        switch (type) {
            case AnimationType::Open:
                // Start from center, expand out
                anim.startX = transform.x + transform.width / 2;
                anim.startY = transform.y + transform.height / 2;
                anim.startW = 0;
                anim.startH = 0;
                anim.startAlpha = 0;
                anim.endAlpha = 1;
                break;
                
            case AnimationType::Close:
                // Collapse to center
                anim.endX = transform.x + transform.width / 2;
                anim.endY = transform.y + transform.height / 2;
                anim.endW = 0;
                anim.endH = 0;
                anim.startAlpha = 1;
                anim.endAlpha = 0;
                break;
                
            case AnimationType::Collapse:
                // Keep position, reduce height to title bar
                anim.endH = WindowWidget::TITLE_BAR_HEIGHT;
                break;
                
            case AnimationType::Expand:
                // Keep position, restore height (stored externally)
                // For now, use a default expansion
                anim.startH = WindowWidget::TITLE_BAR_HEIGHT;
                break;
                
            case AnimationType::FadeIn:
                anim.startAlpha = 0;
                anim.endAlpha = 1;
                break;
                
            case AnimationType::FadeOut:
                anim.startAlpha = 1;
                anim.endAlpha = 0;
                break;
                
            default:
                break;
        }
        
        _animations.push_back(anim);
    }

    void WindowAnimator::animateMove(
        std::shared_ptr<WindowWidget> window,
        float targetX, float targetY,
        float duration,
        EasingType easing
    ) {
        if (!window) return;
        
        // Cancel any existing move animation
        cancelAnimations(window);
        
        WindowAnimation anim;
        anim.window = window;
        anim.type = AnimationType::Move;
        anim.duration = duration;
        anim.easing = easing;
        
        auto transform = window->getAbsoluteTransform();
        anim.startX = transform.x;
        anim.startY = transform.y;
        anim.startW = transform.width;
        anim.startH = transform.height;
        anim.endX = targetX;
        anim.endY = targetY;
        anim.endW = transform.width;
        anim.endH = transform.height;
        
        _animations.push_back(anim);
    }

    void WindowAnimator::animateResize(
        std::shared_ptr<WindowWidget> window,
        float targetWidth, float targetHeight,
        float duration,
        EasingType easing
    ) {
        if (!window) return;
        
        // Cancel any existing resize animation
        cancelAnimations(window);
        
        WindowAnimation anim;
        anim.window = window;
        anim.type = AnimationType::Resize;
        anim.duration = duration;
        anim.easing = easing;
        
        auto transform = window->getAbsoluteTransform();
        anim.startX = transform.x;
        anim.startY = transform.y;
        anim.startW = transform.width;
        anim.startH = transform.height;
        anim.endX = transform.x;
        anim.endY = transform.y;
        anim.endW = targetWidth;
        anim.endH = targetHeight;
        
        _animations.push_back(anim);
    }

    void WindowAnimator::update(float dt) {
        // Update all animations and remove completed ones
        auto it = _animations.begin();
        while (it != _animations.end()) {
            auto window = it->window.lock();
            if (!window) {
                // Window was destroyed, remove animation
                it = _animations.erase(it);
                continue;
            }
            
            it->elapsed += dt;
            float t = std::min(it->elapsed / it->duration, 1.0f);
            float easedT = applyEasing(t, it->easing);
            
            // Apply interpolated values
            float newX = lerp(it->startX, it->endX, easedT);
            float newY = lerp(it->startY, it->endY, easedT);
            float newW = lerp(it->startW, it->endW, easedT);
            float newH = lerp(it->startH, it->endH, easedT);
            
            window->setPosition(newX, newY);
            window->setSize(newW, newH);
            
            // Check if animation is complete
            if (it->isComplete()) {
                // Ensure final values are set exactly
                window->setPosition(it->endX, it->endY);
                window->setSize(it->endW, it->endH);
                
                // Call completion callback
                if (it->onComplete) {
                    it->onComplete();
                }
                
                // Handle special end states
                if (it->type == AnimationType::Close) {
                    window->setVisible(false);
                }
                
                it = _animations.erase(it);
            } else {
                ++it;
            }
        }
    }

    bool WindowAnimator::isAnimating(const std::shared_ptr<WindowWidget>& window) const {
        if (!window) return false;
        
        for (const auto& anim : _animations) {
            if (anim.window.lock() == window) {
                return true;
            }
        }
        return false;
    }

    void WindowAnimator::cancelAnimations(const std::shared_ptr<WindowWidget>& window) {
        if (!window) return;
        
        _animations.erase(
            std::remove_if(_animations.begin(), _animations.end(),
                [&window](const WindowAnimation& anim) {
                    return anim.window.lock() == window;
                }
            ),
            _animations.end()
        );
    }

    void WindowAnimator::cancelAll() {
        _animations.clear();
    }

    float WindowAnimator::applyEasing(float t, EasingType easing) const {
        // Clamp t to [0, 1]
        t = std::max(0.0f, std::min(1.0f, t));
        
        switch (easing) {
            case EasingType::Linear:
                return t;
                
            case EasingType::EaseIn:
                return t * t;
                
            case EasingType::EaseOut:
                return 1 - (1 - t) * (1 - t);
                
            case EasingType::EaseInOut:
                return t < 0.5f ? 2 * t * t : 1 - std::pow(-2 * t + 2, 2) / 2;
                
            case EasingType::EaseOutQuad:
                return 1 - (1 - t) * (1 - t);
                
            case EasingType::EaseInOutCubic:
                return t < 0.5f ? 4 * t * t * t : 1 - std::pow(-2 * t + 2, 3) / 2;
                
            case EasingType::EaseOutBounce: {
                const float n1 = 7.5625f;
                const float d1 = 2.75f;
                
                if (t < 1 / d1) {
                    return n1 * t * t;
                } else if (t < 2 / d1) {
                    t -= 1.5f / d1;
                    return n1 * t * t + 0.75f;
                } else if (t < 2.5f / d1) {
                    t -= 2.25f / d1;
                    return n1 * t * t + 0.9375f;
                } else {
                    t -= 2.625f / d1;
                    return n1 * t * t + 0.984375f;
                }
            }
                
            case EasingType::EaseOutElastic: {
                if (t == 0 || t == 1) return t;
                float p = 0.3f;
                float s = p / 4;
                return std::pow(2, -10 * t) * std::sin((t - s) * (2 * 3.14159265f) / p) + 1;
            }
                
            default:
                return t;
        }
    }

} // namespace rtype::ui
