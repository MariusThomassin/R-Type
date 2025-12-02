/*
** R-Type ECS - TransformComponent
** Position, rotation, and scale data for entities
*/

#pragma once

#include "engine/ecs/core/IComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief Component holding position, rotation, and scale
     *
     * Used by any entity that exists in 2D space.
     */
    struct TransformComponent : public IComponent {
        float x = 0.0f;
        float y = 0.0f;
        float rotation = 0.0f;
        float scaleX = 1.0f;
        float scaleY = 1.0f;

        TransformComponent() = default;

        TransformComponent(float posX, float posY)
            : x(posX), y(posY) {}

        TransformComponent(float posX, float posY, float rot)
            : x(posX), y(posY), rotation(rot) {}

        TransformComponent(float posX, float posY, float rot, float sX, float sY)
            : x(posX), y(posY), rotation(rot), scaleX(sX), scaleY(sY) {}

        std::string getTypeName() const override {
            return "TransformComponent";
        }
    };

} // namespace rtype::ecs
