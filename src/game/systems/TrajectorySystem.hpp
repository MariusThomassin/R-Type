/*
** R-Type ECS - TrajectorySystem
** Updates bullet trajectories for non-linear movement patterns
*/

#pragma once

#include "../../engine/ecs/core/ISystem.hpp"
#include "../../engine/ecs/core/Registry.hpp"
#include "../../engine/ecs/components/TransformComponent.hpp"
#include "../../engine/ecs/components/VelocityComponent.hpp"
#include "../components/bullets/TrajectoryComponent.hpp"
#include "../components/PlayerComponent.hpp"
#include "../components/SpritesheetComponent.hpp"
#include "../components/BulletTypes.hpp"
#include "common/GameMath.hpp"
#include "common/TargetTracker.hpp"
#include "common/TrajectoryUpdaters.hpp"

namespace rtype::ecs {

    /**
     * @brief System that updates trajectory-based bullet movement
     *
     * Runs during Physics phase, modifying velocity based on trajectory type.
     * Works with MovementSystem which applies the final velocity.
     */
    class TrajectorySystem : public ISystem {
    public:
        TrajectorySystem() = default;
        ~TrajectorySystem() override = default;

        void update(float dt) override {
            if (!m_registry) return;

            m_playerTracker.update(*m_registry);

            m_registry->forEach<TrajectoryComponent, TransformComponent, VelocityComponent>(
                [this, dt](EntityId entity) {
                    auto& trajectory = m_registry->getComponent<TrajectoryComponent>(entity);
                    auto& transform = m_registry->getComponent<TransformComponent>(entity);
                    auto& velocity = m_registry->getComponent<VelocityComponent>(entity);

                    if (!trajectory.initialized) {
                        TrajectoryUpdaters::initializeTrajectory(trajectory, transform, velocity);
                    }

                    if (trajectory.delay > 0.0f && trajectory.elapsedTime < trajectory.delay) {
                        trajectory.elapsedTime += dt;
                        return;
                    }

                    trajectory.elapsedTime += dt;

                    updateTrajectory(trajectory, transform, velocity, dt);
                    
                    updateBulletRotation(entity, transform, velocity);
                }
            );
        }

        SystemPhase getPhase() const override {
            return SystemPhase::Physics;
        }

    private:
        void updateTrajectory(TrajectoryComponent& trajectory,
                              TransformComponent& transform,
                              VelocityComponent& velocity,
                              float dt) {
            switch (trajectory.type) {
                case TrajectoryType::Homing:
                    TrajectoryUpdaters::updateHoming(trajectory, transform, velocity,
                                                     m_playerTracker.getPlayerX(),
                                                     m_playerTracker.getPlayerY(),
                                                     m_playerTracker.getPlayerVelX(),
                                                     m_playerTracker.getPlayerVelY(),
                                                     dt);
                    break;

                case TrajectoryType::Sinusoidal:
                    TrajectoryUpdaters::updateSinusoidal(trajectory, velocity, dt);
                    break;

                case TrajectoryType::Bezier:
                    TrajectoryUpdaters::updateBezier(trajectory, transform, velocity, dt);
                    break;

                case TrajectoryType::Circular:
                    TrajectoryUpdaters::updateCircular(trajectory, transform, velocity, dt);
                    break;

                case TrajectoryType::Accelerating:
                    TrajectoryUpdaters::updateAccelerating(trajectory, velocity, dt);
                    break;

                case TrajectoryType::Aimed:
                    TrajectoryUpdaters::updateAimed(trajectory, transform, velocity,
                                                    m_playerTracker.getPlayerX(),
                                                    m_playerTracker.getPlayerY());
                    break;

                case TrajectoryType::Boomerang:
                    TrajectoryUpdaters::updateBoomerang(trajectory, transform, velocity);
                    break;

                case TrajectoryType::Spiral:
                    TrajectoryUpdaters::updateSpiral(trajectory, transform, velocity, dt);
                    break;

                case TrajectoryType::Random:
                    TrajectoryUpdaters::updateRandom(trajectory, velocity, dt);
                    break;
                    
                // New trajectory types
                case TrajectoryType::Zigzag:
                    TrajectoryUpdaters::updateZigzag(trajectory, velocity, dt);
                    break;
                    
                case TrajectoryType::Figure8:
                    TrajectoryUpdaters::updateFigure8(trajectory, transform, velocity, dt);
                    break;
                    
                case TrajectoryType::DelayedHoming:
                    TrajectoryUpdaters::updateDelayedHoming(trajectory, transform, velocity,
                                                            m_playerTracker.getPlayerX(),
                                                            m_playerTracker.getPlayerY(),
                                                            m_playerTracker.getPlayerVelX(),
                                                            m_playerTracker.getPlayerVelY(),
                                                            dt);
                    break;
                    
                case TrajectoryType::Pendulum:
                    TrajectoryUpdaters::updatePendulum(trajectory, velocity, dt);
                    break;
                    
                case TrajectoryType::SpiralInward:
                    TrajectoryUpdaters::updateSpiralInward(trajectory, transform, velocity, dt);
                    break;
                    
                case TrajectoryType::Whip:
                    TrajectoryUpdaters::updateWhip(trajectory, velocity, dt);
                    break;
                    
                case TrajectoryType::Wobble:
                    TrajectoryUpdaters::updateWobble(trajectory, velocity, dt);
                    break;

                case TrajectoryType::Linear:
                default:
                    // No modification needed
                    break;
            }
        }
        
        /**
         * @brief Updates bullet rotation based on velocity for directional bullet types
         * 
         * Rice, Ball, and Dot bullets rotate to face their direction of travel.
         */
        void updateBulletRotation(EntityId entity, TransformComponent& transform, 
                                  const VelocityComponent& velocity) {
            if (!m_registry->hasComponent<SpritesheetComponent>(entity)) return;
            
            const auto& sprite = m_registry->getComponent<SpritesheetComponent>(entity);
            if (!sprite.usesBulletMapping()) return;
            
            BulletType type = sprite.getBulletType();
            
            if (type == BulletType::Rice || 
                type == BulletType::Dot) {
                transform.rotation = std::atan2(velocity.vy, velocity.vx) * (180.0f / GameMath::M_PI_F) - 90.0f;
            }
        }

    private:
        PlayerTracker m_playerTracker;
    };

} // namespace rtype::ecs
