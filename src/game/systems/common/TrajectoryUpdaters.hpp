/*
** R-Type ECS - Trajectory Updaters
** Individual trajectory update functions for moving entities
** Used by bullets, effects, enemies, and other moving objects
*/

#pragma once

#include "../../../engine/ecs/components/TransformComponent.hpp"
#include "../../../engine/ecs/components/VelocityComponent.hpp"
#include "../../components/bullets/TrajectoryComponent.hpp"
#include "GameMath.hpp"
#include <cmath>
#include <cstdlib>

namespace rtype::ecs {

    /**
     * @brief Collection of trajectory update functions
     * Generic movement patterns that can be applied to any entity with
     * TransformComponent, VelocityComponent, and TrajectoryComponent
     */
    struct TrajectoryUpdaters {
        
        static void initializeTrajectory(TrajectoryComponent& traj,
                                         const TransformComponent& transform,
                                         const VelocityComponent& velocity) {
            traj.baseVelX = velocity.vx;
            traj.baseVelY = velocity.vy;
            traj.initialized = true;

            switch (traj.type) {
                case TrajectoryType::Circular:
                    traj.currentAngle = std::atan2(velocity.vy, velocity.vx);
                    break;
                case TrajectoryType::Boomerang:
                    traj.originX = transform.x;
                    traj.originY = transform.y;
                    traj.boomerangPhase = 0.0f;
                    break;
                case TrajectoryType::Bezier:
                    traj.startX = transform.x;
                    traj.startY = transform.y;
                    break;
                case TrajectoryType::Figure8:
                    traj.originX = transform.x;
                    traj.originY = transform.y;
                    break;
                case TrajectoryType::SpiralInward:
                    if (traj.spiralTargetX == 0.0f && traj.spiralTargetY == 0.0f) {
                        float speed = GameMath::speed(velocity.vx, velocity.vy);
                        float dist = 300.0f;
                        traj.spiralTargetX = transform.x + (velocity.vx / speed) * dist;
                        traj.spiralTargetY = transform.y + (velocity.vy / speed) * dist;
                    }
                    break;
                default:
                    break;
            }
        }

        static void updateHoming(TrajectoryComponent& traj,
                                 const TransformComponent& transform,
                                 VelocityComponent& velocity,
                                 float targetX, float targetY,
                                 float targetVelX, float targetVelY,
                                 float dt) {
            if (traj.homingDuration > 0.0f && traj.elapsedTime > traj.homingDuration + traj.delay) {
                return;
            }

            float aimX = targetX;
            float aimY = targetY;

            if (traj.predictTarget) {
                float currentSpeed = GameMath::speed(velocity.vx, velocity.vy);
                if (currentSpeed > 0.0f) {
                    float dist = GameMath::distance(transform.x, transform.y, aimX, aimY);
                    float travelTime = dist / currentSpeed;
                    aimX += targetVelX * travelTime * 0.5f;
                    aimY += targetVelY * travelTime * 0.5f;
                }
            }

            float dx = aimX - transform.x;
            float dy = aimY - transform.y;
            float desiredAngle = std::atan2(dy, dx);
            float currentAngle = std::atan2(velocity.vy, velocity.vx);

            float angleDiff = GameMath::normalizeAngle(desiredAngle - currentAngle);

            float maxTurn = traj.homingStrength * dt;
            if (std::abs(angleDiff) > maxTurn) {
                angleDiff = (angleDiff > 0.0f) ? maxTurn : -maxTurn;
            }

            float newAngle = currentAngle + angleDiff;
            float speed = GameMath::speed(velocity.vx, velocity.vy);
            velocity.vx = std::cos(newAngle) * speed;
            velocity.vy = std::sin(newAngle) * speed;
        }

