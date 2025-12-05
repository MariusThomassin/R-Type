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
     * 
     * @note Implements IRenderer interface using raylib functions
     * @note Manages texture loading/unloading and basic drawing
     */
    class RaylibRenderer : public IRenderer {
    public:
        /**
         * @brief Construct a new RaylibRenderer object
         * @param screenWidth The width of the rendering screen
         * @param screenHeight The height of the rendering screen
         */
        RaylibRenderer(int screenWidth = 1280, int screenHeight = 720);

        /**
         * @brief Destroy the RaylibRenderer object
         */
        ~RaylibRenderer() override;

        // ==================== Basic Shapes ====================

        /**
         * @brief Draw a filled rectangle
         * @param x Top-left x coordinate
         * @param y Top-left y coordinate
         * @param width Rectangle width
         * @param height Rectangle height
         * @param color Fill color
         */
        void drawRect(float x, float y, float width, float height, const RenderColor& color) override;
        /**
         * @brief Draw a rotated rectangle
         * @param rect Rectangle parameters
         * @param originX X coordinate of rotation origin
         * @param originY Y coordinate of rotation origin
         * @param rotation Rotation angle in degrees
         * @param color Fill color
         */
        void drawRectRotated(const RenderRect& rect, float originX, float originY, float rotation, const RenderColor& color) override;
        /**
         * @brief Draw a filled circle
         * @param centerX X coordinate of circle center
         * @param centerY Y coordinate of circle center
         * @param radius Circle radius
         * @param color Fill color
         */
        void drawCircle(float centerX, float centerY, float radius, const RenderColor& color) override;
        /**
         * @brief Draw a line
         * @param x1 Starting point x coordinate
         * @param y1 Starting point y coordinate
         * @param x2 Ending point x coordinate
         * @param y2 Ending point y coordinate
         * @param color Line color
         */
        void drawLine(float x1, float y1, float x2, float y2, const RenderColor& color) override;

        // ==================== Textures ====================

        /**
         * @brief Load a texture from file
         * @param id Unique identifier for the texture
         * @param path File path to the texture image
         * @return True if loaded successfully, false otherwise
         */
        bool loadTexture(const std::string& id, const std::string& path) override;
        /**
         * @brief Unload a previously loaded texture
         * @param id Unique identifier of the texture to unload
         */
        void unloadTexture(const std::string& id) override;

        /**
         * @brief Draw a texture at specified position
         * @param id Unique identifier of the texture
         * @param x X coordinate
         * @param y Y coordinate
         * @param tint Color tint to apply
         */
        void drawTexture(const std::string& id, float x, float y, const RenderColor& tint) override;
        /**
         * @brief Draw a portion of a texture with rotation
         * @param id Unique identifier of the texture
         * @param src Source rectangle within the texture
         * @param dst Destination rectangle on screen
         * @param rotation Rotation angle in degrees
         * @param tint Color tint to apply
         */
        void drawTextureRect(const std::string& id, const RenderRect& src, const RenderRect& dst, float rotation, const RenderColor& tint) override;

        // ==================== Text ====================

        /**
         * @brief Draw text at specified position
         * @param text The string to draw
         * @param x X coordinate
         * @param y Y coordinate
         * @param fontSize Font size in points
         * @param color Text color
         */
        void drawText(const std::string& text, float x, float y, int fontSize, const RenderColor& color) override;

        // ==================== Frame Management ====================

        /**
         * @brief Begin a new frame for rendering
         */
        void beginFrame() override;
        /**
         * @brief End the current frame and present it
         */
        void endFrame() override;
        /**
         * @brief Clear the screen with specified color
         * @param color Clear color
         */
        void clear(const RenderColor& color) override;

        // ==================== Screen Info ====================

        /**
         * @brief Get the current screen width
         * @return Screen width in pixels
         */
        int getScreenWidth() const override;
        /**
         * @brief Get the current screen height
         * @return Screen height in pixels
         */
        int getScreenHeight() const override;
        /**
         * @brief Set the screen size
         * @param width New screen width in pixels
         * @param height New screen height in pixels
         */
        void setScreenSize(int width, int height);

        // ==================== Direct Access (for advanced usage) ====================

        /**
         * @brief Get raw texture for direct raylib calls
         * @param id Unique identifier of the texture
         * @return Pointer to the Texture2D, or nullptr if not found
         * @note Use sparingly - prefer using IRenderer methods
         */
        const Texture2D* getRawTexture(const std::string& id) const;
        /**
         * @brief Get all textures (for legacy RenderContext compatibility)
         * @return Map of texture IDs to Texture2D objects
         */
        const std::unordered_map<std::string, Texture2D>& getTextures() const;

    private:
        /**
         * @brief Convert RenderColor to raylib Color
         * @param c RenderColor to convert
         * @return raylib Color representation
         */
        static Color toRaylibColor(const RenderColor& c);

        /**
         * @brief Member variables
         */
        int m_screenWidth;
        /**
         * @brief Member variables
         */
        int m_screenHeight;
        /**
         * @brief Loaded textures mapped by ID
         */
        std::unordered_map<std::string, Texture2D> m_textures;
    };

} // namespace rtype::ecs
