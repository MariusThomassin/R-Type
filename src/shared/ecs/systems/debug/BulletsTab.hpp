/*
** R-Type ECS - Bullets Debug Tab
** Browse bullet sprite types and colors
*/

#pragma once

#include "DebugTab.hpp"
#include "../../components/BulletSprites.hpp"

namespace rtype::ecs::debug {

    class BulletsTab : public IDebugTab {
    public:
        const char* getName() const override { return "Bullets"; }

        void update(float dt) override {
            m_animTime += dt;
            m_timer += dt;
            
            if (!m_isHovering && m_timer >= 2.0f) {
                m_timer = 0;
                m_colorIdx = (m_colorIdx + 1) % static_cast<int>(BulletColor::COUNT);
                if (m_colorIdx == 0) {
                    m_typeIdx = (m_typeIdx + 1) % AVAILABLE_BULLET_TYPE_COUNT;
                }
            }
        }

        void handleMouse(const MouseInput& mouse) override {
            m_isHovering = false;
            int mx = static_cast<int>(mouse.x);
            int my = static_cast<int>(mouse.y);
            
            if (my >= m_colorRowY && my < m_colorRowY + 80) {
                m_isHovering = true;
                for (int c = 0; c < static_cast<int>(BulletColor::COUNT); c++) {
                    int btnX = 30 + c * 66;
                    if (mx >= btnX && mx < btnX + 60 && mouse.leftPressed) {
                        m_colorIdx = c;
                        m_timer = 0;
                    }
                }
            }
            
            if (my >= m_typeListY && my < m_typeListY + AVAILABLE_BULLET_TYPE_COUNT * 20) {
                m_isHovering = true;
                if (mouse.leftPressed) {
                    int idx = (my - m_typeListY) / 20;
                    if (idx >= 0 && idx < AVAILABLE_BULLET_TYPE_COUNT) {
                        m_typeIdx = idx;
                        m_timer = 0;
                    }
                }
            }
        }

        void draw(int y) override {
            DrawText("Bullet Sprite Browser", 30, y, 20, WHITE);
            DrawText("Click to select  |  Auto-cycles when idle", 260, y + 3, 14, {150, 150, 150, 255});
            y += 40;

            BulletType type = AVAILABLE_BULLET_TYPES[m_typeIdx];
            BulletColor color = static_cast<BulletColor>(m_colorIdx);

            char buf[128];
            snprintf(buf, sizeof(buf), "Type: %s  |  Color: %s", getBulletTypeName(type), getBulletColorName(color));
            DrawText(buf, 30, y, 18, {255, 255, 100, 255}); y += 35;

            if (!m_textures) return;

            auto it = m_textures->find("touhou_bullets");
            if (it == m_textures->end()) return;

            DrawText("Click a color:", 30, y, 14, {150, 150, 150, 255}); y += 22;
            m_colorRowY = y;
            
            int px = 30;
            float scale = 3.0f;
            for (int c = 0; c < static_cast<int>(BulletColor::COUNT); c++) {
                int sx, sy, sw, sh;
                if (getBulletSourceRect(type, static_cast<BulletColor>(c), sx, sy, sw, sh)) {
                    bool selected = (c == m_colorIdx);
                    bool hovered = isMouseOver(px - 3, y - 3, static_cast<int>(sw * scale) + 6, static_cast<int>(sh * scale) + 6);
                    
                    if (selected) {
                        DrawRectangle(px - 3, y - 3, static_cast<int>(sw * scale) + 6, static_cast<int>(sh * scale) + 6, {255, 255, 100, 80});
                    } else if (hovered) {
                        DrawRectangle(px - 3, y - 3, static_cast<int>(sw * scale) + 6, static_cast<int>(sh * scale) + 6, {100, 100, 150, 80});
                    }
                    
                    Rectangle src = {static_cast<float>(sx), static_cast<float>(sy), static_cast<float>(sw), static_cast<float>(sh)};
                    Rectangle dst = {static_cast<float>(px), static_cast<float>(y), sw * scale, sh * scale};
                    DrawTexturePro(it->second, src, dst, {0, 0}, 0, WHITE);
                    DrawText(getBulletColorName(static_cast<BulletColor>(c)), px, y + static_cast<int>(sh * scale) + 4, 10, {120, 120, 120, 255});
                }
                px += static_cast<int>(22 * scale);
            }
            y += 100;

            DrawText("Click a type:", 30, y, 14, {150, 150, 150, 255}); y += 22;
            m_typeListY = y;
            
            for (int t = 0; t < AVAILABLE_BULLET_TYPE_COUNT; t++) {
                bool sel = (t == m_typeIdx);
                bool hovered = isMouseOver(30, y, 200, 18);
                
                if (sel) {
                    DrawRectangle(25, y - 2, 210, 20, {60, 80, 120, 255});
                } else if (hovered) {
                    DrawRectangle(25, y - 2, 210, 20, {50, 50, 70, 255});
                }
                
                snprintf(buf, sizeof(buf), "%s", getBulletTypeName(AVAILABLE_BULLET_TYPES[t]));
                DrawText(buf, 30, y, 14, sel ? Color{255, 255, 100, 255} : Color{180, 180, 180, 255});
                y += 20;
            }
        }

    private:
        int m_typeIdx = 0;
        int m_colorIdx = 0;
        float m_timer = 0.0f;
        bool m_isHovering = false;
        int m_colorRowY = 0;
        int m_typeListY = 0;
    };

} // namespace rtype::ecs::debug
