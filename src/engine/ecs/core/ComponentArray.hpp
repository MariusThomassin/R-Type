/*
** R-Type ECS - ComponentArray
** Type-safe, cache-friendly component storage using sparse set
*/

#pragma once

#include "IComponent.hpp"
#include "IComponentArray.hpp"
#include "Types.hpp"

#include <array>
#include <cassert>
#include <optional>
#include <unordered_map>
#include <vector>

namespace rtype::ecs {

    /**
     * @brief Packed array for storing components of a single type
     * @tparam T The component type to store
     * 
     * Uses a sparse set pattern for O(1) access while keeping
     * components packed in memory for cache-friendly iteration.
     */
    template <typename T>
    class ComponentArray : public IComponentArray {
        static_assert(std::is_base_of<IComponent, T>::value,
            "T must derive from IComponent");

    public:
        ComponentArray() = default;
        ~ComponentArray() override = default;

        /**
         * @brief Add a component to an entity
         * @param entity The entity ID
         * @param component The component data to store
         */
        void insertComponent(EntityId entity, T component) {
            assert(!hasComponent(entity) && "Component already exists for entity");

            std::size_t newIndex = m_components.size();
            m_entityToIndex[entity] = newIndex;
            m_indexToEntity[newIndex] = entity;
            m_components.push_back(std::move(component));
        }

        /**
         * @brief Remove a component from an entity
         * @param entity The entity ID
         * 
         * Uses swap-and-pop to maintain packed array.
         */
        void removeComponent(EntityId entity) {
            assert(hasComponent(entity) && "Component does not exist for entity");

            std::size_t removedIndex = m_entityToIndex[entity];
            std::size_t lastIndex = m_components.size() - 1;

            if (removedIndex != lastIndex) {
                m_components[removedIndex] = std::move(m_components[lastIndex]);

                EntityId lastEntity = m_indexToEntity[lastIndex];
                m_entityToIndex[lastEntity] = removedIndex;
                m_indexToEntity[removedIndex] = lastEntity;
            }

            m_components.pop_back();
            m_entityToIndex.erase(entity);
            m_indexToEntity.erase(lastIndex);
        }

        /**
         * @brief Get a component for an entity
         * @param entity The entity ID
         * @return Reference to the component
         */
        T& getComponent(EntityId entity) {
            assert(hasComponent(entity) && "Component does not exist for entity");
            return m_components[m_entityToIndex[entity]];
        }

        /**
         * @brief Get a const component for an entity
         * @param entity The entity ID
         * @return Const reference to the component
         */
        const T& getComponent(EntityId entity) const {
            assert(hasComponent(entity) && "Component does not exist for entity");
            return m_components[m_entityToIndex.at(entity)];
        }

        /**
         * @brief Try to get a component (returns nullptr if not found)
         * @param entity The entity ID
         * @return Pointer to component or nullptr
         */
        T* tryGetComponent(EntityId entity) {
            auto it = m_entityToIndex.find(entity);
            if (it == m_entityToIndex.end()) {
                return nullptr;
            }
            return &m_components[it->second];
        }

        const T* tryGetComponent(EntityId entity) const {
            auto it = m_entityToIndex.find(entity);
            if (it == m_entityToIndex.end()) {
                return nullptr;
            }
            return &m_components[it->second];
        }

        void entityDestroyed(EntityId entity) override {
            if (hasComponent(entity)) {
                removeComponent(entity);
            }
        }

        bool hasComponent(EntityId entity) const override {
            return m_entityToIndex.find(entity) != m_entityToIndex.end();
        }

        std::size_t size() const override {
            return m_components.size();
        }

        auto begin() { return m_components.begin(); }
        auto end() { return m_components.end(); }
        auto begin() const { return m_components.begin(); }
        auto end() const { return m_components.end(); }

        /**
         * @brief Get entity ID at a specific index
         * @param index Index in the packed array
         * @return The entity ID
         */
        EntityId getEntityAtIndex(std::size_t index) const {
            assert(index < m_components.size() && "Index out of bounds");
            return m_indexToEntity.at(index);
        }

    private:
        std::vector<T> m_components;

        std::unordered_map<EntityId, std::size_t> m_entityToIndex;
        std::unordered_map<std::size_t, EntityId> m_indexToEntity;
    };

} // namespace rtype::ecs
