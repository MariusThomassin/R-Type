/*
** R-Type ECS - SpinSystem
** Updates rotation for entities with SpinComponent
*/

#pragma once

#include "../../engine/ecs/core/ISystem.hpp"
#include "../../engine/ecs/core/Registry.hpp"
#include "../../engine/ecs/components/TransformComponent.hpp"
#include "../components/bullets/SpinComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief System that updates continuous rotation for spinning entities
     *
     * Runs during Physics phase, updating transform rotation from SpinComponent.
     */
    class SpinSystem : public ISystem {
    public:
        SpinSystem() = default;
        ~SpinSystem() override = default;

        void update(float dt) override {
            if (!m_registry) return;

            m_registry->forEachWith<SpinComponent, TransformComponent>(
                [dt](EntityId, SpinComponent& spin, TransformComponent& transform) {
                    float rotationDelta = spin.update(dt);
                    transform.rotation = rotationDelta;
                }
            );
        }

        SystemPhase getPhase() const override {
            return SystemPhase::Physics;
        }
    };

} // namespace rtype::ecs
