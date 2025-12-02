/*
** R-Type ECS - Pattern Factory
** Factory methods for creating common Touhou-style patterns
*/

#pragma once

#include "BulletPatternComponent.hpp"
#include "PatternWave.hpp"
#include "game/components/BulletTypes.hpp"
#include "game/components/bullets/TrajectoryTypes.hpp"

namespace rtype::ecs {

    /**
     * @brief Factory for creating common bullet pattern types
     */
    struct PatternFactory {
        /**
         * @brief Simple circle pattern (classic danmaku)
         */
        static BulletPatternComponent createCircle(int bulletCount, float speed,
                                                   BulletType type, BulletColor color);

        /**
         * @brief Aimed fan pattern (common for aimed shots)
         */
        static BulletPatternComponent createAimedFan(int bulletCount, float spread, float speed,
                                                     BulletType type, BulletColor color);

        /**
         * @brief Rotating spiral pattern
         */
        static BulletPatternComponent createSpiral(int arms, int bulletsPerArm, float speed,
                                                   float rotSpeed, BulletType type, BulletColor color);

        /**
         * @brief Layered rings pattern (expanding circles)
         */
        static BulletPatternComponent createRings(int ringCount, int bulletsPerRing, float baseSpeed,
                                                  float speedIncrement, BulletType type, BulletColor color);

        /**
         * @brief Stream of aimed bullets
         */
        static BulletPatternComponent createStream(int bulletCount, float interval, float speed,
                                                   BulletType type, BulletColor color, bool aimed = true);

        /**
         * @brief Rose/flower pattern (multiple layered fans)
         */
        static BulletPatternComponent createRose(int petalCount, int bulletsPerPetal, float speed,
                                                 BulletType type, BulletColor color);

        /**
         * @brief Homing bullet pattern
         */
        static BulletPatternComponent createHoming(int bulletCount, float speed, float homingStrength,
                                                   BulletType type, BulletColor color);

        /**
         * @brief Cross/plus pattern
         */
        static BulletPatternComponent createCross(int bulletsPerArm, float speed,
                                                  BulletType type, BulletColor color);

        /**
         * @brief Sinusoidal wave pattern
         */
        static BulletPatternComponent createWave(int bulletCount, float speed, float amplitude,
                                                 float frequency, BulletType type, BulletColor color);

        /**
         * @brief Double helix pattern
         */
        static BulletPatternComponent createHelix(int bulletCount, float speed, float rotSpeed,
                                                  BulletType type, BulletColor color1, BulletColor color2);

        /**
         * @brief Shotgun/spread pattern
         */
        static BulletPatternComponent createShotgun(int bulletCount, float spread, float speed,
                                                    float speedVariation, BulletType type, BulletColor color);
    };

} // namespace rtype::ecs
