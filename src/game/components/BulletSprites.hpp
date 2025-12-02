/*
** R-Type ECS - BulletSprites
** Auto-generated sprite mapping for Touhou bullet spritesheet
** Grid: 16x16, Offset: (2, 9)
** 
** This header provides mapping functions for bullet types/colors to
** spritesheet coordinates.
*/

#pragma once

#include "BulletTypes.hpp"

namespace rtype::ecs {

    // Spritesheet configuration
    constexpr int BULLET_SHEET_OFFSET_X = 2;
    constexpr int BULLET_SHEET_OFFSET_Y = 9;
    constexpr int BULLET_FRAME_WIDTH = 16;
    constexpr int BULLET_FRAME_HEIGHT = 16;

    /**
     * @brief Get the frame coordinates for a bullet type and color
     * 
     * Returns the column (frameX) and row (frameY) in the spritesheet grid.
     * Use with SpritesheetComponent to render bullets.
     * 
     * @param type The bullet type
     * @param color The bullet color
     * @param frameX Output: column in spritesheet
     * @param frameY Output: row in spritesheet
     * @return true if mapping exists, false otherwise
     */
    inline bool getBulletFrame(BulletType type, BulletColor color, int& frameX, int& frameY) {
        frameX = 19;
        frameY = 3;
        
        switch (type) {
            case BulletType::Pellet:
                frameY = 4;
                switch (color) {
                    case BulletColor::Red:     frameX = 20; break;
                    case BulletColor::Orange:  frameX = 33; break;
                    case BulletColor::Yellow:  frameX = 32; break;
                    case BulletColor::Green:   frameX = 30; break;
                    case BulletColor::Cyan:    frameX = 27; break;
                    case BulletColor::Blue:    frameX = 25; break;
                    case BulletColor::Purple:  frameX = 23; break;
                    case BulletColor::Magenta: frameX = 22; break;
                    case BulletColor::White:   frameX = 19; break;
                    case BulletColor::Black:   frameX = 34; break;
                    default: return false;
                }
                break;
                
            case BulletType::Dot:
                frameY = 7;
                switch (color) {
                    case BulletColor::Red:     frameX = 20; break;
                    case BulletColor::Orange:  frameX = 33; break;
                    case BulletColor::Yellow:  frameX = 32; break;
                    case BulletColor::Green:   frameX = 30; break;
                    case BulletColor::Cyan:    frameX = 27; break;
                    case BulletColor::Blue:    frameX = 25; break;
                    case BulletColor::Purple:  frameX = 23; break;
                    case BulletColor::Magenta: frameX = 22; break;
                    case BulletColor::White:   frameX = 19; break;
                    case BulletColor::Black:   frameX = 34; break;
                    default: return false;
                }
                break;
                
            case BulletType::Ball:
                frameY = 5;
                switch (color) {
                    case BulletColor::Red:     frameX = 20; break;
                    case BulletColor::Orange:  frameX = 33; break;
                    case BulletColor::Yellow:  frameX = 32; break;
                    case BulletColor::Green:   frameX = 30; break;
                    case BulletColor::Cyan:    frameX = 27; break;
                    case BulletColor::Blue:    frameX = 25; break;
                    case BulletColor::Purple:  frameX = 23; break;
                    case BulletColor::Magenta: frameX = 22; break;
                    case BulletColor::White:   frameX = 19; break;
                    case BulletColor::Black:   frameX = 34; break;
                    default: return false;
                }
                break;
                
            case BulletType::Outline:
                frameY = 3;
                switch (color) {
                    case BulletColor::Red:     frameX = 20; break;
                    case BulletColor::Orange:  frameX = 33; break;
                    case BulletColor::Yellow:  frameX = 32; break;
                    case BulletColor::Green:   frameX = 30; break;
                    case BulletColor::Cyan:    frameX = 27; break;
                    case BulletColor::Blue:    frameX = 25; break;
                    case BulletColor::Purple:  frameX = 23; break;
                    case BulletColor::Magenta: frameX = 22; break;
                    case BulletColor::White:   frameX = 19; break;
                    case BulletColor::Black:   frameX = 34; break;
                    default: return false;
                }
                break;
                
            case BulletType::Rice:
                frameY = 6;
                switch (color) {
                    case BulletColor::Red:     frameX = 20; break;
                    case BulletColor::Orange:  frameX = 33; break;
                    case BulletColor::Yellow:  frameX = 32; break;
                    case BulletColor::Green:   frameX = 30; break;
                    case BulletColor::Cyan:    frameX = 27; break;
                    case BulletColor::Blue:    frameX = 25; break;
                    case BulletColor::Purple:  frameX = 23; break;
                    case BulletColor::Magenta: frameX = 22; break;
                    case BulletColor::White:   frameX = 19; break;
                    case BulletColor::Black:   frameX = 34; break;
                    default: return false;
                }
                break;
                
            case BulletType::Orb:
                frameY = 8;
                switch (color) {
                    case BulletColor::Red:     frameX = 24; break;
                    case BulletColor::Orange:  frameX = 32; break;
                    case BulletColor::Yellow:  frameX = 27; break;
                    case BulletColor::Green:   frameX = 26; break;
                    default: return false;  // Only 4 colors available for Orb
                }
                break;
                
            case BulletType::BallLarge:
                frameY = 9;
                switch (color) {
                    case BulletColor::Magenta: frameX = 23; break;
                    default: return false;  // Only Magenta available
                }
                break;
                
            default:
                return false;
        }
        
        return true;
    }

