/*
** R-Type ECS - Renderer Interface
** Abstraction layer for rendering operations
** Decouples rendering logic from raylib specifics
*/

#pragma once

#include <string>
#include <cstdint>

namespace rtype::ecs {

    /**
     * @brief Color representation (decoupled from raylib)
     */
    struct RenderColor {
        uint8_t r = 255;
        uint8_t g = 255;
        uint8_t b = 255;
        uint8_t a = 255;

        RenderColor() = default;
        RenderColor(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
            : r(red), g(green), b(blue), a(alpha) {}

        // Color operations
        RenderColor withAlpha(uint8_t alpha) const {
            return {r, g, b, alpha};
        }

        RenderColor scaled(float factor) const {
            return {
                static_cast<uint8_t>(r * factor),
                static_cast<uint8_t>(g * factor),
                static_cast<uint8_t>(b * factor),
                a
            };
        }

        // Common colors
        static RenderColor White() { return {255, 255, 255, 255}; }
        static RenderColor Black() { return {0, 0, 0, 255}; }
        static RenderColor Red() { return {255, 0, 0, 255}; }
        static RenderColor Green() { return {0, 255, 0, 255}; }
        static RenderColor Blue() { return {0, 0, 255, 255}; }
        static RenderColor Yellow() { return {255, 255, 0, 255}; }
        static RenderColor Cyan() { return {0, 255, 255, 255}; }
        static RenderColor Magenta() { return {255, 0, 255, 255}; }
    };

    /**
     * @brief Rectangle definition for rendering
     */
    struct RenderRect {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    /**
     * @brief Abstract renderer interface
     * 
     * This allows swapping out rendering backends (raylib, SDL, etc.)
     * and enables headless testing/server execution.
     */
    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        // Basic shapes
        virtual void drawRect(float x, float y, float width, float height, 
                              const RenderColor& color) = 0;
        virtual void drawRectRotated(const RenderRect& rect, float originX, float originY,
                                     float rotation, const RenderColor& color) = 0;
        virtual void drawCircle(float centerX, float centerY, float radius,
                                const RenderColor& color) = 0;
        virtual void drawLine(float x1, float y1, float x2, float y2,
                              const RenderColor& color) = 0;

        // Textures
        virtual bool loadTexture(const std::string& id, const std::string& path) = 0;
        virtual void unloadTexture(const std::string& id) = 0;
        virtual void drawTexture(const std::string& id, float x, float y,
                                 const RenderColor& tint = RenderColor::White()) = 0;
        virtual void drawTextureRect(const std::string& id, 
                                     const RenderRect& src, const RenderRect& dst,
                                     float rotation = 0.0f,
                                     const RenderColor& tint = RenderColor::White()) = 0;

        // Text
        virtual void drawText(const std::string& text, float x, float y, 
                              int fontSize, const RenderColor& color) = 0;

        // Frame management
        virtual void beginFrame() = 0;
        virtual void endFrame() = 0;
        virtual void clear(const RenderColor& color) = 0;

        // Screen info
        virtual int getScreenWidth() const = 0;
        virtual int getScreenHeight() const = 0;
    };

    /**
     * @brief Null renderer for headless operation (server, tests)
     */
    class NullRenderer : public IRenderer {
    public:
        void drawRect(float, float, float, float, const RenderColor&) override {}
        void drawRectRotated(const RenderRect&, float, float, float, const RenderColor&) override {}
        void drawCircle(float, float, float, const RenderColor&) override {}
        void drawLine(float, float, float, float, const RenderColor&) override {}

        bool loadTexture(const std::string&, const std::string&) override { return true; }
        void unloadTexture(const std::string&) override {}
        void drawTexture(const std::string&, float, float, const RenderColor&) override {}
        void drawTextureRect(const std::string&, const RenderRect&, const RenderRect&,
                             float, const RenderColor&) override {}

        void drawText(const std::string&, float, float, int, const RenderColor&) override {}

        void beginFrame() override {}
        void endFrame() override {}
        void clear(const RenderColor&) override {}

        int getScreenWidth() const override { return m_width; }
        int getScreenHeight() const override { return m_height; }

        void setScreenSize(int w, int h) { m_width = w; m_height = h; }

    private:
        int m_width = 1280;
        int m_height = 720;
    };

} // namespace rtype::ecs
