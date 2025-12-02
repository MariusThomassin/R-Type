/*
** R-Type ECS - BulletTypes
** Bullet type and color enumerations
** Used by SpritesheetComponent and BulletSprites
*/

#pragma once

namespace rtype::ecs {

    /**
     * @brief Bullet visual types available in the spritesheet
     *
     * Based on typical Touhou bullet spritesheet layouts.
     * Each type can have multiple color variants.
     */
    enum class BulletType : int {
        // Small bullets (8x8 or 16x16)
        Pellet = 0,         // Small round bullets
        Dot = 1,            // Tiny dots
        Ball = 2,           // Medium round bullets
        Outline = 3,        // Ring/outline bullets

        // Medium bullets (16x16 or 32x32)
        Rice = 4,           // Oval/rice shaped
        Kunai = 5,          // Pointed kunai
        Scale = 6,          // Scale/leaf shaped
        Bill = 7,           // Bill/elongated

        // Large bullets (32x32 or larger)
        BallLarge = 8,      // Large spheres
        Orb = 9,            // Glowing orbs
        Bubble = 10,        // Transparent bubbles
        Heart = 11,         // Heart shaped

        // Special bullets
        Star = 12,          // Star shaped
        Butterfly = 13,     // Butterfly/moth
        Knife = 14,         // Knife/dagger
        Arrow = 15,         // Arrows

        // Laser segments
        LaserHead = 16,
        LaserBody = 17,
        LaserTail = 18,

        COUNT
    };

    /**
     * @brief Color variants for bullets
     *
     * Standard Touhou color palette for bullets.
     */
    enum class BulletColor : int {
        Red = 0,
        Orange = 1,
        Yellow = 2,
        Green = 3,
        Cyan = 4,
        Blue = 5,
        Purple = 6,
        Magenta = 7,
        White = 8,
        Black = 9,

        COUNT
    };

} // namespace rtype::ecs