    /**
     * @brief Get the pixel source rectangle for a bullet
     * 
     * @param type The bullet type
     * @param color The bullet color
     * @param srcX Output: X pixel position in spritesheet
     * @param srcY Output: Y pixel position in spritesheet
     * @param srcW Output: Width in pixels
     * @param srcH Output: Height in pixels
     * @return true if mapping exists
     */
    inline bool getBulletSourceRect(BulletType type, BulletColor color,
                                    int& srcX, int& srcY, int& srcW, int& srcH) {
        int frameX, frameY;
        if (!getBulletFrame(type, color, frameX, frameY)) {
            return false;
        }
        
        srcX = BULLET_SHEET_OFFSET_X + frameX * BULLET_FRAME_WIDTH;
        srcY = BULLET_SHEET_OFFSET_Y + frameY * BULLET_FRAME_HEIGHT;
        srcW = BULLET_FRAME_WIDTH;
        srcH = BULLET_FRAME_HEIGHT;
        return true;
    }

    /**
     * @brief Get human-readable name for bullet type
     */
    inline const char* getBulletTypeName(BulletType type) {
        switch (type) {
            case BulletType::Pellet:    return "Pellet";
            case BulletType::Dot:       return "Dot";
            case BulletType::Ball:      return "Ball";
            case BulletType::Outline:   return "Outline";
            case BulletType::Rice:      return "Rice";
            case BulletType::Kunai:     return "Kunai";
            case BulletType::Scale:     return "Scale";
            case BulletType::Bill:      return "Bill";
            case BulletType::BallLarge: return "BallLarge";
            case BulletType::Orb:       return "Orb";
            case BulletType::Bubble:    return "Bubble";
            case BulletType::Heart:     return "Heart";
            case BulletType::Star:      return "Star";
            case BulletType::Butterfly: return "Butterfly";
            case BulletType::Knife:     return "Knife";
            case BulletType::Arrow:     return "Arrow";
            default:                    return "Unknown";
        }
    }

    /**
     * @brief Get human-readable name for bullet color
     */
    inline const char* getBulletColorName(BulletColor color) {
        switch (color) {
            case BulletColor::Red:     return "Red";
            case BulletColor::Orange:  return "Orange";
            case BulletColor::Yellow:  return "Yellow";
            case BulletColor::Green:   return "Green";
            case BulletColor::Cyan:    return "Cyan";
            case BulletColor::Blue:    return "Blue";
            case BulletColor::Purple:  return "Purple";
            case BulletColor::Magenta: return "Magenta";
            case BulletColor::White:   return "White";
            case BulletColor::Black:   return "Black";
            default:                   return "Unknown";
        }
    }

    /**
     * @brief Get list of available bullet types that have sprite mappings
     */
    inline constexpr BulletType AVAILABLE_BULLET_TYPES[] = {
        BulletType::Pellet,
        BulletType::Dot,
        BulletType::Ball,
        BulletType::Outline,
        BulletType::Rice,
        BulletType::Orb,
        BulletType::BallLarge
    };
    
    inline constexpr int AVAILABLE_BULLET_TYPE_COUNT = 7;

} // namespace rtype::ecs
