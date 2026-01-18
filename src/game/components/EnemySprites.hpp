/*
** R-Type ECS - Enemy Sprite Definitions
** Hardcoded sprite configurations for enemy types
*/

#pragma once

#include "EnemyComponent.hpp"
#include <raylib.h>
#include <cmath>

namespace rtype::ecs {

    struct EnemySpriteInfo {
        Color bodyColor;
        Color glowColor;
        Color coreColor;
        float baseSize;
        int shapeType;  // 0=circle, 1=diamond, 2=square, 3=triangle, 4=hexagon
    };

    inline EnemySpriteInfo getEnemySpriteInfo(EnemyType type) {
        switch (type) {
            case EnemyType::Basic:
                return {
                    {100, 200, 255, 255},   // Cyan body
                    {100, 200, 255, 100},   // Cyan glow
                    {200, 240, 255, 200},   // Light core
                    16.0f,
                    0  // Circle
                };
            case EnemyType::Shooter:
                return {
                    {255, 150, 50, 255},    // Orange body
                    {255, 150, 50, 100},    // Orange glow
                    {255, 220, 150, 200},   // Light core
                    18.0f,
                    1  // Diamond
                };
            case EnemyType::Chaser:
                return {
                    {200, 100, 255, 255},   // Purple body
                    {200, 100, 255, 100},   // Purple glow
                    {230, 180, 255, 200},   // Light core
                    14.0f,
                    3  // Triangle
                };
            case EnemyType::Turret:
                return {
                    {100, 255, 100, 255},   // Green body
                    {100, 255, 100, 100},   // Green glow
                    {200, 255, 200, 200},   // Light core
                    20.0f,
                    2  // Square
                };
            case EnemyType::Boss:
                return {
                    {255, 50, 50, 255},     // Red body
                    {255, 50, 50, 150},     // Red glow
                    {255, 200, 200, 255},   // Bright core
                    48.0f,                   // 96x96 -> radius 48
                    4  // Hexagon
                };
            default:
                return {
                    {255, 100, 100, 255},
                    {255, 100, 100, 100},
                    {255, 200, 200, 200},
                    16.0f,
                    0
                };
        }
    }

    inline void drawEnemyShape(float x, float y, float size, int shapeType, Color color) {
        switch (shapeType) {
            case 0: // Circle
                DrawCircle(static_cast<int>(x), static_cast<int>(y), size, color);
                break;
            case 1: // Diamond
                DrawPoly({x, y}, 4, size, 45.0f, color);
                break;
            case 2: // Square
                DrawRectangle(static_cast<int>(x - size), static_cast<int>(y - size),
                              static_cast<int>(size * 2), static_cast<int>(size * 2), color);
                break;
            case 3: // Triangle
                DrawTriangle(
                    {x - size, y},
                    {x + size * 0.7f, y - size * 0.8f},
                    {x + size * 0.7f, y + size * 0.8f},
                    color
                );
                break;
            case 4: // Hexagon
                DrawPoly({x, y}, 6, size, 0.0f, color);
                break;
            default:
                DrawCircle(static_cast<int>(x), static_cast<int>(y), size, color);
                break;
        }
    }

    inline void renderEnemySprite(float x, float y, float scale, EnemyType type, float animTime) {
        EnemySpriteInfo info = getEnemySpriteInfo(type);
        float size = info.baseSize * scale;
        
        float pulse = 0.5f + 0.5f * std::sin(animTime * 4.0f);
        Color glowColor = info.glowColor;
        glowColor.a = static_cast<unsigned char>(info.glowColor.a * pulse);
        
        drawEnemyShape(x, y, size * 1.5f, info.shapeType, glowColor);
        drawEnemyShape(x, y, size, info.shapeType, info.bodyColor);
        drawEnemyShape(x, y, size * 0.4f, info.shapeType, info.coreColor);
        
        if (type == EnemyType::Boss) {
            float ringAngle = animTime * 60.0f;
            for (int i = 0; i < 6; i++) {
                float angle = ringAngle + i * 60.0f;
                float orbX = x + std::cos(angle * 3.14159f / 180.0f) * size * 1.2f;
                float orbY = y + std::sin(angle * 3.14159f / 180.0f) * size * 1.2f;
                DrawCircle(static_cast<int>(orbX), static_cast<int>(orbY), 
                           size * 0.2f, {255, 100, 100, 200});
            }
        }
    }

} // namespace rtype::ecs
