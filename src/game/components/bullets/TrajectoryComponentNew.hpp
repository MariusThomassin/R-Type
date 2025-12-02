/*
** R-Type ECS - TrajectoryComponentNew
** Refactored trajectory component using variant-based storage
*/

#pragma once

#include "../../../engine/ecs/core/IComponent.hpp"
#include "trajectories/Trajectories.hpp"
#include "TrajectoryTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Optimized trajectory component
     * 
     * Uses std::variant for type-specific data, dramatically reducing
     * memory footprint compared to storing all trajectory fields.
     * 
     * Memory comparison:
     * - Old TrajectoryComponent: ~400 bytes (all fields always present)
     * - New TrajectoryComponentNew: ~100 bytes max (variant + common state)
     */
    struct TrajectoryComponentNew : public IComponent {
        // Common state for all trajectories
        TrajectoryState state;

        // Delay before trajectory activates
        float activationDelay = 0.0f;

        // Modifier flags
        TrajectoryModifier modifiers = TrajectoryModifier::None;

        // Type-specific data (only stores active type)
        TrajectoryVariant data;

        // ==================== Constructors ====================

        TrajectoryComponentNew() : data(LinearTrajectory{}) {}

        template <typename T>
        explicit TrajectoryComponentNew(T&& trajectory)
            : data(std::forward<T>(trajectory)) {}

        // ==================== Type queries ====================

        /**
         * @brief Get the trajectory type
         */
        TrajectoryType getType() const {
            return getTrajectoryType(data);
        }

        /**
         * @brief Check if trajectory is a specific type
         */
        template <typename T>
        bool is() const {
            return std::holds_alternative<T>(data);
        }

        // ==================== Data access ====================

        /**
         * @brief Get trajectory data (throws if wrong type)
         */
        template <typename T>
        T& get() {
            return std::get<T>(data);
        }

        template <typename T>
        const T& get() const {
            return std::get<T>(data);
        }

        /**
         * @brief Try to get trajectory data (returns nullptr if wrong type)
         */
        template <typename T>
        T* tryGet() {
            return std::get_if<T>(&data);
        }

        template <typename T>
        const T* tryGet() const {
            return std::get_if<T>(&data);
        }

        // ==================== Fluent modifiers ====================

        TrajectoryComponentNew& withDelay(float delay) {
            activationDelay = delay;
            return *this;
        }

        TrajectoryComponentNew& withModifier(TrajectoryModifier mod) {
            modifiers = static_cast<TrajectoryModifier>(
                static_cast<unsigned int>(modifiers) | 
                static_cast<unsigned int>(mod)
            );
            return *this;
        }

        TrajectoryComponentNew& faceDirection() {
            return withModifier(TrajectoryModifier::FaceDirection);
        }

        TrajectoryComponentNew& easeIn() {
            return withModifier(TrajectoryModifier::EaseIn);
        }

        TrajectoryComponentNew& easeOut() {
            return withModifier(TrajectoryModifier::EaseOut);
        }

        // ==================== IComponent ====================

        std::string getTypeName() const override {
            return "TrajectoryComponentNew";
        }
    };

    // ==================== Convenience factory functions ====================

    namespace Trajectory {

        inline TrajectoryComponentNew linear() {
            return TrajectoryComponentNew(LinearTrajectory{});
        }

        inline TrajectoryComponentNew homing(EntityId target, float strength = 5.0f) {
            return TrajectoryComponentNew(HomingTrajectory(target, strength));
        }

        inline TrajectoryComponentNew delayedHoming(EntityId target, float delay, float strength = 5.0f) {
            return TrajectoryComponentNew(DelayedHomingTrajectory(target, delay, strength));
        }

        inline TrajectoryComponentNew sinusoidal(float amplitude = 200.0f, float frequency = 15.0f) {
            return TrajectoryComponentNew(SinusoidalTrajectory(amplitude, frequency));
        }

        inline TrajectoryComponentNew wobble(float intensity = 20.0f, float speed = 10.0f) {
            return TrajectoryComponentNew(WobbleTrajectory(intensity, speed));
        }

        inline TrajectoryComponentNew pendulum(float amplitude = 60.0f, float frequency = 2.0f) {
            return TrajectoryComponentNew(PendulumTrajectory(amplitude, frequency));
        }

        inline TrajectoryComponentNew bezier(float sx, float sy, float c1x, float c1y,
                                              float c2x, float c2y, float ex, float ey,
                                              float duration = 2.0f) {
            return TrajectoryComponentNew(BezierTrajectory(sx, sy, c1x, c1y, c2x, c2y, ex, ey, duration));
        }

        inline TrajectoryComponentNew figure8(float width = 150.0f, float height = 100.0f) {
            return TrajectoryComponentNew(Figure8Trajectory(width, height));
        }

        inline TrajectoryComponentNew circular(float cx, float cy, float radius, float angVel = 3.0f) {
            return TrajectoryComponentNew(CircularTrajectory(cx, cy, radius, angVel));
        }

        inline TrajectoryComponentNew spiral(float expansion = 50.0f, float tightness = 2.0f) {
            return TrajectoryComponentNew(SpiralTrajectory(expansion, tightness));
        }

        inline TrajectoryComponentNew spiralInward(float tx, float ty, float rate = 30.0f) {
            return TrajectoryComponentNew(SpiralInwardTrajectory(tx, ty, rate));
        }

        inline TrajectoryComponentNew accelerating(float targetSpeed, float accel) {
            return TrajectoryComponentNew(AcceleratingTrajectory(targetSpeed, accel));
        }

        inline TrajectoryComponentNew whip(float maxSpeed = 800.0f) {
            return TrajectoryComponentNew(WhipTrajectory(0.3f, maxSpeed));
        }

        inline TrajectoryComponentNew boomerang(float distance = 300.0f) {
            return TrajectoryComponentNew(BoomerangTrajectory(distance));
        }

        inline TrajectoryComponentNew random(float interval = 0.5f, float angleRange = 45.0f) {
            return TrajectoryComponentNew(RandomTrajectory(interval, angleRange));
        }

        inline TrajectoryComponentNew zigzag(float width = 100.0f, float length = 50.0f) {
            return TrajectoryComponentNew(ZigzagTrajectory(width, length));
        }

        inline TrajectoryComponentNew aimed(float delay = 0.0f) {
            return TrajectoryComponentNew(AimedTrajectory(delay));
        }

    } // namespace Trajectory

} // namespace rtype::ecs
