/*
** R-Type ECS - SystemManager
** Manages and updates all ECS systems
*/

#pragma once

#include "ISystem.hpp"
#include "Types.hpp"

#include <algorithm>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace rtype::ecs {

    class Registry;

    /**
     * @brief Manages all systems in the ECS
     *
     * Handles system registration, retrieval, and ordered updates
     * based on system phases.
     */
    class SystemManager {
    public:
        explicit SystemManager(Registry* registry)
            : m_registry(registry) {}

        ~SystemManager() = default;

        SystemManager(const SystemManager&) = delete;
        SystemManager& operator=(const SystemManager&) = delete;

        /**
         * @brief Add a new system
         * @tparam T System type (must derive from ISystem)
         * @tparam Args Constructor argument types
         * @param args Constructor arguments
         * @return Pointer to the created system
         */
        template <typename T, typename... Args>
        T* addSystem(Args&&... args) {
            static_assert(std::is_base_of<ISystem, T>::value,
                "T must derive from ISystem");

            std::type_index typeIndex(typeid(T));

            auto system = std::make_unique<T>(std::forward<Args>(args)...);
            system->setRegistry(m_registry);

            T* ptr = system.get();
            m_systems[typeIndex] = std::move(system);
            m_systemOrder.push_back(ptr);

            sortSystems();

            return ptr;
        }

        /**
         * @brief Get a system by type
         * @tparam T System type
         * @return Pointer to the system, or nullptr if not found
         */
        template <typename T>
        T* getSystem() {
            std::type_index typeIndex(typeid(T));
            auto it = m_systems.find(typeIndex);
            if (it == m_systems.end()) {
                return nullptr;
            }
            return static_cast<T*>(it->second.get());
        }

        template <typename T>
        const T* getSystem() const {
            std::type_index typeIndex(typeid(T));
            auto it = m_systems.find(typeIndex);
            if (it == m_systems.end()) {
                return nullptr;
            }
            return static_cast<const T*>(it->second.get());
        }

        /**
         * @brief Remove a system by type
         * @tparam T System type
         * @return true if system was removed
         */
        template <typename T>
        bool removeSystem() {
            std::type_index typeIndex(typeid(T));
            auto it = m_systems.find(typeIndex);
            if (it == m_systems.end()) {
                return false;
            }

            ISystem* ptr = it->second.get();
            m_systemOrder.erase(
                std::remove(m_systemOrder.begin(), m_systemOrder.end(), ptr),
                m_systemOrder.end()
            );

            m_systems.erase(it);
            return true;
        }

        /**
         * @brief Update all enabled systems in phase order
         * @param dt Delta time since last update
         */
        void updateAll(float dt) {
            for (ISystem* system : m_systemOrder) {
                if (system->isEnabled()) {
                    system->update(dt);
                }
            }
        }

        /**
         * @brief Update only systems in a specific phase
         * @param phase The phase to update
         * @param dt Delta time
         */
        void updatePhase(SystemPhase phase, float dt) {
            for (ISystem* system : m_systemOrder) {
                if (system->isEnabled() && system->getPhase() == phase) {
                    system->update(dt);
                }
            }
        }

        /**
         * @brief Get the number of registered systems
         */
        std::size_t getSystemCount() const {
            return m_systems.size();
        }

        /**
         * @brief Enable or disable a system by type
         * @tparam T System type
         * @param enabled New enabled state
         */
        template <typename T>
        void setSystemEnabled(bool enabled) {
            T* system = getSystem<T>();
            if (system) {
                system->setEnabled(enabled);
            }
        }

    private:
        /**
         * @brief Sort systems by their execution phase
         */
        void sortSystems() {
            std::sort(m_systemOrder.begin(), m_systemOrder.end(),
                [](const ISystem* a, const ISystem* b) {
                    return static_cast<int>(a->getPhase()) <
                           static_cast<int>(b->getPhase());
                }
            );
        }

    private:
        Registry* m_registry;
        std::unordered_map<std::type_index, std::unique_ptr<ISystem>> m_systems;
        std::vector<ISystem*> m_systemOrder;
    };

} // namespace rtype::ecs