        static void updateSinusoidal(TrajectoryComponent& traj,
                                     VelocityComponent& velocity,
                                     float dt) {
            (void)dt;
            float effectiveTime = traj.elapsedTime - traj.delay;
            if (effectiveTime < 0.0f) return;

            float waveDerivative = std::cos(effectiveTime * traj.waveFrequency * 2.0f * GameMath::M_PI_F + traj.wavePhase)
                                   * traj.waveFrequency * 2.0f * GameMath::M_PI_F;

            if (traj.wavePerpendicular) {
                float baseSpeed = GameMath::speed(traj.baseVelX, traj.baseVelY);
                if (baseSpeed > 0.0f) {
                    float perpX, perpY;
                    GameMath::perpendicular(traj.baseVelX, traj.baseVelY, perpX, perpY);
                    velocity.vx = traj.baseVelX + perpX * waveDerivative * traj.waveAmplitude;
                    velocity.vy = traj.baseVelY + perpY * waveDerivative * traj.waveAmplitude;
                }
            } else {
                velocity.vx = traj.baseVelX;
                velocity.vy = traj.baseVelY + waveDerivative * traj.waveAmplitude;
            }
        }

        static void updateBezier(TrajectoryComponent& traj,
                                 TransformComponent& transform,
                                 VelocityComponent& velocity,
                                 float dt) {
            float effectiveTime = traj.elapsedTime - traj.delay;
            if (effectiveTime < 0.0f) return;

            float t = effectiveTime / traj.bezierDuration;
            if (t > 1.0f) t = 1.0f;

            float newX, newY;
            GameMath::bezierPoint(t,
                                  traj.startX, traj.startY,
                                  traj.control1X, traj.control1Y,
                                  traj.control2X, traj.control2Y,
                                  traj.endX, traj.endY,
                                  newX, newY);

            velocity.vx = (newX - transform.x) / dt;
            velocity.vy = (newY - transform.y) / dt;
            transform.x = newX;
            transform.y = newY;
        }

        static void updateCircular(TrajectoryComponent& traj,
                                   TransformComponent& transform,
                                   VelocityComponent& velocity,
                                   float dt) {
            traj.currentAngle += traj.angularVelocity * dt;
            traj.orbitRadius += traj.radiusChangeRate * dt;
            if (traj.orbitRadius < 0.0f) traj.orbitRadius = 0.0f;

            float targetX = traj.orbitCenterX + std::cos(traj.currentAngle) * traj.orbitRadius;
            float targetY = traj.orbitCenterY + std::sin(traj.currentAngle) * traj.orbitRadius;

            velocity.vx = (targetX - transform.x) / dt;
            velocity.vy = (targetY - transform.y) / dt;
            GameMath::clampSpeed(velocity.vx, velocity.vy, velocity.maxSpeed);
        }

        static void updateAccelerating(TrajectoryComponent& traj,
                                       VelocityComponent& velocity,
                                       float dt) {
            float effectiveTime = traj.elapsedTime - traj.delay;
            if (effectiveTime < traj.speedChangeDelay) return;

            float currentSpeed = GameMath::speed(velocity.vx, velocity.vy);
            if (currentSpeed < 0.001f) return;

            float speedDiff = traj.targetSpeed - currentSpeed;
            float speedChange = traj.acceleration * dt;

            if (std::abs(speedDiff) < speedChange) {
                speedChange = std::abs(speedDiff);
            }

            float newSpeed = currentSpeed + (speedDiff > 0.0f ? speedChange : -speedChange);
            GameMath::setVelocitySpeed(velocity.vx, velocity.vy, newSpeed);
        }

        static void updateAimed(TrajectoryComponent& traj,
                                const TransformComponent& transform,
                                VelocityComponent& velocity,
                                float targetX, float targetY) {
            if (traj.hasAimed) return;

            float dx = targetX - transform.x;
            float dy = targetY - transform.y;
            float angle = std::atan2(dy, dx);

            float speed = GameMath::speed(velocity.vx, velocity.vy);
            velocity.vx = std::cos(angle) * speed;
            velocity.vy = std::sin(angle) * speed;

            traj.hasAimed = true;
            traj.baseVelX = velocity.vx;
            traj.baseVelY = velocity.vy;
        }

        static void updateBoomerang(TrajectoryComponent& traj,
                                    const TransformComponent& transform,
                                    VelocityComponent& velocity) {
            float distFromOrigin = GameMath::distance(transform.x, transform.y, traj.originX, traj.originY);

            if (traj.boomerangPhase < 0.5f) {
                if (distFromOrigin >= traj.boomerangDistance) {
                    traj.boomerangPhase = 1.0f;
                    velocity.vx = -velocity.vx;
                    velocity.vy = -velocity.vy;
                }
            }
        }

