/*
** R-Type ECS - View
** Cached entity query with lazy invalidation
*/

#pragma once

#include "Types.hpp"
#include "ComponentStorage.hpp"
#include "IComponentArray.hpp"

#include <vector>
#include <tuple>
#include <algorithm>

namespace rtype::ecs {

    /**
     * @brief Lightweight view over entities with specific components
     * 
     * Unlike getEntitiesWith(), views don't allocate on every call.
     * They cache results and can be iterated multiple times efficiently.
     */
    template <typename... Components>
    class View {
    public:
        using ComponentTuple = std::tuple<ComponentStorage<Components>*...>;

        View() = default;

        /**
         * @brief Construct view from component storages
         */
        explicit View(ComponentStorage<Components>*... storages)
            : m_storages(storages...) {
            findSmallestStorage();
        }

        /**
         * @brief Iterator over view entities
         */
        class Iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = EntityId;
            using difference_type = std::ptrdiff_t;
            using pointer = const EntityId*;
            using reference = const EntityId&;

            Iterator(const View* view, std::size_t index)
                : m_view(view), m_index(index) {
                skipInvalid();
            }

            reference operator*() const {
                return m_view->m_leadEntities[m_index];
            }

            Iterator& operator++() {
                ++m_index;
                skipInvalid();
                return *this;
            }

            Iterator operator++(int) {
                Iterator tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const Iterator& other) const {
                return m_index == other.m_index;
            }

            bool operator!=(const Iterator& other) const {
                return m_index != other.m_index;
            }

        private:
            void skipInvalid() {
                while (m_index < m_view->m_leadEntities.size() &&
                       !m_view->hasAllComponents(m_view->m_leadEntities[m_index])) {
                    ++m_index;
                }
            }

            const View* m_view;
            std::size_t m_index;
        };

        Iterator begin() const {
            return Iterator(this, 0);
        }

        Iterator end() const {
            return Iterator(this, m_leadEntities.size());
        }

        /**
         * @brief Get number of matching entities (requires full scan)
         */
        std::size_t size() const {
            std::size_t count = 0;
            for (EntityId entity : m_leadEntities) {
                if (hasAllComponents(entity)) {
                    ++count;
                }
            }
            return count;
        }

        /**
         * @brief Check if view is empty
         */
        bool empty() const {
            return begin() == end();
        }

        /**
         * @brief Execute function for each matching entity
         */
        template <typename Func>
        void each(Func&& func) {
            for (EntityId entity : *this) {
                func(entity, get<Components>(entity)...);
            }
        }

        /**
         * @brief Get a specific component for an entity
         */
        template <typename T>
        T& get(EntityId entity) {
            return std::get<ComponentStorage<T>*>(m_storages)->get(entity);
        }

        template <typename T>
        const T& get(EntityId entity) const {
            return std::get<ComponentStorage<T>*>(m_storages)->get(entity);
        }

    private:
        /**
         * @brief Find the smallest storage to use as lead iterator
         */
        void findSmallestStorage() {
            std::size_t minSize = std::numeric_limits<std::size_t>::max();

            auto checkStorage = [&](auto* storage) {
                if (storage && storage->size() < minSize) {
                    minSize = storage->size();
                    m_leadEntities = storage->entities();
                }
            };

            std::apply([&](auto*... storages) {
                (checkStorage(storages), ...);
            }, m_storages);
        }

        /**
         * @brief Check if entity has all required components
         */
        bool hasAllComponents(EntityId entity) const {
            return std::apply([entity](auto*... storages) {
                return (storages->contains(entity) && ...);
            }, m_storages);
        }

        ComponentTuple m_storages;
        std::vector<EntityId> m_leadEntities;  // Reference to smallest storage's entities
    };

    /**
     * @brief Single-component view (optimized case)
     */
    template <typename T>
    class SingleView {
    public:
        explicit SingleView(ComponentStorage<T>* storage) : m_storage(storage) {}

        auto begin() const { return m_storage->entities().begin(); }
        auto end() const { return m_storage->entities().end(); }

        std::size_t size() const { return m_storage->size(); }
        bool empty() const { return m_storage->size() == 0; }

        T& get(EntityId entity) { return m_storage->get(entity); }
        const T& get(EntityId entity) const { return m_storage->get(entity); }

        template <typename Func>
        void each(Func&& func) {
            const auto& entities = m_storage->entities();
            auto& components = m_storage->components();
            for (std::size_t i = 0; i < entities.size(); ++i) {
                func(entities[i], components[i]);
            }
        }

    private:
        ComponentStorage<T>* m_storage;
    };

} // namespace rtype::ecs
