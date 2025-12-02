/*
** R-Type ECS - Targeting Calculator
** Calculates aimed shot angles based on target position
** Used by bullets, enemies, turrets, and any aiming systems
*/

#pragma once

#include "GameMath.hpp"
#include "TargetTracker.hpp"
#include "../../components/patterns/PatternTypes.hpp"
#include <cmath>
#include <cstdlib>

namespace rtype::ecs {

    /**
     * @brief Calculates aimed shot angles for any targeting scenario
     * Can be used for bullets, enemy AI, turrets, homing effects, etc.
     */
    struct TargetingCalculator {

        /**
         * @brief Calculate the base angle with aim mode applied
         */
        static float calculateAimedAngle(float baseAngle,
                                          AimMode aimMode,
                                          float spawnX, float spawnY,
                                          float speed,
                                          float angleSpread,
                                          const TargetTracker& targetTracker) {
            switch (aimMode) {
                case AimMode::AtPlayer:
                    return baseAngle + calculateAngleToTarget(spawnX, spawnY, targetTracker);

                case AimMode::AtPlayerLead:
                    return baseAngle + calculateAngleToTargetLead(spawnX, spawnY, speed, targetTracker);

                case AimMode::Random:
                    return baseAngle + calculateRandomOffset(angleSpread);

                case AimMode::Fixed:
                case AimMode::Sequence:
                default:
                    return baseAngle;
            }
        }

        /**
         * @brief Calculate angle to a specific position
         */
        static float calculateAngleToPosition(float fromX, float fromY, float toX, float toY) {
            float dx = toX - fromX;
            float dy = toY - fromY;
            return std::atan2(dy, dx) * GameMath::RAD_TO_DEG;
        }

        /**
         * @brief Calculate angle to tracked target
         */
        static float calculateAngleToTarget(float spawnX, float spawnY,
                                            const TargetTracker& tracker) {
            float dx = tracker.getX() - spawnX;
            float dy = tracker.getY() - spawnY;
            return std::atan2(dy, dx) * GameMath::RAD_TO_DEG;
        }

        /**
         * @brief Calculate angle with lead prediction
         */
        static float calculateAngleToTargetLead(float spawnX, float spawnY,
                                                 float projectileSpeed,
                                                 const TargetTracker& tracker) {
            if (projectileSpeed <= 0.0f) {
                return calculateAngleToTarget(spawnX, spawnY, tracker);
            }

            float targetX = tracker.getX();
            float targetY = tracker.getY();

            // Lead prediction
            float dist = GameMath::distance(spawnX, spawnY, targetX, targetY);
            float travelTime = dist / projectileSpeed;
            targetX += tracker.getVelX() * travelTime * 0.5f;
            targetY += tracker.getVelY() * travelTime * 0.5f;

            float dx = targetX - spawnX;
            float dy = targetY - spawnY;
            return std::atan2(dy, dx) * GameMath::RAD_TO_DEG;
        }

        /**
         * @brief Calculate random angle offset
         */
        static float calculateRandomOffset(float angleSpread) {
            return (static_cast<float>(std::rand()) / RAND_MAX - 0.5f) * angleSpread;
        }

    private:
        // Backward compatibility - private versions that use PlayerTracker name
        static float calculateAngleToPlayer(float spawnX, float spawnY,
                                            const TargetTracker& tracker) {
            return calculateAngleToTarget(spawnX, spawnY, tracker);
        }

        static float calculateAngleToPlayerLead(float spawnX, float spawnY,
                                                 float speed,
                                                 const TargetTracker& tracker) {
            return calculateAngleToTargetLead(spawnX, spawnY, speed, tracker);
        }
    };

    // Backward compatibility alias
    using AimCalculator = TargetingCalculator;

} // namespace rtype::ecs