        static void updateSpiral(TrajectoryComponent& traj,
                                 const TransformComponent& transform,
                                 VelocityComponent& velocity,
                                 float dt) {
            float effectiveTime = traj.elapsedTime - traj.delay;
            if (effectiveTime < 0.0f) return;

            if (effectiveTime < dt * 2.0f) {
                traj.originX = transform.x;
                traj.originY = transform.y;
            }

            float angle = effectiveTime * traj.spiralTightness * 2.0f * GameMath::M_PI_F;
            float radius = effectiveTime * traj.spiralExpansionRate;

            float targetX = traj.originX + std::cos(angle) * radius;
            float targetY = traj.originY + std::sin(angle) * radius;

            velocity.vx = (targetX - transform.x) / dt;
            velocity.vy = (targetY - transform.y) / dt;
            GameMath::clampSpeed(velocity.vx, velocity.vy, velocity.maxSpeed);
        }

        static void updateRandom(TrajectoryComponent& traj,
                                 VelocityComponent& velocity,
                                 float dt) {
            traj.randomTimer += dt;

            if (traj.randomTimer >= traj.randomInterval) {
                traj.randomTimer = 0.0f;

                float angleChange = (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) *
                                    2.0f * traj.randomAngleRange * GameMath::DEG_TO_RAD;

                float currentAngle = std::atan2(velocity.vy, velocity.vx);
                float newAngle = currentAngle + angleChange;

                float speed = GameMath::speed(velocity.vx, velocity.vy);
                velocity.vx = std::cos(newAngle) * speed;
                velocity.vy = std::sin(newAngle) * speed;
            }
        }
        
        // ============== New Trajectory Types ==============
        
        static void updateZigzag(TrajectoryComponent& traj,
                                 VelocityComponent& velocity,
                                 float dt) {
            float effectiveTime = traj.elapsedTime - traj.delay;
            if (effectiveTime < 0.0f) return;
            
            float speed = GameMath::speed(traj.baseVelX, traj.baseVelY);
            traj.zigzagProgress += speed * dt;
            
            if (traj.zigzagProgress >= traj.zigzagLength) {
                traj.zigzagProgress = 0.0f;
                traj.zigzagDirection *= -1;
            }
            
            float perpX, perpY;
            GameMath::perpendicular(traj.baseVelX, traj.baseVelY, perpX, perpY);
            
            float sideOffset = traj.zigzagWidth * traj.zigzagDirection;
            float sideSpeed = sideOffset / (traj.zigzagLength / speed);
            
            velocity.vx = traj.baseVelX + perpX * sideSpeed;
            velocity.vy = traj.baseVelY + perpY * sideSpeed;
        }
        
        static void updateFigure8(TrajectoryComponent& traj,
                                  const TransformComponent& transform,
                                  VelocityComponent& velocity,
                                  float dt) {
            float effectiveTime = traj.elapsedTime - traj.delay;
            if (effectiveTime < 0.0f) return;
            
            float t = effectiveTime * traj.figure8Speed;
            
            float sinT = std::sin(t);
            float cosT = std::cos(t);
            float denom = 1.0f + sinT * sinT;
            
            float targetX = traj.originX + (traj.figure8Width * cosT) / denom;
            float targetY = traj.originY + (traj.figure8Height * sinT * cosT) / denom;
            
            velocity.vx = (targetX - transform.x) / dt;
            velocity.vy = (targetY - transform.y) / dt;
            GameMath::clampSpeed(velocity.vx, velocity.vy, velocity.maxSpeed);
        }
        
        static void updateDelayedHoming(TrajectoryComponent& traj,
                                        const TransformComponent& transform,
                                        VelocityComponent& velocity,
                                        float targetX, float targetY,
                                        float targetVelX, float targetVelY,
                                        float dt) {
            float effectiveTime = traj.elapsedTime - traj.delay;
            
            if (effectiveTime < traj.homingDelay) {
                return;
            }
            
            if (!traj.homingStarted) {
                traj.homingStarted = true;
                traj.baseVelX = velocity.vx;
                traj.baseVelY = velocity.vy;
            }
            
            updateHoming(traj, transform, velocity, targetX, targetY, targetVelX, targetVelY, dt);
        }
        
