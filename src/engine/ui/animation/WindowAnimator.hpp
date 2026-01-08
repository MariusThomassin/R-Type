/*
** R-Type Engine - WindowAnimator
** Provides smooth animations for window operations
*/

#ifndef WINDOWANIMATOR_HPP_
#define WINDOWANIMATOR_HPP_

#include "../widgets/WindowWidget.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace rtype::ui {

    /**
     * @brief Animation types supported by the WindowAnimator
     */
    enum class AnimationType {
        Open,       // Window appearing
        Close,      // Window disappearing
        Collapse,   // Window collapsing to title bar
        Expand,     // Window expanding from title bar
        Move,       // Window position change
        Resize,     // Window size change
        FadeIn,     // Opacity increase
        FadeOut     // Opacity decrease
    };

    /**
     * @brief Easing function types for animations
     */
    enum class EasingType {
        Linear,
        EaseIn,
        EaseOut,
        EaseInOut,
        EaseOutQuad,
        EaseInOutCubic,
        EaseOutBounce,
        EaseOutElastic
    };

    /**
     * @brief Represents an active animation on a window
     */
    struct WindowAnimation {
        std::weak_ptr<WindowWidget> window;
        AnimationType type;
        float elapsed = 0.0f;
        float duration = 0.3f;
        EasingType easing = EasingType::EaseOutQuad;
        
        // Start and end values
        float startX = 0, startY = 0, startW = 0, startH = 0;
        float endX = 0, endY = 0, endW = 0, endH = 0;
        float startAlpha = 1.0f, endAlpha = 1.0f;
        
        // Completion callback
        std::function<void()> onComplete;
        
        bool isComplete() const { return elapsed >= duration; }
    };

    /**
     * @brief Manages window animations
     * 
     * Provides smooth animations for window operations such as
     * opening, closing, moving, resizing, and collapsing.
     */
    class WindowAnimator {
    public:
        WindowAnimator() = default;
        ~WindowAnimator() = default;

        /**
         * @brief Start an animation on a window
         * @param window The window to animate
         * @param type Animation type
         * @param duration Animation duration in seconds
         * @param easing Easing function to use
         * @param onComplete Optional callback when animation completes
         */
        void animate(
            std::shared_ptr<WindowWidget> window,
            AnimationType type,
            float duration = 0.3f,
            EasingType easing = EasingType::EaseOutQuad,
            std::function<void()> onComplete = nullptr
        );

        /**
         * @brief Animate window move to new position
         * @param window The window to animate
         * @param targetX Target X position
         * @param targetY Target Y position
         * @param duration Animation duration
         * @param easing Easing function
         */
        void animateMove(
            std::shared_ptr<WindowWidget> window,
            float targetX, float targetY,
            float duration = 0.3f,
            EasingType easing = EasingType::EaseOutQuad
        );

        /**
         * @brief Animate window resize
         * @param window The window to animate
         * @param targetWidth Target width
         * @param targetHeight Target height
         * @param duration Animation duration
         * @param easing Easing function
         */
        void animateResize(
            std::shared_ptr<WindowWidget> window,
            float targetWidth, float targetHeight,
            float duration = 0.3f,
            EasingType easing = EasingType::EaseOutQuad
        );

        /**
         * @brief Update all active animations
         * @param dt Delta time since last frame
         */
        void update(float dt);

        /**
         * @brief Check if a window is currently being animated
         * @param window The window to check
         * @return true if window has an active animation
         */
        bool isAnimating(const std::shared_ptr<WindowWidget>& window) const;

        /**
         * @brief Cancel all animations on a window
         * @param window The window to stop animating
         */
        void cancelAnimations(const std::shared_ptr<WindowWidget>& window);

        /**
         * @brief Cancel all active animations
         */
        void cancelAll();

        /**
         * @brief Get the number of active animations
         * @return Count of active animations
         */
        size_t getActiveAnimationCount() const { return _animations.size(); }

    private:
        std::vector<WindowAnimation> _animations;

        /**
         * @brief Apply easing function to a progress value
         * @param t Progress (0.0 to 1.0)
         * @param easing Easing type
         * @return Eased progress value
         */
        float applyEasing(float t, EasingType easing) const;

        /**
         * @brief Linear interpolation
         * @param start Start value
         * @param end End value
         * @param t Progress (0.0 to 1.0)
         * @return Interpolated value
         */
        float lerp(float start, float end, float t) const {
            return start + (end - start) * t;
        }
    };

} // namespace rtype::ui

#endif // WINDOWANIMATOR_HPP_
