/*
** R-Type ECS - IComponent Interface
** Base interface for all ECS components
*/

#pragma once

#include "Types.hpp"
#include <string>
#include <typeinfo>

namespace rtype::ecs {

    /**
     * @brief Base interface for all components in the ECS
     * 
     * Components are pure data containers. They should not contain
     * game logic - that belongs in Systems.
     */
    class IComponent {
    public:
        virtual ~IComponent() = default;

        /**
         * @brief Get the type name of this component (for debugging/serialization)
         * @return String representation of the component type
         */
        virtual std::string getTypeName() const = 0;
    };

    /**
     * @brief Helper class for compile-time component type ID generation
     * 
     * Each component type gets a unique ID at compile time using
     * a static counter. This enables efficient component storage
     * and signature matching.
     */
    class ComponentTypeIdGenerator {
    public:
        template <typename T>
        static ComponentTypeId getTypeId() {
            static const ComponentTypeId id = nextId++;
            return id;
        }

    private:
        static inline ComponentTypeId nextId = 0;
    };

    /**
     * @brief Convenience function to get a component's type ID
     * @tparam T The component type
     * @return Unique ID for this component type
     */
    template <typename T>
    ComponentTypeId getComponentTypeId() {
        static_assert(std::is_base_of<IComponent, T>::value,
            "T must derive from IComponent");
        return ComponentTypeIdGenerator::getTypeId<T>();
    }

} // namespace rtype::ecs