        static void updatePendulum(TrajectoryComponent& traj,
                                   VelocityComponent& velocity,
                                   float dt) {
            (void)dt;
            float effectiveTime = traj.elapsedTime - traj.delay;
            if (effectiveTime < 0.0f) return;
            
            float dampFactor = traj.pendulumDamping > 0.0f ? 
                               std::exp(-traj.pendulumDamping * effectiveTime) : 1.0f;
            float angle = traj.pendulumAmplitude * dampFactor * 
                          std::sin(effectiveTime * traj.pendulumFrequency * 2.0f * GameMath::M_PI_F);
            
            float baseAngle = std::atan2(traj.baseVelY, traj.baseVelX);
            float newAngle = baseAngle + angle * GameMath::DEG_TO_RAD;
            
            float speed = GameMath::speed(traj.baseVelX, traj.baseVelY);
            velocity.vx = std::cos(newAngle) * speed;
            velocity.vy = std::sin(newAngle) * speed;
        }
        
        static void updateSpiralInward(TrajectoryComponent& traj,
                                       const TransformComponent& transform,
                                       VelocityComponent& velocity,
                                       float dt) {
            float effectiveTime = traj.elapsedTime - traj.delay;
            if (effectiveTime < 0.0f) return;
            
            float dx = traj.spiralTargetX - transform.x;
            float dy = traj.spiralTargetY - transform.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            
            if (dist < 5.0f) return; // Close enough
            
            float angle = effectiveTime * traj.spiralTightness * 2.0f * GameMath::M_PI_F;
            
            float targetDist = std::max(5.0f, dist - traj.spiralInwardRate * dt);
            float targetX = traj.spiralTargetX - std::cos(angle) * targetDist;
            float targetY = traj.spiralTargetY - std::sin(angle) * targetDist;
            
            velocity.vx = (targetX - transform.x) / dt;
            velocity.vy = (targetY - transform.y) / dt;
            GameMath::clampSpeed(velocity.vx, velocity.vy, velocity.maxSpeed);
        }
        
        static void updateWhip(TrajectoryComponent& traj,
                               VelocityComponent& velocity,
                               float dt) {
            (void)dt;
            float effectiveTime = traj.elapsedTime - traj.delay;
            if (effectiveTime < 0.0f) return;
            
            float duration = 2.0f; // Total whip duration
            float t = effectiveTime / duration;
            if (t > 1.0f) t = 1.0f;
            
            float speed;
            if (t < traj.whipAccelPhase) {
                float accelT = t / traj.whipAccelPhase;
                speed = traj.whipMinSpeed + (traj.whipMaxSpeed - traj.whipMinSpeed) * accelT * accelT;
            } else {
                float decelT = (t - traj.whipAccelPhase) / (1.0f - traj.whipAccelPhase);
                speed = traj.whipMaxSpeed - (traj.whipMaxSpeed - traj.whipMinSpeed) * decelT * decelT;
            }
            
            GameMath::setVelocitySpeed(velocity.vx, velocity.vy, speed);
        }
        
        static void updateWobble(TrajectoryComponent& traj,
                                 VelocityComponent& velocity,
                                 float dt) {
            (void)dt;
            float effectiveTime = traj.elapsedTime - traj.delay;
            if (effectiveTime < 0.0f) return;
            
            float wobbleX = std::sin(effectiveTime * traj.wobbleSpeed) * 
                            std::cos(effectiveTime * traj.wobbleSpeed * 1.7f) * traj.wobbleIntensity;
            float wobbleY = std::cos(effectiveTime * traj.wobbleSpeed * 0.8f) * 
                            std::sin(effectiveTime * traj.wobbleSpeed * 1.3f) * traj.wobbleIntensity;
            
            velocity.vx = traj.baseVelX + wobbleX;
            velocity.vy = traj.baseVelY + wobbleY;
        }
    };

} // namespace rtype::ecs
