/*
** EPITECH PROJECT, 2025
** R-Type
** File description:
** RaylibRenderer - Raylib-based renderer implementation
*/

#include "RaylibRenderer.hpp"

namespace rtype::ecs {

    RaylibRenderer::RaylibRenderer(int screenWidth, int screenHeight) : m_screenWidth(screenWidth), m_screenHeight(screenHeight)
    {
        InitWindow(m_screenWidth, m_screenHeight, "R-Type ECS - Raylib Renderer");
        SetTargetFPS(60);
    }

    RaylibRenderer::~RaylibRenderer()
    {
        // Unload all textures
        if (IsWindowReady()) {
            for (auto& [id, tex] : m_textures) {
                UnloadTexture(tex);
            }
        }
        CloseWindow();
    }

    // ==================== Basic Shapes ====================

    void RaylibRenderer::drawRect(float x, float y, float width, float height, const RenderColor& color)
    {
        DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height), toRaylibColor(color));
    }

    void RaylibRenderer::drawRectRotated(const RenderRect& rect, float originX, float originY, float rotation, const RenderColor& color)
    {
        Rectangle raylibRect = {rect.x, rect.y, rect.width, rect.height};
        Vector2 origin = {originX, originY};
        DrawRectanglePro(raylibRect, origin, rotation, toRaylibColor(color));
    }

    void RaylibRenderer::drawCircle(float centerX, float centerY, float radius, const RenderColor& color)
    {
        DrawCircle(static_cast<int>(centerX), static_cast<int>(centerY), radius, toRaylibColor(color));
    }

    void RaylibRenderer::drawLine(float x1, float y1, float x2, float y2, const RenderColor& color)
    {
        DrawLine(static_cast<int>(x1), static_cast<int>(y1), static_cast<int>(x2), static_cast<int>(y2), toRaylibColor(color));
    }

    // ==================== Textures ====================

    bool RaylibRenderer::loadTexture(const std::string& id, const std::string& path)
    {
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

    void RaylibRenderer::unloadTexture(const std::string& id)
    {
        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            if (IsWindowReady()) {
                UnloadTexture(it->second);
            }
            m_textures.erase(it);
        }
    }

    void RaylibRenderer::drawTexture(const std::string& id, float x, float y, const RenderColor& tint)
    {
        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            DrawTexture(it->second, static_cast<int>(x), static_cast<int>(y),
                        toRaylibColor(tint));
        }
    }

    void RaylibRenderer::drawTextureRect(const std::string& id, const RenderRect& src, const RenderRect& dst, float rotation, const RenderColor& tint)
    {
        auto it = m_textures.find(id);
        if (it != m_textures.end()) {
            Rectangle srcRect = {src.x, src.y, src.width, src.height};
            Rectangle dstRect = {dst.x, dst.y, dst.width, dst.height};
            Vector2 origin = {dst.width / 2.0f, dst.height / 2.0f};
            DrawTexturePro(it->second, srcRect, dstRect, origin, rotation, toRaylibColor(tint));
        }
    }

    // ==================== Text ====================

    void RaylibRenderer::drawText(const std::string& text, float x, float y, int fontSize, const RenderColor& color)
    {
        DrawText(text.c_str(), static_cast<int>(x), static_cast<int>(y), fontSize, toRaylibColor(color));
    }

    // ==================== Frame Management ====================

    void RaylibRenderer::beginFrame()
    {
        BeginDrawing();
    }

    void RaylibRenderer::endFrame()
    {
        EndDrawing();
    }

    void RaylibRenderer::clear(const RenderColor& color)
    {
        ClearBackground(toRaylibColor(color));
    }

    // ==================== Screen Info ====================

    int RaylibRenderer::getScreenWidth() const
    {
        return m_screenWidth;
    }

    int RaylibRenderer::getScreenHeight() const
    {
        return m_screenHeight;
    }

    void RaylibRenderer::setScreenSize(int width, int height)
    {
        m_screenWidth = width;
        m_screenHeight = height;
    }

    // ==================== Direct Access ====================

    const Texture2D* RaylibRenderer::getRawTexture(const std::string& id) const
    {
        auto it = m_textures.find(id);
        return (it != m_textures.end()) ? &it->second : nullptr;
    }

    const std::unordered_map<std::string, Texture2D>& RaylibRenderer::getTextures() const
    {
        return m_textures;
    }

    // ==================== Private Methods ====================

    Color RaylibRenderer::toRaylibColor(const RenderColor& c)
    {
        return {c.r, c.g, c.b, c.a};
    }

} // namespace rtype::ecs
