/*
** R-Type ECS - Raylib Renderer Implementation
** Concrete renderer using raylib for graphics
*/

#pragma once

#include "IRenderer.hpp"
#include <raylib.h>
#include <unordered_map>
#include <string>

namespace rtype::ecs {

    /**
     * @brief Raylib-based renderer implementation
     */
    class RaylibRenderer : public IRenderer {
    public:
        RaylibRenderer(int screenWidth = 1280, int screenHeight = 720)
            : m_screenWidth(screenWidth)
            , m_screenHeight(screenHeight) {}

        ~RaylibRenderer() override {
            // Unload all textures
            if (IsWindowReady()) {
                for (auto& [id, tex] : m_textures) {
                    UnloadTexture(tex);
                }
            }
        }

        // ==================== Basic Shapes ====================

        void drawRect(float x, float y, float width, float height,
                      const RenderColor& color) override {
            DrawRectangle(static_cast<int>(x), static_cast<int>(y),
                          static_cast<int>(width), static_cast<int>(height),
                          toRaylibColor(color));
        }

        void drawRectRotated(const RenderRect& rect, float originX, float originY,
                             float rotation, const RenderColor& color) override {
            Rectangle raylibRect = {rect.x, rect.y, rect.width, rect.height};
            Vector2 origin = {originX, originY};
            DrawRectanglePro(raylibRect, origin, rotation, toRaylibColor(color));
        }

        void drawCircle(float centerX, float centerY, float radius,
                        const RenderColor& color) override {
            DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY),
                       radius, toRaylibColor(color));
        }

        void drawLine(float x1, float y1, float x2, float y2,
                      const RenderColor& color) override {
            DrawLine(static_cast<int>(x1), static_cast<int>(y1),
                     static_cast<int>(x2), static_cast<int>(y2),
                     toRaylibColor(color));
        }

        // ==================== Textures ====================

        bool loadTexture(const std::string& id, const std::string& path) override {
            if (m_textures.find(id) != m_textures.end()) {
                return true; // Already loaded
            }

            Texture2D tex = LoadTexture(path.c_str());
            if (tex.id == 0) {
                return false;
            }

            m_textures[id] = tex;
            return true;
        }

        void unloadTexture(const std::string& id) override {
            auto it = m_textures.find(id);
            if (it != m_textures.end()) {
                if (IsWindowReady()) {
                    UnloadTexture(it->second);
                }
                m_textures.erase(it);
            }
        }

        void drawTexture(const std::string& id, float x, float y,
                         const RenderColor& tint) override {
            auto it = m_textures.find(id);
            if (it != m_textures.end()) {
                DrawTexture(it->second, static_cast<int>(x), static_cast<int>(y),
                            toRaylibColor(tint));
            }
        }

        void drawTextureRect(const std::string& id,
                             const RenderRect& src, const RenderRect& dst,
                             float rotation,
                             const RenderColor& tint) override {
            auto it = m_textures.find(id);
            if (it != m_textures.end()) {
                Rectangle srcRect = {src.x, src.y, src.width, src.height};
                Rectangle dstRect = {dst.x, dst.y, dst.width, dst.height};
                Vector2 origin = {dst.width / 2.0f, dst.height / 2.0f};
                DrawTexturePro(it->second, srcRect, dstRect, origin,
                               rotation, toRaylibColor(tint));
            }
        }

        // ==================== Text ====================

        void drawText(const std::string& text, float x, float y,
                      int fontSize, const RenderColor& color) override {
            DrawText(text.c_str(), static_cast<int>(x), static_cast<int>(y),
                     fontSize, toRaylibColor(color));
        }

        // ==================== Frame Management ====================

        void beginFrame() override {
            BeginDrawing();
        }

        void endFrame() override {
            EndDrawing();
        }

        void clear(const RenderColor& color) override {
            ClearBackground(toRaylibColor(color));
        }

        // ==================== Screen Info ====================

        int getScreenWidth() const override { return m_screenWidth; }
        int getScreenHeight() const override { return m_screenHeight; }

        void setScreenSize(int width, int height) {
            m_screenWidth = width;
            m_screenHeight = height;
        }

        // ==================== Direct Access (for advanced usage) ====================

        /**
         * @brief Get raw texture for direct raylib calls
         * @note Use sparingly - prefer using IRenderer methods
         */
        const Texture2D* getRawTexture(const std::string& id) const {
            auto it = m_textures.find(id);
            return (it != m_textures.end()) ? &it->second : nullptr;
        }

        /**
         * @brief Get all textures (for legacy RenderContext compatibility)
         */
        const std::unordered_map<std::string, Texture2D>& getTextures() const {
            return m_textures;
        }

    private:
        static Color toRaylibColor(const RenderColor& c) {
            return {c.r, c.g, c.b, c.a};
        }

        int m_screenWidth;
        int m_screenHeight;
        std::unordered_map<std::string, Texture2D> m_textures;
    };

} // namespace rtype::ecs
