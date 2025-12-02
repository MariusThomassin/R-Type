/*
** R-Type ECS - SpriteComponent
** Visual representation data for rendering
*/

#pragma once

#include "../IComponent.hpp"
#include <string>

namespace rtype::ecs {

    /**
     * @brief Component holding sprite/texture rendering data
     *
     * Used by entities that need to be rendered visually.
     * The textureId refers to a texture managed by the asset system.
     */
    struct SpriteComponent : public IComponent {
        std::string textureId;   // Asset manager texture key
        int layer = 0;           // Render layer (higher = on top)
        bool isVisible = true;   // Visibility toggle

        float srcX = 0.0f;
        float srcY = 0.0f;
        float srcWidth = 0.0f;
        float srcHeight = 0.0f;

        unsigned char tintR = 255;
        unsigned char tintG = 255;
        unsigned char tintB = 255;
        unsigned char tintA = 255;

        SpriteComponent() = default;

        explicit SpriteComponent(const std::string& texId)
            : textureId(texId) {}

        SpriteComponent(const std::string& texId, int renderLayer)
            : textureId(texId), layer(renderLayer) {}

        std::string getTypeName() const override {
            return "SpriteComponent";
        }
    };

} // namespace rtype::ecs
