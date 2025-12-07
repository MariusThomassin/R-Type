/*
** R-Type ECS - SpinSystem
** Updates rotation for entities with SpinComponent
*/

#pragma once

#include "engine/ecs/core/ISystem.hpp"
#include "engine/ecs/core/Registry.hpp"
#include "engine/ecs/components/TransformComponent.hpp"
#include "game/components/bullets/SpinComponent.hpp"

namespace rtype::ecs {

    /**
     * @brief System that updates continuous rotation for spinning entities
     *
     * Runs during Physics phase, updating transform rotation from SpinComponent.
     */
    class SpinSystem : public ISystem {
        public:
            /**
             * @brief Construct a new Spin System object
             */
            SpinSystem() = default;
            /**
             * @brief Destroy the Spin System object
             */
            ~SpinSystem() override = default;

            /**
             * @brief Update all entities with SpinComponent
             * @param dt Delta time since last update
             */
            void update(float dt) override {
                if (!m_registry) return;

                m_registry->forEachWith<SpinComponent, TransformComponent>(
                    [dt](EntityId, SpinComponent& spin, TransformComponent& transform) {
                        float rotationDelta = spin.update(dt);
                        transform.rotation = rotationDelta;
                    }
                );
            }

            /**
             * @brief Get the system phase (Physics)
             * @return SystemPhase
             */
            SystemPhase getPhase() const override {
                return SystemPhase::Physics;
            }
        };
} // namespace rtype::ecs
