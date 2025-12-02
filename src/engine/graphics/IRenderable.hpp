/*
** R-Type ECS - IRenderable Interface
** Optional interface for components that can render themselves
** This enables self-rendering components, reducing RenderSystem complexity
*/

#pragma once

#include <raylib.h>
#include <unordered_map>
#include <string>
#include <cmath>

namespace rtype::ecs {

    // Forward declaration
    struct TransformComponent;

    /**
     * @brief Context passed to renderable components
     * 
     * Contains all resources and state needed for rendering,
     * allowing components to render themselves without coupling
     * to the RenderSystem implementation.
     */
    struct RenderContext {
        const std::unordered_map<std::string, Texture2D>* textures = nullptr;
        
        int screenWidth = 1280;
        int screenHeight = 720;
        
        float animTime = 0.0f;
        float deltaTime = 0.0f;
        
        /**
         * @brief Get a texture by ID
         * @param id Texture identifier
         * @return Pointer to texture, or nullptr if not found
         */
        const Texture2D* getTexture(const std::string& id) const {
            if (!textures) return nullptr;
            auto it = textures->find(id);
            return (it != textures->end()) ? &it->second : nullptr;
        }
    };

    /**
     * @brief Interface for components that can render themselves
     * 
     * Components implementing this interface take ownership of their
     * rendering logic, making the RenderSystem a thin coordinator.
     * 
     * This follows the "Tell, Don't Ask" principle - instead of
     * RenderSystem asking for component data and deciding how to
     * render, it tells the component to render itself.
     * 
     * Benefits:
     * - Rendering logic lives with the data it operates on
     * - Adding new renderable components doesn't modify RenderSystem
     * - Components can have specialized rendering without bloating RenderSystem
     * - Easier to test rendering in isolation
     */
    class IRenderable {
    public:
        virtual ~IRenderable() = default;

        /**
         * @brief Render this component
         * 
         * @param transform The entity's transform (position, rotation, scale)
         * @param ctx Rendering context with resources and state
         */
        virtual void render(const TransformComponent& transform, 
                           const RenderContext& ctx) const = 0;

        /**
         * @brief Check if this renderable should be drawn
         * @return true if visible and should render
         */
        virtual bool isRenderable() const { return true; }

        /**
         * @brief Get the render layer (for sorting)
         * @return Layer value (higher = rendered on top)
         */
        virtual int getRenderLayer() const { return 0; }
    };

} // namespace rtype::ecs
