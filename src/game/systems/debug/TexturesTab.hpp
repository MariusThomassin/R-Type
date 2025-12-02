/*
** R-Type ECS - Textures Debug Tab
** Shows loaded textures with preview and usage info
*/

#pragma once

#include "DebugTab.hpp"
#include "../../components/BulletSprites.hpp"

namespace rtype::ecs::debug {

    class TexturesTab : public IDebugTab {
    public:
        const char* getName() const override { return "Textures"; }

        void update(float dt) override {
            m_animTime += dt;
        }

        void handleMouse(const MouseInput& mouse) override {
            if (mouse.y > 200) {
                m_zoom = std::max(1.0f, std::min(8.0f, m_zoom + mouse.wheelDelta * 0.5f));
                m_scrollY -= mouse.wheelDelta * 40;
                m_scrollY = std::max(0.0f, m_scrollY);
            }
            
            if (mouse.leftPressed && mouse.y >= 125 && mouse.y < 125 + static_cast<int>(m_textures->size()) * 20) {
                int idx = (static_cast<int>(mouse.y) - 125) / 20;
                if (idx >= 0 && idx < static_cast<int>(m_textures->size())) {
                    m_selectedIdx = idx;
                }
            }
        }

        void draw(int y) override {
            DrawText("Loaded Textures", 30, y, 20, WHITE);
            DrawText("Scroll: zoom (Ctrl+scroll)  Click: select", 220, y + 3, 14, {150, 150, 150, 255});
            y += 35;

            if (!m_textures || m_textures->empty()) {
                DrawText("No textures loaded!", 30, y, 16, {255, 100, 100, 255});
                return;
            }

            DrawText("Loaded Textures:", 30, y, 16, {100, 200, 255, 255}); y += 24;
            int idx = 0;
            std::string selectedId;
            for (const auto& [id, tex] : *m_textures) {
                char buf[128];
                snprintf(buf, sizeof(buf), "%s %s: %dx%d (%d KB)", 
                    idx == m_selectedIdx ? ">" : " ",
                    id.c_str(), tex.width, tex.height, 
                    (tex.width * tex.height * 4) / 1024);
                DrawText(buf, 30, y, 14, idx == m_selectedIdx ? Color{255, 255, 100, 255} : Color{180, 180, 180, 255});
                if (idx == m_selectedIdx) selectedId = id;
                y += 20;
                idx++;
            }
            y += 10;

            if (!selectedId.empty()) {
                auto it = m_textures->find(selectedId);
                if (it != m_textures->end()) {
                    const Texture2D& tex = it->second;
                    
                    DrawText("Usage in Engine:", 30, y, 16, {100, 200, 255, 255}); y += 24;
                    
                    if (selectedId == "touhou_bullets") {
                        DrawText("- SpritesheetComponent: Bullet sprites (16x16 grid)", 30, y, 14, {150, 150, 150, 255}); y += 20;
                        char buf[128];
                        snprintf(buf, sizeof(buf), "- Grid offset: (%d, %d)  Cell size: %dx%d",
                            BULLET_SHEET_OFFSET_X, BULLET_SHEET_OFFSET_Y, BULLET_FRAME_WIDTH, BULLET_FRAME_HEIGHT);
                        DrawText(buf, 30, y, 14, {150, 150, 150, 255}); y += 20;
                        snprintf(buf, sizeof(buf), "- Available types: %d  Colors: %d", AVAILABLE_BULLET_TYPE_COUNT, (int)BulletColor::COUNT);
                        DrawText(buf, 30, y, 14, {150, 150, 150, 255}); y += 20;
                    } else {
                        DrawText("- Used by SpriteComponent entities", 30, y, 14, {150, 150, 150, 255}); y += 20;
                    }
                    y += 10;

                    drawTexturePreview(tex, selectedId, y);
                }
            }
        }

    private:
        float m_zoom = 3.0f;
        float m_scrollY = 0.0f;
        int m_selectedIdx = 0;

        void drawTexturePreview(const Texture2D& tex, const std::string& id, int y) {
            DrawText("Preview:", 30, y, 16, {100, 200, 255, 255}); y += 24;

            int px = 30, pw = m_screenWidth - 60, ph = m_screenHeight - y - 40;
            BeginScissorMode(px, y, pw, ph);

            for (int cx = 0; cx < pw / 16 + 1; cx++) {
                for (int cy = 0; cy < ph / 16 + 1; cy++) {
                    Color c = ((cx + cy) % 2 == 0) ? Color{35, 35, 45, 255} : Color{45, 45, 55, 255};
                    DrawRectangle(px + cx * 16, y + cy * 16, 16, 16, c);
                }
            }

            float scrollY = std::max(0.0f, m_scrollY);
            Rectangle src = {0, 0, (float)tex.width, (float)tex.height};
            Rectangle dst = {(float)px, y - scrollY, tex.width * m_zoom, tex.height * m_zoom};
            DrawTexturePro(tex, src, dst, {0, 0}, 0, WHITE);

            if (id == "touhou_bullets") {
                int ox = BULLET_SHEET_OFFSET_X, oy = BULLET_SHEET_OFFSET_Y;
                int gw = BULLET_FRAME_WIDTH, gh = BULLET_FRAME_HEIGHT;
                int cols = (tex.width - ox) / gw, rows = (tex.height - oy) / gh;

                for (int c = 0; c <= cols; c++) {
                    float lx = px + (ox + c * gw) * m_zoom;
                    DrawLine((int)lx, y + (int)(oy * m_zoom - scrollY), 
                            (int)lx, y + (int)((oy + rows * gh) * m_zoom - scrollY), {255, 255, 0, 60});
                }
                for (int r = 0; r <= rows; r++) {
                    float ly = y + (oy + r * gh) * m_zoom - scrollY;
                    DrawLine(px + (int)(ox * m_zoom), (int)ly,
                            px + (int)((ox + cols * gw) * m_zoom), (int)ly, {255, 255, 0, 60});
                }
            }

            EndScissorMode();
        }
    };

} // namespace rtype::ecs::debug
